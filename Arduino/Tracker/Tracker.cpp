/*
Class "Tracker" Created by Fredrik Treven to support UWB Tracker through Arduino
Februrary 3rd 2017
Cybercom AB
Chalmers University of Technology
*/



#include "Tracker.h"

TrackerClass Tracker;

//Variables for filter
uint16_t TrackerClass::delta;
uint16_t TrackerClass::filtCounter;
float TrackerClass::first_val, TrackerClass::init_val;
float TrackerClass::avg, TrackerClass::sum;
float filt_list[4 * FILTER_LENGTH + 4 * NUM_VARS];
uint8_t array_ptr, vars_ptr;


//Variables for multi-lat
const float TrackerClass::SEPARATION = 31.0; //Distance between all anchors in both x and y directions (cm)
											 /* Coordinates of anchors with respect to reference point RL (0,0) */
float TrackerClass::FLx = 0.0;
float TrackerClass::FLy = SEPARATION;
float TrackerClass::FRx = SEPARATION;
float TrackerClass::FRy = SEPARATION;
float TrackerClass::RRx = SEPARATION;
float TrackerClass::RRy = 0.0;
float TrackerClass::RLx = 0.0;
float TrackerClass::RLy = 0.0;

//Position of target
float TrackerClass::xcoord;
float TrackerClass::ycoord;

/*Distances being read in (keep track as global vars) */
float TrackerClass::d1; //Distance in from FL
float TrackerClass::d2; //Distance in from FR
float TrackerClass::d3; //Distance in from RR
float TrackerClass::d4; //Distance in from RL

void TrackerClass::initLoc()
{
	d1 = 0.0;
	d2 = 0.0;
	d3 = 0.0;
	d4 = 0.0;
}

void TrackerClass::loc(float distance, uint8_t anchor)
{
	switch (anchor) //Determine which anchor the distance is being read in from an update that distance
	{
	case 0:
		d1 = distance;
		break;
	case 1:
		d2 = distance;
		break;
	case 2:
		d3 = distance;
		break;
	case 3:
		d4 = distance;
		break;
	}

	uint8_t number = numDists(); //Get number of distances received thus far

	if (number < 4)
	{
		xcoord = 0.0;
		ycoord = 0.0; //If the number of distances from each anchor have not arrived return 0
	}
	else
	{
		/*Define Matrix A and b */
		float a11 = FLx - RLx;
		float a12 = FLy - RLy;
		float a21 = FRx - RLx;
		float a22 = FRy - RLy;
		float a31 = RRx - RLx;
		float a32 = RRy - RLy;
		float A[3][2] = { { -2 * a11,-2 * a12 },{ -2 * a21,-2 * a22 },{ -2 * a31,-2 * a32 } };

		float f = pow(d4, 2)*-1 + pow(RLx, 2) + pow(RLy, 2);
		float b[3][1];
		b[0][0] = pow(d1, 2) - pow(FLx, 2) - pow(FLy, 2) + f;
		b[1][0] = pow(d2, 2) - pow(FRx, 2) - pow(FRy, 2) + f;
		b[2][0] = pow(d3, 2) - pow(RRx, 2) - pow(RRy, 2) + f;

		/*Transpose Matrix A */

		float A_t[2][3];
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				A_t[j][i] = A[i][j];
			}
		}

		/*Multiply Transposed A and A */

		float temp[2][2];
		float Ap;
		//Perform matrix multiplication

		for (int k = 0; k < 2; k++)
		{
			for (int i = 0; i < 2; i++)
			{
				Ap = 0.0;
				for (int j = 0; j < 3; j++)
				{
					Ap = A_t[k][j] * A[j][i] + Ap;
				}
				temp[k][i] = Ap;
			}
		}

		/*inv = temp ^-1 */

		float inv[2][2];
		float det = temp[0][0] * temp[1][1];
		float det2 = temp[0][1] * temp[1][0];
		det = det - det2;

		inv[0][0] = temp[1][1];
		inv[0][1] = -1 * temp[0][1];
		inv[1][0] = -1 * temp[1][0];
		inv[1][1] = temp[0][0];


		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				inv[i][j] = inv[i][j] / det;
			}
		}

		/*Multiply transposed A by b */

		float temp2[2][1];


		for (int k = 0; k < 2; k++)
		{
			for (int i = 0; i < 1; i++)
			{
				Ap = 0.0;
				for (int j = 0; j < 3; j++)
				{
					Ap = A_t[k][j] * b[j][i] + Ap;
				}
				temp2[k][i] = Ap;
			}
		}

		/*Find position P matrix with P = inv x temp2 */

		float P[2][1];


		for (int k = 0; k < 2; k++)
		{
			for (int i = 0; i < 1; i++)
			{
				Ap = 0.0;
				for (int j = 0; j < 2; j++)
				{
					Ap = inv[k][j] * temp2[j][i] + Ap;
				}
				P[k][i] = Ap;
			}
		}

		xcoord = P[0][0];
		ycoord = P[1][0];



	}
}

float TrackerClass::getX()
{
	return xcoord;
}

float TrackerClass::getY()
{
	return ycoord;
}

uint8_t TrackerClass::numDists()
{
	uint8_t counter = 0;

	if (d1 != 0.0)
		counter += 1;
	if (d2 != 0.0)
		counter += 1;
	if (d3 != 0.0)
		counter += 1;
	if (d4 != 0.0)
		counter += 1;

	return counter;


}

void TrackerClass::initFilter()
{
	init_val = 30;
	sum = init_val; //Sum needs to be set to intial value for finding the first average (no large bearing on code)
	filtCounter = 0;

	//Populate filter (for all anchors)
	for (int i = 0; i < 4 * FILTER_LENGTH + 4 * NUM_VARS; i++)
	{
		filt_list[i] = init_val;
	}

	//Now set values for each anchor's sum and counter (initial avg is trivial)
	/* Note: in the array 1st = FL, 2nd = RL (the reference point), 3rd = RR, 4th = FR */
	for (int i = 0; i < 4; i++)
	{
		filt_list[i*FILTER_LENGTH + i*NUM_VARS + FILTER_LENGTH] = sum;
		filt_list[i*FILTER_LENGTH + i*NUM_VARS + FILTER_LENGTH + 2] = filtCounter;
	}


	/* 	for(int i = 0; i < 4*FILTER_LENGTH + 4*NUM_VARS; i++)
	{
	Serial.print(i);
	Serial.print(": ");
	Serial.println(filt_list[i]);
	} */
}


float TrackerClass::filter(float newDist, uint8_t anchor)
{
	delta = 100; //Range in which new reading must reside (avg +/- delta)
	array_ptr = anchor*FILTER_LENGTH + anchor*NUM_VARS; //Points to the start of the specific anchor's array
	vars_ptr = array_ptr + FILTER_LENGTH; //Start of variables (sum, avg, counter) for each anchor


	if (newDist > 0)
	{
		first_val = filt_list[array_ptr]; //Grab the most outdated value of the distance array
		filt_list[vars_ptr + 2] = filt_list[vars_ptr + 2] + 1; //Update specific anchor's counter

		for (int k = 0; k < FILTER_LENGTH; k++)
		{
			filt_list[k + array_ptr] = filt_list[k + array_ptr + 1]; //Shift all values to the left leaving last element empty
		}
	}

	if (filt_list[vars_ptr + 2] < FILTER_LENGTH) //If the array has not yet been filled
	{
		if (newDist > 0)
		{
			filt_list[vars_ptr] = filt_list[vars_ptr] + newDist; //sum = sum + newDist;
			filt_list[vars_ptr + 1] = filt_list[vars_ptr] / filt_list[vars_ptr + 2]; //avg = sum/filtCounter;
			filt_list[vars_ptr - 1] = newDist; //Place new distance as latest element
		}
		return 0; //Return 0 until the array has been filled
	}
	else
	{
		if (newDist > 0 && newDist >= filt_list[vars_ptr + 1] - delta  && newDist <= filt_list[vars_ptr + 1] + delta) //If the received distance is within range
		{
			filt_list[vars_ptr - 1] = newDist; //Update most recent element in array with newest received distance
			filt_list[vars_ptr] = filt_list[vars_ptr] - first_val + newDist; //sum = sum - first_val + newDist; Update sum by subtracting most dated element and adding most recent one
			filt_list[vars_ptr + 1] = filt_list[vars_ptr] / FILTER_LENGTH; //avg = sum/FILTER_LENGTH; Update average
		}
		else //Value not in range
		{
			filt_list[vars_ptr - 1] = filt_list[vars_ptr - 2]; //Duplicate second most recent value and keep as most recent
			filt_list[vars_ptr + 1] = filt_list[vars_ptr + 1]; //Average remains unchanged
		}
		return filt_list[vars_ptr + 1]; //Return the specified anchor's average
	}
}