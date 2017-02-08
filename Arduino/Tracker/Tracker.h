

#ifndef Tracker_h
#define Tracker_h

#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include <SPI.h>

#define FILTER_LENGTH 12
#define NUM_VARS 3


class TrackerClass {
public:
	/* ##########Functions############## */
	
	//Filter
	static void initFilter();
	static float filter(float newDist, uint8_t anchor);
	
	//Mutlilateration
	static void initLoc();
	static float getX();
	static float getY();
	static void loc(float distance, uint8_t anchor);
	static uint8_t numDists();
	
	/* ##############Variables############# */
	//Filter
	static uint16_t delta, filtCounter;
	static float first_val, init_val;
	static float avg,sum;
	
	//Multilateration
	static const float SEPARATION;
	static float FLx, FLy, RLx, RLy, RRx, RRy, FRx, FRy;
	static float d1, d2, d3, d4;
	static float xcoord, ycoord;
};

extern TrackerClass Tracker;

#endif