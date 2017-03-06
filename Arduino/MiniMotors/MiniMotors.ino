// Wire Slave Receiver
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Receives data as an I2C/TWI slave device
// Refer to the "Wire Master Writer" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

float coord[2];

uint8_t counter = 0;
bool newdata = false;

//Movement PINS
uint8_t _PIN_Left_F = 9;
uint8_t _PIN_Right_F = 7;
uint8_t _PIN_Left_B = 8;
uint8_t _PIN_Right_B = 6;


void setup() {
  Wire.begin(4);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);           // start serial for output
  coord[0] = 0;
  coord[1] = 0;
      //Movement setup
    pinMode(_PIN_Left_F, OUTPUT); // Leflt Forward
    pinMode(_PIN_Right_F, OUTPUT); // Right Forward
    pinMode(_PIN_Left_B, OUTPUT); // Left backwards
    pinMode(_PIN_Right_B, OUTPUT); //  right barckwards   
}

void loop() {
    if (coord[0] != 0){
        if (coord[1]<160 && coord[1]>0){      
            digitalWrite(_PIN_Left_F, LOW);
            digitalWrite(_PIN_Right_F, LOW); 
            digitalWrite(_PIN_Left_B, LOW);
            digitalWrite(_PIN_Right_B, LOW);  
            if (coord[0]>60) {
               digitalWrite(_PIN_Left_F, HIGH);
               digitalWrite(_PIN_Right_B, HIGH);     
               delay(120);
               digitalWrite(_PIN_Left_F, LOW);
               digitalWrite(_PIN_Right_B, LOW);    
            }else if (coord[0]<-60) {
               digitalWrite(_PIN_Right_F, HIGH);
               digitalWrite(_PIN_Left_B, HIGH);
               delay(120);
               digitalWrite(_PIN_Right_F, LOW);
               digitalWrite(_PIN_Left_B, LOW);   
            }else{
               digitalWrite(_PIN_Right_F, LOW);
               digitalWrite(_PIN_Left_B, LOW);
               digitalWrite(_PIN_Left_F, LOW);
               digitalWrite(_PIN_Right_B, LOW);               
            }       
            delay(100);               
        }else if (coord[1]<0){
            digitalWrite(_PIN_Left_F, LOW);
            digitalWrite(_PIN_Right_F, LOW);
            if(coord[0]<0){
                digitalWrite(_PIN_Right_F, HIGH);
                digitalWrite(_PIN_Left_B, HIGH);
                delay(120);        
                digitalWrite(_PIN_Right_F, LOW);
                digitalWrite(_PIN_Left_B, LOW);                   
            }else{
                digitalWrite(_PIN_Left_F, HIGH);
                digitalWrite(_PIN_Right_B, HIGH);
                delay(120);        
                digitalWrite(_PIN_Left_F, LOW);
                digitalWrite(_PIN_Right_B, LOW);
            }
            delay(100);           
        }else if(coord[1] > 160){
            digitalWrite(_PIN_Left_F, LOW);
            digitalWrite(_PIN_Right_F, LOW); 
            digitalWrite(_PIN_Left_B, LOW);
            digitalWrite(_PIN_Right_B, LOW);  
            if (coord[0]>150) {
               digitalWrite(_PIN_Left_F, HIGH);
               //digitalWrite(_PIN_Right_B, HIGH);     
               delay(100);
               digitalWrite(_PIN_Left_F, LOW);
               //digitalWrite(_PIN_Right_B, LOW);    
            }else if (coord[0]<-150) {
               digitalWrite(_PIN_Right_F, HIGH);
               //digitalWrite(_PIN_Left_B, HIGH);
               delay(100);
               digitalWrite(_PIN_Right_F, LOW);
               //digitalWrite(_PIN_Left_B, LOW);   
            }else if (coord[0]<-50 && coord[0]>-150) {
               digitalWrite(_PIN_Right_F, HIGH);
               //digitalWrite(_PIN_Left_B, HIGH);
               delay(80);
               digitalWrite(_PIN_Right_F, LOW);
               //digitalWrite(_PIN_Left_B, LOW);   
            }else if (coord[0]<150 && coord[0]>50) {
               digitalWrite(_PIN_Left_F, HIGH);
               //digitalWrite(_PIN_Right_B, HIGH);     
               delay(80);
               digitalWrite(_PIN_Left_F, LOW);
               //digitalWrite(_PIN_Right_B, LOW);    
            }else{
               digitalWrite(_PIN_Right_F, LOW);
               digitalWrite(_PIN_Left_B, LOW);
               digitalWrite(_PIN_Left_F, LOW);
               digitalWrite(_PIN_Right_B, LOW);               
            }            
           digitalWrite(_PIN_Left_F, HIGH);
           digitalWrite(_PIN_Right_F, HIGH);
           delay(500); 
        }                                 
    }
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
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
  String SerialUSBdata = "(" + String(coord[0]) + "," + String(coord[1]) + ")";
  Serial.println(SerialUSBdata);  

}
