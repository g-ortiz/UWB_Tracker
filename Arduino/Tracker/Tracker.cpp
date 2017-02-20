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
float TrackerClass::RLx = 0.0;
float TrackerClass::RLy = 0.0;
float TrackerClass::RRx = SEPARATION;
float TrackerClass::RRy = 0.0;
float TrackerClass::FRx = SEPARATION;
float TrackerClass::FRy = SEPARATION;

//Position of target
float TrackerClass::xcoord;
float TrackerClass::ycoord;

/*Distances being read in (keep track as global vars) */
float TrackerClass::d1; //Distance in from FL
float TrackerClass::d2; //Distance in from RL
float TrackerClass::d3; //Distance in from RR
float TrackerClass::d4; //Distance in from FR

//Movement PINS
uint8_t TrackerClass::_PIN_Left_F = 9;
uint8_t TrackerClass::_PIN_Right_F = 8;
uint8_t TrackerClass::_PIN_Left_B = 4;
uint8_t TrackerClass::_PIN_Right_B = 3;

//Kalman
float TrackerClass::ax, TrackerClass::ay;
float TrackerClass::px, TrackerClass::py;
float TrackerClass::gx, TrackerClass::gy;
float TrackerClass::std_dev;
float TrackerClass::x_hat;
float TrackerClass::y_hat;
uint8_t TrackerClass::kalman_count;
float TrackerClass::x_prev, TrackerClass::y_prev;


void TrackerClass::initTracker()
{
	//For filter
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
	
	//For multilat
	d1 = 0.0;
	d2 = 0.0;
	d3 = 0.0;
	d4 = 0.0;
	
	//Initialize first states of Kalman Filter
	ax = 1; //For static location
	ay = 1;
	px = 1; //Prediction error, arbitrary initial value
	py = 1;
	std_dev = 20; //Standard deviation based on sensor
	kalman_count = 0;
}

void TrackerClass::loc(float distance, uint8_t anchor, float coord[])
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

		/*Initial position estimate*/
		float x_c = P[0][0];
		float y_c = P[1][0];
		
		/*Find minimum distance to project values onto*/
		
		float min = 100000; //Set min high initially
		float ds[4] = {d1, d2, d3, d4}; //Create array of distances
		float d_min; //Minimum distance
		
		for (int j = 0; j < number; j++) //Recall: number = number of distances recorded (4)
		{
			if(ds[j] < min)
			{
				d_min = ds[j]; //Set as minimum distance
				min = d_min; //Update min
			}
		}
		
		float dist = sqrt(pow(x_c,2) + pow(y_c,2)); //Distance from origin to point (can maybe use d4 for this?)
		
		int8_t sign;
		
		//Get sign of y coordinate
		if(y_c >= 0)
			sign = 1;
		else
			sign = -1;
		
		//Determine projected values
		
		float x_new = sign*x_c*d_min/dist;
		float y_new = abs(y_c)*d_min/dist;
		
		
		if(y_c >= 0)
		{
			xcoord = x_new;
			ycoord = y_new;
		}
		else
		{
			xcoord = -1*x_new;
			ycoord = -1*y_new;
		}

	}
	
	coord[0] = xcoord;
	coord[1] = ycoord;
	return;
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


float TrackerClass::filter(float newDist, uint8_t anchor, float coord[])
{
	delta = 100; //Range in which new reading must reside to be included in average (avg +/- delta)
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
		float new_avg = filt_list[vars_ptr + 1]; 
		loc(new_avg, anchor, coord+2);
		if (anchor == 3){
			array_ptr = (anchor-3)*FILTER_LENGTH + (anchor-3)*NUM_VARS; //Points to the start of the specific anchor's array
			vars_ptr = array_ptr + FILTER_LENGTH; //Start of variables (sum, avg, counter) for each anchor		
			float radiusfl = filt_list[vars_ptr + 1];
			array_ptr = (anchor-2)*FILTER_LENGTH + (anchor-2)*NUM_VARS; //Points to the start of the specific anchor's array
			vars_ptr = array_ptr + FILTER_LENGTH; //Start of variables (sum, avg, counter) for each anchor		
			float radiusfr = filt_list[vars_ptr + 1];
			array_ptr = (anchor-1)*FILTER_LENGTH + (anchor-1)*NUM_VARS; //Points to the start of the specific anchor's array
			vars_ptr = array_ptr + FILTER_LENGTH; //Start of variables (sum, avg, counter) for each anchor		
			float radiusrr = filt_list[vars_ptr + 1]	;			
			array_ptr = (anchor)*FILTER_LENGTH + (anchor)*NUM_VARS; //Points to the start of the specific anchor's array
			vars_ptr = array_ptr + FILTER_LENGTH; //Start of variables (sum, avg, counter) for each anchor		
			float radiusrl = filt_list[vars_ptr + 1]	;
			circles(0,SEPARATION,radiusfl,SEPARATION, SEPARATION, radiusfr,radiusrl,coord, Front);
		
		}
			
		return  new_avg;//Return the specified anchor's average (so we can print)
	}
}

void TrackerClass::kalman(float coord[])
{
	y_raw = coord[1];
	x_raw = coord[0];
	if(kalman_count < 1)
	{
		y_hat = y_raw;
		x_hat = x_raw;
		x_prev = x_raw;
		y_prev = y_raw;
		kalman_count = kalman_count + 1;
	}
	else
	{
		//Predict
		x_hat = x_hat*ax;
		y_hat = y_hat*ay;
		
		px = ax*px*ax;
		py = ay*py*ay;
		
		//Update
		gx = px/(px + std_dev);
		gy = py/(py + std_dev);
		
		x_hat = x_hat + gx*(x_prev - x_hat);
		y_hat = y_hat + gy*(y_prev - y_hat);
		
		px = (1-gx)*px;
		py = (1-gy)*py;
		
		x_prev = x_raw;
		y_prev = y_raw;
	}
	coord[0] = x_hat;
	coord[1] = y_hat;
}


void TrackerClass::circles( float cx0, float cy0, float radius0, float cx1, float cy1, float radius1, float radius2, float coord[], uint8_t side)
{
	// Find the distance between the centers.
    float dx = cx0 - cx1;
    float dy = cy0 - cy1;
	double dist = sqrt(dx * dx + dy * dy);

	// See how many solutions there are.
	if (dist > radius0 + radius1)
	{
		// No solutions, the circles are too far apart.
		coord[0] = 0; 
		coord[1] = 0; 
		return;
	}
	else if (dist < abs(radius0 - radius1))
	{
		// No solutions, one circle contains the other.
		coord[0] = 0; 
		coord[1] = 0; 
		return;
	}
	else if ((dist == 0) && (radius0 == radius1))
	{
		// No solutions, the circles coincide.
		coord[0] = 0; 
		coord[1] = 0; 
		return;
	}
	else
	{
		// Find a and h.
		double a = (radius0 * radius0 -
			radius1 * radius1 + dist * dist) / (2 * dist);
		double h = sqrt(radius0 * radius0 - a * a);

		// Find P2.
		double cx2 = cx0 + a * (cx1 - cx0) / dist;
		double cy2 = cy0 + a * (cy1 - cy0) / dist;

		if (side == Front){
			if (radius0<radius2){
				if(((float)(cy2 - h * (cx1 - cx0) / dist))>0){
					coord[0] = (float)(cx2 + h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 - h * (cx1 - cx0) / dist);
				}else{
					coord[0] = (float)(cx2 - h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 + h * (cx1 - cx0) / dist);			
				}
			}else{
				if(((float)(cy2 - h * (cx1 - cx0) / dist))<0){
					coord[0] = (float)(cx2 + h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 - h * (cx1 - cx0) / dist);
				}else{
					coord[0] = (float)(cx2 - h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 + h * (cx1 - cx0 ) / dist);			
				}
			}
		}else if (side == Left){
			if (radius0<radius2){
				if(((float)(cx2 + h * (cy1 - cy0) / dist))>0){
					coord[0] = (float)(cx2 + h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 - h * (cx1 - cx0) / dist);
				}else{
					coord[0] = (float)(cx2 - h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 + h * (cx1 - cx0) / dist);			
				}
			}else{
				if(((float)(cx2 + h * (cy1 - cy0) / dist))<0){
					coord[0] = (float)(cx2 + h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 - h * (cx1 - cx0) / dist);
				}else{
					coord[0] = (float)(cx2 - h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 + h * (cx1 - cx0 ) / dist);			
				}
			}
		}else if (side == Back){
			if (radius2<radius0){
				if(((float)(cy2 - h * (cx1 - cx0) / dist))>0){
					coord[0] = (float)(cx2 + h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 - h * (cx1 - cx0) / dist);
				}else{
					coord[0] = (float)(cx2 - h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 + h * (cx1 - cx0) / dist);			
				}
			}else{
				if(((float)(cy2 - h * (cx1 - cx0) / dist))<0){
					coord[0] = (float)(cx2 + h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 - h * (cx1 - cx0) / dist);
				}else{
					coord[0] = (float)(cx2 - h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 + h * (cx1 - cx0 ) / dist);			
				}
			}
		}else if (side == Left){
			if (radius2<radius0){
				if(((float)(cx2 + h * (cy1 - cy0) / dist))>0){
					coord[0] = (float)(cx2 + h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 - h * (cx1 - cx0) / dist);
				}else{
					coord[0] = (float)(cx2 - h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 + h * (cx1 - cx0) / dist);			
				}
			}else{
				if(((float)(cx2 + h * (cy1 - cy0) / dist))<0){
					coord[0] = (float)(cx2 + h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 - h * (cx1 - cx0) / dist);
				}else{
					coord[0] = (float)(cx2 - h * (cy1 - cy0) / dist);
					coord[1] = (float)(cy2 + h * (cx1 - cx0 ) / dist);			
				}
			}
		}
		
		
		
		
		return;
	}
}



void TrackerClass::movement(float coord[], uint8_t moveto[])
{
	if (coord[0] != 0){
		if(coord[1]<200 && coord[1]>0){
			digitalWrite(_PIN_Left_F, LOW);
			digitalWrite(_PIN_Right_F, LOW);
			if (coord[0]>50) {
				moveto[0]= 1;
				digitalWrite(_PIN_Left_F, HIGH);
				digitalWrite(_PIN_Right_B, HIGH);
				delay(80);				
				digitalWrite(_PIN_Left_F, LOW);
				digitalWrite(_PIN_Right_B, LOW);
			}else if (coord[0]<-50) {
				moveto[0]= 2;
				digitalWrite(_PIN_Right_F, HIGH);
				digitalWrite(_PIN_Left_B, HIGH);
				delay(80);				
				digitalWrite(_PIN_Right_F, LOW);
				digitalWrite(_PIN_Left_B, LOW);
			}else{
				moveto[0]= 0;
				digitalWrite(_PIN_Left_F, LOW);
				digitalWrite(_PIN_Right_B, LOW);	
				digitalWrite(_PIN_Right_F, LOW);
				digitalWrite(_PIN_Left_B, LOW);
			}
		}else if (coord[1]>200){
			if (coord[0]>200) {
				moveto[0]= 1;
				digitalWrite(_PIN_Left_F, HIGH);
				digitalWrite(_PIN_Right_B, HIGH);
				delay(80);				
				digitalWrite(_PIN_Left_F, LOW);
				digitalWrite(_PIN_Right_B, LOW);
			}else if (coord[0]<-200) {
				moveto[0]= 2;
				digitalWrite(_PIN_Right_F, HIGH);
				digitalWrite(_PIN_Left_B, HIGH);
				delay(80);				
				digitalWrite(_PIN_Right_F, LOW);
				digitalWrite(_PIN_Left_B, LOW);
			}else if (coord[0]>0 && coord[0]<200){
				moveto[0]= 1;
				digitalWrite(_PIN_Left_F, HIGH);
				digitalWrite(_PIN_Right_B, HIGH);
				delay(20);				
				digitalWrite(_PIN_Left_F, LOW);
				digitalWrite(_PIN_Right_B, LOW);
			}else if (coord[0]>-200 && coord[0]<0) {
				moveto[0]= 2;
				digitalWrite(_PIN_Right_F, HIGH);
				digitalWrite(_PIN_Left_B, HIGH);
				delay(20);				
				digitalWrite(_PIN_Right_F, LOW);
				digitalWrite(_PIN_Left_B, LOW);				
			}else{
				moveto[0]= 0;
				digitalWrite(_PIN_Left_F, LOW);
				digitalWrite(_PIN_Right_B, LOW);	
				digitalWrite(_PIN_Right_F, LOW);
				digitalWrite(_PIN_Left_B, LOW);
			}
			digitalWrite(_PIN_Left_F, HIGH);
			digitalWrite(_PIN_Right_F, HIGH);
	}else if (coord[1]<0){
			digitalWrite(_PIN_Left_F, LOW);
			digitalWrite(_PIN_Right_F, LOW);
			if(coord[0]<0){
				digitalWrite(_PIN_Left_F, HIGH);
				digitalWrite(_PIN_Right_B, HIGH);
				delay(80);				
				digitalWrite(_PIN_Left_F, LOW);
				digitalWrite(_PIN_Right_B, LOW);
			}else{
				digitalWrite(_PIN_Right_F, HIGH);
				digitalWrite(_PIN_Left_B, HIGH);
				delay(80);				
				digitalWrite(_PIN_Right_F, LOW);
				digitalWrite(_PIN_Left_B, LOW);
				
			}				
		}
	}	
}
