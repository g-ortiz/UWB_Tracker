

#ifndef Tracker_h
#define Tracker_h

#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include <SPI.h>

#define FILTER_LENGTH 2
#define NUM_VARS 3


#define Front 0
#define Left 1
#define Back 2
#define Right 3

class TrackerClass {
public:
	/* ##########Functions############## */
	
	static void initTracker();
	//Filter
	static float filter(float newDist, uint8_t anchor, float coord[]);
	
	//Mutlilateration
	static void loc(float distance, uint8_t anchor, float coord[]);
	static uint8_t numDists();
	
	//Movement
	static void movement(float coord[], uint8_t moveto[]);
	static void circles( float cx0, float cy0, float radius0, float cx1, float cy1, float radius1, float radius2, float coord[], uint8_t side);
	
	//Kalman
	static void kalman(float coord[]);
	/* ##############Variables############# */
	//Filter
	static uint16_t delta, filtCounter;
	static float first_val, init_val;
	static float avg,sum;
	
	//Multilateration
	static const float SEPARATIONx, SEPARATIONy;
	static float FLx, FLy, RLx, RLy, RRx, RRy, FRx, FRy;
	static float d1, d2, d3, d4;
	static float xcoord, ycoord;
	
	//Movement
	static uint8_t _PIN_Left_F, _PIN_Right_F, _PIN_Left_B, _PIN_Right_B;
	//Kalman
	static float ax, ay, px, py, gx, gy;
	static float std_dev;
	static float x_hat, y_hat, x_prev, y_prev;
	static uint8_t kalman_count;
};

extern TrackerClass Tracker;

#endif