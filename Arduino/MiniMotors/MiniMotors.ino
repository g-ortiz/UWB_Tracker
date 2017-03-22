/*
--UWB Tracker
--Motor Controller
--Gabriel Ortiz
--Fredrik Treven
*/

#include <Wire.h>
#include <Math.h>


float coord[2];

uint8_t counter = 0;
bool newdata = false;

bool modeFront = true;

//Movement PINS
uint8_t _PIN_Left_F = 5;
uint8_t _PIN_Right_F = 3;
uint8_t _PIN_Left_B = 9;
uint8_t _PIN_Right_B = 6;

uint32_t lastActivity;
uint32_t resetPeriod = 60;

float angle = 0;

void setup() {
  Wire.begin(4);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(115200);         // start serial 
  coord[0] = 0;
  coord[1] = 0;
  
    //Movement setup
  pinMode(_PIN_Left_F, OUTPUT);  // Left Forward
  pinMode(_PIN_Right_F, OUTPUT); // Right Forward
  pinMode(_PIN_Left_B, OUTPUT);  // Left backwards
  pinMode(_PIN_Right_B, OUTPUT); //  right barckwards   
}

void loop() {
    int32_t curMillis = millis();
    if (curMillis - lastActivity > resetPeriod) { 
          analogWrite(_PIN_Right_F, 0);
          analogWrite(_PIN_Left_B, 0);
          analogWrite(_PIN_Left_F, 0);
          analogWrite(_PIN_Right_B, 0);
          //Serial.println("Timeout!");
    }
    if (coord[0] != 0 && newdata){
        newdata = false;
        angle = atan2(coord[1] - 0, coord[0] - 0 );
        angle = angle * 180 / PI;
        //Serial.println(angle);
        if (angle>50 && angle<130){
          modeFront = true;
        }else if (angle<-50 && angle>-130){
          modeFront = false; 
        }
        if (coord[1]<=170 && modeFront){
            if(angle>=0 && angle<=70){
                    float NewValue = (((angle - 0) * (180 - 255)) / (70 - 0)) + 255;
                    analogWrite(_PIN_Left_F, NewValue);
                    analogWrite(_PIN_Right_B, NewValue);
                    //Serial.println(NewValue);
            }else if (angle>=110 && angle<=180){
                    float NewValue = (((angle - 110) * (255 - 180)) / (180 - 110)) + 180;
                    analogWrite(_PIN_Right_F, NewValue);
                    analogWrite(_PIN_Left_B, NewValue);
                    //Serial.println(NewValue);           
            }else{
                    analogWrite(_PIN_Right_F, 0);
                    analogWrite(_PIN_Left_B, 0);
                    analogWrite(_PIN_Left_F, 0);
                    analogWrite(_PIN_Right_B, 0);
            }
            lastActivity = millis();
        }else if(coord[1]>170 && modeFront){
            analogWrite(_PIN_Right_B, 0);            
            analogWrite(_PIN_Left_B, 0);
            if(angle>=0 && angle<70){
                    float NewValue = 255;
                    analogWrite(_PIN_Left_F, NewValue);
                    //Serial.println(NewValue);  
                    NewValue = 220;
                    analogWrite(_PIN_Right_F, NewValue);          
                    //Serial.println(NewValue);  
            }else if (angle>110 && angle<=180){
                    float NewValue = 255;
                    analogWrite(_PIN_Right_F, NewValue); 
                    //Serial.println(NewValue);  
                    NewValue = 220;
                    analogWrite(_PIN_Left_F, NewValue);                            
                    //Serial.println(NewValue);  
            }else if (angle>=70 && angle<=110){
                    float NewValue = 220;
                    analogWrite(_PIN_Right_F, NewValue); 
                    analogWrite(_PIN_Left_F, NewValue);                            
                    //Serial.println(NewValue);  
            }
            lastActivity = millis();
        }else if (coord[1]>=-170 && !modeFront){
            if(angle<=0 && angle>=-70){
                    float NewValue = (((angle - (0)) * (180 - 255)) / ((-70) - 0)) + 255;
                    analogWrite(_PIN_Left_B, NewValue);
                    analogWrite(_PIN_Right_F, NewValue);
                    //Serial.println(NewValue);
            }else if (angle<=-110 && angle>=-180){
                    float NewValue = (((angle - (-110)) * (255 - 110)) / ((-180) - (-110))) + 180;
                    analogWrite(_PIN_Right_B, NewValue);
                    analogWrite(_PIN_Left_F, NewValue);
                    //Serial.println(NewValue);           
            }else{
                    analogWrite(_PIN_Right_F, 0);
                    analogWrite(_PIN_Left_B, 0);
                    analogWrite(_PIN_Left_F, 0);
                    analogWrite(_PIN_Right_B, 0);
            }
            lastActivity = millis();
        }else if(coord[1]<-170 && !modeFront){
            analogWrite(_PIN_Right_F, 0);            
            analogWrite(_PIN_Left_F, 0);
            if(angle<0 && angle>-70){
                    float NewValue = 255;
                    analogWrite(_PIN_Left_B, NewValue);
                    //Serial.println(NewValue);  
                    NewValue = 220;
                    analogWrite(_PIN_Right_B, NewValue);          
                    //Serial.println(NewValue);  
            }else if (angle<-110 && angle>-180){
                    float NewValue = 255;
                    analogWrite(_PIN_Right_B, NewValue); 
                    //Serial.println(NewValue);  
                    NewValue = 220;
                    analogWrite(_PIN_Left_B, NewValue);                            
                    //Serial.println(NewValue);  
            }else if (angle<=-70 && angle>=-110){
                    float NewValue = 220;
                    analogWrite(_PIN_Right_B, NewValue); 
                    analogWrite(_PIN_Left_B, NewValue);                            
                    //Serial.println(NewValue);  
            }
            lastActivity = millis();
        }
    }
}


// function that executes whenever data is received from master
void receiveEvent(int howMany) {
  newdata = true;
  byte dataX[4];
  dataX[0]= Wire.read();// receive byte as an integer
  dataX[1]= Wire.read();// receive byte as an integer
  dataX[2]= Wire.read();// receive byte as an integer
  dataX[3]= Wire.read();// receive byte as an integer
  byte dataY[4];
  dataY[0]= Wire.read();// receive byte as an integer
  dataY[1]= Wire.read();// receive byte as an integer
  dataY[2]= Wire.read();// receive byte as an integer
  dataY[3]= Wire.read();// receive byte as an integer

  coord[0] = *(float *)&dataX;
  coord[1] = *(float *)&dataY;
  
  //String SerialUSBdata = "(" + String(coord[0]) + "," + String(coord[1]) + ")";
  //Serial.println(SerialUSBdata);  
}

