/*
Class "Tracker" Created by Fredrik Treven to support UWB Tracker through Arduino
Februrary 3rd 2017
Cybercom AB
Chalmers University of Technology
*/



#include "Tracker.h"

TrackerClass Tracker;

//Variables for filter
uint16_t TrackerClass::delta, TrackerClass::filtCounter;
float TrackerClass::first_val, TrackerClass::init_val;
float TrackerClass::avg, TrackerClass::sum;
float filt_list[FILTER_LENGTH];


//Variables for multi-lat
const float TrackerClass::SEPARATION = 20.0; //Distance between all anchors in both x and y directions (cm)
/* Coordinates of anchors with respect to reference point RL (0,0) */
float TrackerClass::FLx = 0.0;
float TrackerClass::FLy = SEPARATION;
float TrackerClass::RLx = 0.0;
float TrackerClass::RLy = 0.0;
float TrackerClass::FRx = SEPARATION;
float TrackerClass::FRy = SEPARATION;
float TrackerClass::RRx = SEPARATION;
float TrackerClass::RRy = 0.0;
/*Distances being read in (keep track as global vars) */
float TrackerClass::d1; //Distance in from FL
float TrackerClass::d2; //Distance in from RL
float TrackerClass::d3; //Distance in from RR
float TrackerClass::d4; //Distance in from FR


void TrackerClass::initFilter()
{
	init_val = 30;
	sum = init_val;
	filtCounter = 0;
	for(int i = 0; i < FILTER_LENGTH; i++)
    {
      filt_list[i] = init_val;
      //Serial.print(filt_list[i]);
	  //Serial.print(" ");
    }
}


float TrackerClass::filter(float newDist)
{
	delta = 100; //Range in which new reading must reside (avg +/- delta)
	
	if(newDist > 0)
	{
		first_val = filt_list[0]; //Grab the most outdated value of the distance array
		filtCounter = filtCounter + 1;
		
		for(int k = 0; k < FILTER_LENGTH; k++)
		{
			filt_list[k] = filt_list[k+1]; //Shift all values to the left leaving last element empty
		}
	}
	
	if(filtCounter < FILTER_LENGTH) //If the array has not yet been filled
	{
		if(newDist > 0)
		{
			sum = sum + newDist;
			avg = sum/filtCounter;
			filt_list[FILTER_LENGTH-1] = newDist; //Place new distance as latest element
		}
		return 0; //Return 0 until the array has been filled
	}
	else
	{
		if(newDist > 0 && newDist >= avg - delta  && newDist <= avg + delta) //If the received distance is within range
		{
			filt_list[FILTER_LENGTH-1] = newDist; //Update most recent element in array with newest received distance
			sum = sum - first_val + newDist; //Update sum by subtracting most dated element and adding most recent one
			avg = sum/FILTER_LENGTH; //Update average
		}			
		else //Value not in range
		{
			filt_list[FILTER_LENGTH-1] = filt_list[FILTER_LENGTH-2]; //Duplicate second most recent value and keep as most recent
			avg = avg; //Average remains unchanged
		}
		return avg;
	}
}