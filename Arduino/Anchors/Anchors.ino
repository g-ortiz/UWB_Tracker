/*
 * UWB - Tracker
 * Anchors
 * Gabriel Ortiz 
 * Fredrik Treven
 * Adapted from: Decawave DW1000 library for arduino RangingAchor example.
 */

#include <SPI.h>
#include <DW1000FL.h>
#include <DW1000FR.h>
#include <DW1000RR.h>
#include <DW1000RL.h>
#include <Tracker.h>


// Pins in Arduino M0 Pro
const uint8_t PIN_RST_FL = 7; // reset pin
const uint8_t PIN_IRQ_FL = 5; // irq pin
const uint8_t PIN_SS_FL = 6; // spi select pin
const uint8_t PIN_RST_FR = 13; // reset pin
const uint8_t PIN_IRQ_FR = 11; // irq pin
const uint8_t PIN_SS_FR = 12; // spi select pin
const uint8_t PIN_RST_RR = A3; // reset pin
const uint8_t PIN_IRQ_RR = A5; // irq pin
const uint8_t PIN_SS_RR = A4; // spi select pin
const uint8_t PIN_RST_RL = A0; // reset pin
const uint8_t PIN_IRQ_RL = A2; // irq pin
const uint8_t PIN_SS_RL = A1; // spi select pin

const uint8_t PIN_Left_F = 9;
const uint8_t PIN_Right_F = 8;
const uint8_t PIN_Left_B = 4;
const uint8_t PIN_Right_B = 3;


// Expected messages FL
#define POLL 0
#define POLL_ACK 1
#define RANGE 2
#define RANGE_ACK 3
#define RANGE_FAILED 255


// Anchor Names
#define F_L 0
#define F_R 1
#define R_R 2
#define R_L 3

float coords[4];
uint8_t moveto[2];
float ranges[4];

// Receiving anchor
uint8_t anchorRanging = F_L;
// message flow state
volatile byte expectedMsgId = POLL_ACK;
// message sent/received state
volatile boolean sentAck = false;
volatile boolean receivedAck = false;
// protocol error state
boolean protocolFailed = false;
// timestamps
DW1000Time timePollSent;
DW1000Time timePollReceived;
DW1000Time timePollAckSent;
DW1000Time timePollAckReceived;
DW1000Time timeRangeSent;
DW1000Time timeRangeReceived;
// last computed range/time
DW1000Time timeComputedRange;
// data buffer
#define LEN_DATA 16
byte data[LEN_DATA];
// watchdog and reset period
uint32_t lastActivity;
uint32_t resetPeriod = 250;
// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;
// ranging counter (per second)
uint16_t successRangingCount = 0;
uint32_t rangingCountPeriod = 0;
float samplingRate = 0;

uint32_t movementPeriod = 0;

void setup() {
    // Setup Code
    // Begin //SerialUSB communication
    ////SerialUSB1.begin(9600);
    SerialUSB.begin(115200);
    delay(1000);
    // ################# FRONT LEFT####################//
    DW1000FL.begin(PIN_IRQ_FL, PIN_RST_FL);    
    DW1000FL.select(PIN_SS_FL); //    
    DW1000FL.newConfiguration();
    DW1000FL.setDefaults();
    DW1000FL.setDeviceAddress(1);
    DW1000FL.setNetworkId(12);
    DW1000FL.enableMode(DW1000FL.MODE_LONGDATA_RANGE_LOWPOWER);
    DW1000FL.commitConfiguration();
    DW1000FL.enableDebounceClock();
    DW1000FL.enableLedBlinking();
    // set function callbacks for sent and received messages
    DW1000FL.attachSentHandler(handleSent);
    DW1000FL.attachReceivedHandler(handleReceived);    

    // ################# FRONT RIGHT####################//
    DW1000FR.begin(PIN_IRQ_FR, PIN_RST_FR);    
    DW1000FR.select(PIN_SS_FR); //    
    DW1000FR.newConfiguration();
    DW1000FR.setDefaults();
    DW1000FR.setDeviceAddress(5);
    DW1000FR.setNetworkId(12);
    DW1000FR.enableMode(DW1000FR.MODE_LONGDATA_RANGE_LOWPOWER);
    DW1000FR.commitConfiguration();
    DW1000FR.enableDebounceClock();
    DW1000FR.enableLedBlinking();
    // set function callbacks for sent and received messages
    DW1000FR.attachSentHandler(handleSent);
    DW1000FR.attachReceivedHandler(handleReceived);    


    // ################# REAR RIGHT####################//
    DW1000RR.begin(PIN_IRQ_RR, PIN_RST_RR);    
    DW1000RR.select(PIN_SS_RR); //    
    DW1000RR.newConfiguration();
    DW1000RR.setDefaults();
    DW1000RR.setDeviceAddress(6);
    DW1000RR.setNetworkId(12);
    DW1000RR.enableMode(DW1000RR.MODE_LONGDATA_RANGE_LOWPOWER);
    DW1000RR.commitConfiguration();
    DW1000RR.enableDebounceClock();
    DW1000RR.enableLedBlinking();
    // set function callbacks for sent and received messages
    DW1000RR.attachSentHandler(handleSent);
    DW1000RR.attachReceivedHandler(handleReceived);    

    // ################# REAR LEFT####################//
    DW1000RL.begin(PIN_IRQ_RL, PIN_RST_RL);    
    DW1000RL.select(PIN_SS_RL); //    
    DW1000RL.newConfiguration();
    DW1000RL.setDefaults();
    DW1000RL.setDeviceAddress(7);
    DW1000RL.setNetworkId(12);
    DW1000RL.enableMode(DW1000RL.MODE_LONGDATA_RANGE_LOWPOWER);
    DW1000RL.commitConfiguration();
    DW1000RL.enableDebounceClock();
    DW1000RL.enableLedBlinking();
    // set function callbacks for sent and received messages
    DW1000RL.attachSentHandler(handleSent);
    DW1000RL.attachReceivedHandler(handleReceived);            


    //Movement setup
    pinMode(PIN_Left_F, OUTPUT); // Leflt Forward
    pinMode(PIN_Right_F, OUTPUT); // Right Forward
    pinMode(PIN_Left_B, OUTPUT); // Left backwards
    pinMode(PIN_Right_B, OUTPUT); //  right barckwards   
  
    SerialUSB.println("start");

	  //Initialize filter and multilateration   
    Tracker.initTracker();    
    coords[0] = 0;
    coords[1] = 0;
    DW1000FR.receivePermanently(false);
    DW1000RR.receivePermanently(false);   
    DW1000RL.receivePermanently(false);        
    // start receive mode, wait for POLL message
    SPI.usingInterrupt(digitalPinToInterrupt(PIN_IRQ_FL));
    attachInterrupt(digitalPinToInterrupt(PIN_IRQ_FL), DW1000FL.handleInterrupt, RISING);    
    receiverFL();
    transmitPollFL();
    // reset watchdog
    noteActivity();
    // for first time ranging frequency computation
    rangingCountPeriod = millis();
}

void noteActivity() {
    // reset watchdog
    lastActivity = millis();
}

void resetInactiveFL() {
    // when watchdog times out, reset device
    expectedMsgId = POLL_ACK;
    transmitPollFL();
    noteActivity();
    delay(5);    
    receiverFL();
    ////SerialUSB.println("Timeout");
}

void resetInactiveFR() {
    // when watchdog times out, reset device
    expectedMsgId = POLL_ACK;
    transmitPollFR();
    noteActivity();
    delay(5);
    receiverFR();
    ////SerialUSB.println("Timeout");
}

void resetInactiveRR() {
    // when watchdog times out, reset device
    expectedMsgId = POLL_ACK;
    transmitPollRR();
    noteActivity();
    delay(5);
    receiverRR();
    ////SerialUSB.println("Timeout");
}

void resetInactiveRL() {
    // when watchdog times out, reset device
    expectedMsgId = POLL_ACK;
    transmitPollRL();
    noteActivity();
    delay(5);
    receiverRL();
    ////SerialUSB.println("Timeout");
}

void handleSent() {
    // change state when ACK sent
    sentAck = true;
}

void handleReceived() {
    // change state when ACK received
    receivedAck = true;
}

void transmitPollFL() {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    //SerialUSB.println("Sent POLL_FL");
    data[0] = POLL;
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();  
}

void transmitPollFR() {
    DW1000FR.newTransmit();
    DW1000FR.setDefaults();
    //SerialUSB.println("Sent POLL_FR");
    data[0] = POLL; 
    DW1000FR.setData(data, LEN_DATA);
    DW1000FR.startTransmit();
}

void transmitPollRR() {
    DW1000RR.newTransmit();
    DW1000RR.setDefaults();
    //SerialUSB.println("Sent POLL_RR");
    data[0] = POLL; 
    DW1000RR.setData(data, LEN_DATA);
    DW1000RR.startTransmit();
}

void transmitPollRL() {
    DW1000RL.newTransmit();
    DW1000RL.setDefaults();
    //SerialUSB.println("Sent POLL_RL");
    data[0] = POLL; 
    DW1000RL.setData(data, LEN_DATA);
    DW1000RL.startTransmit();
}

void transmitRangeFL() {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    //SerialUSB.println("Send Range_FL");
    data[0] = RANGE;
    // delay sending the message and remember expected future sent timestamp
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    timeRangeSent = DW1000FL.setDelay(deltaTime);
    timeRangeSent.getTimestamp(data + 1);
    DW1000FL.setDelay(deltaTime);
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();
}

void transmitRangeFR() {
    DW1000FR.newTransmit();
    DW1000FR.setDefaults();
    //SerialUSB.println("Send Range_FR");
    data[0] = RANGE;   
    // delay sending the message and remember expected future sent timestamp
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    timeRangeSent = DW1000FR.setDelay(deltaTime);
    timeRangeSent.getTimestamp(data + 1);
    DW1000FR.setDelay(deltaTime);
    DW1000FR.setData(data, LEN_DATA);
    DW1000FR.startTransmit();
}

void transmitRangeRR() {
    DW1000RR.newTransmit();
    DW1000RR.setDefaults();
    //SerialUSB.println("Send Range_RR");
    data[0] = RANGE;   
    // delay sending the message and remember expected future sent timestamp
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    timeRangeSent = DW1000RR.setDelay(deltaTime);
    timeRangeSent.getTimestamp(data + 1);
    DW1000RR.setDelay(deltaTime);
    DW1000RR.setData(data, LEN_DATA);
    DW1000RR.startTransmit();
}

void transmitRangeRL() {
    DW1000RL.newTransmit();
    DW1000RL.setDefaults();
    //SerialUSB.println("Send Range_RL");
    data[0] = RANGE;   
    // delay sending the message and remember expected future sent timestamp
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    timeRangeSent = DW1000RL.setDelay(deltaTime);
    timeRangeSent.getTimestamp(data + 1);
    DW1000RL.setDelay(deltaTime);
    DW1000RL.setData(data, LEN_DATA);
    DW1000RL.startTransmit();
}

void receiverFL() {
    DW1000FL.newReceive();
    DW1000FL.setDefaults();
    // Enable receiver
    DW1000FL.receivePermanently(true);
    DW1000FL.startReceive();
}

void receiverFR() {
    DW1000FR.newReceive();
    DW1000FR.setDefaults();
    // Enable receiver
    DW1000FR.receivePermanently(true);
    DW1000FR.startReceive();
}

void receiverRR() {
    DW1000RR.newReceive();
    DW1000RR.setDefaults();
    // Enable receiver
    DW1000RR.receivePermanently(true);
    DW1000RR.startReceive();
}

void receiverRL() {
    DW1000RL.newReceive();
    DW1000RL.setDefaults();
    // Enable receiver
    DW1000RL.receivePermanently(true);
    DW1000RL.startReceive();
}

// Ranging Algorithm
void computeRangeAsymmetric() {
    // asymmetric two-way ranging (more computation intense, less error prone)
    DW1000Time round1 = (timePollAckReceived - timePollSent).wrap();  
    DW1000Time reply1 = (timePollAckSent - timePollReceived).wrap();
    DW1000Time round2 = (timeRangeReceived - timePollAckSent).wrap();
    DW1000Time reply2 = (timeRangeSent - timePollAckReceived).wrap();  
    DW1000Time tof = (round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2);
    // set tof timestamp
    timeComputedRange.setTimestamp(tof);
}

void loop() {
    // reset if wathcdog timed out
    int32_t curMillis = millis(); // get current time
    if (!sentAck && !receivedAck) {
        if (curMillis - lastActivity > resetPeriod) {
            //SerialUSB.println("WATCHDOG TIMEOUT");
            if (anchorRanging == F_L){
                resetInactiveFL();
            }else if(anchorRanging == F_R){
                resetInactiveFR();            
            }else if(anchorRanging == R_R){
                resetInactiveRR();            
            }else if(anchorRanging == R_L){
                resetInactiveRL();            
            }
        }
        return;
    }
    if (anchorRanging == F_L){    
        if (sentAck) {               
            sentAck = false;
            byte msgId = data[0];             
            if (msgId == POLL){
                DW1000FL.getTransmitTimestamp(timePollSent);            
                noteActivity();          
            }else if (msgId == RANGE){
                DW1000FL.getTransmitTimestamp(timeRangeSent);              
                noteActivity();
            }
        }
        if (receivedAck) {            
            receivedAck = false;
            // get message and parse
            DW1000FL.getData(data, LEN_DATA);
            byte msgId = data[0];            
            if (msgId != expectedMsgId) {
                expectedMsgId = POLL_ACK;                              
                return;
            }
            if (msgId == POLL_ACK) {                 
                DW1000FL.getReceiveTimestamp(timePollAckReceived);              
                timePollReceived.setTimestamp(data + 1);
                timePollAckSent.setTimestamp(data + 6);                  
                expectedMsgId = RANGE_ACK;
                transmitRangeFL();
                DW1000FL.receivePermanently(false);
                DW1000FR.receivePermanently(true);
                noteActivity();
            } else if (msgId == RANGE_ACK) {          
                    timeRangeReceived.setTimestamp(data + 1);             
                    computeRangeAsymmetric();  
                    float distance = timeComputedRange.getAsMeters()*100;
                    ranges[0] = Tracker.filter(distance ,F_L, coords);                                  
                    anchorRanging = F_R;
                    expectedMsgId = POLL_ACK;
                    SPI.usingInterrupt(digitalPinToInterrupt(PIN_IRQ_FR));
                    attachInterrupt(digitalPinToInterrupt(PIN_IRQ_FR), DW1000FR.handleInterrupt, RISING);                    
                    //delay(100); 
                    transmitPollFR();                                             
                    noteActivity();   
            } else if (msgId == RANGE_FAILED) {
                //SerialUSB.println("Received Range_FAILED_FL");                       
                expectedMsgId = POLL_ACK;
                transmitPollFL();
                noteActivity();
            }
        }
    }else if (anchorRanging == F_R){
        if (sentAck) {
            sentAck = false;
            byte msgId = data[0];      
            if (msgId == POLL){
                DW1000FR.getTransmitTimestamp(timePollSent);            
                noteActivity();          
            }else if (msgId == RANGE){
                DW1000FR.getTransmitTimestamp(timeRangeSent);           
                noteActivity();
            }
        }
        if (receivedAck) {
            receivedAck = false;
            // get message and parse
            DW1000FR.getData(data, LEN_DATA);
            byte msgId = data[0];
            if (msgId != expectedMsgId) {
                // unexpected message, start over again
                expectedMsgId = POLL_ACK;
                return;                
            }
            if (msgId == POLL_ACK) { 
                DW1000FR.getReceiveTimestamp(timePollAckReceived);            
                timePollReceived.setTimestamp(data + 1);
                timePollAckSent.setTimestamp(data + 6);                 
                expectedMsgId = RANGE_ACK;
                transmitRangeFR();
                DW1000FR.receivePermanently(false); 
                DW1000RR.receivePermanently(true);                                    
                noteActivity();
            } else if (msgId == RANGE_ACK) {           
                    timeRangeReceived.setTimestamp(data + 1);                
                    computeRangeAsymmetric();  
                    float distance = timeComputedRange.getAsMeters()*100;
                    ranges[1] = Tracker.filter(distance , F_R, coords);                                
                    anchorRanging = R_R;          
                    expectedMsgId = POLL_ACK;                                 
                    SPI.usingInterrupt(digitalPinToInterrupt(PIN_IRQ_RR));
                    attachInterrupt(digitalPinToInterrupt(PIN_IRQ_RR), DW1000RR.handleInterrupt, RISING);             
                    transmitPollRR();                   
                    noteActivity();
            } else if (msgId == RANGE_FAILED) {                                  
                expectedMsgId = POLL_ACK;
                transmitPollFR();
                noteActivity();
            }
        }
	  }else if (anchorRanging == R_R){
        if (sentAck) {
            sentAck = false;
            byte msgId = data[0];          
            if (msgId == POLL){
                DW1000RR.getTransmitTimestamp(timePollSent);              
                noteActivity();          
            }else if (msgId == RANGE){
                DW1000RR.getTransmitTimestamp(timeRangeSent);         
                noteActivity();
            }
        }
        if (receivedAck) {
            receivedAck = false;
            // get message and parse
            DW1000RR.getData(data, LEN_DATA);
            byte msgId = data[0];
            if (msgId != expectedMsgId) {
                // unexpected message, start over again
                expectedMsgId = POLL_ACK;      
                return;                
            }
            if (msgId == POLL_ACK) { 
                DW1000RR.getReceiveTimestamp(timePollAckReceived);             
                timePollReceived.setTimestamp(data + 1);
                timePollAckSent.setTimestamp(data + 6);                      
                expectedMsgId = RANGE_ACK;
                transmitRangeRR();
                DW1000RR.receivePermanently(false); 
                DW1000RL.receivePermanently(true);                                    
                noteActivity();
            } else if (msgId == RANGE_ACK) {          
                    timeRangeReceived.setTimestamp(data + 1);                  
                    computeRangeAsymmetric();  
                    float distance = timeComputedRange.getAsMeters()*100;
                    ranges[2] = Tracker.filter(distance ,R_R, coords);                              
                    anchorRanging = R_L;          
                    expectedMsgId = POLL_ACK;   
                    //DW1000FL.clearAllStatus();                               
                    SPI.usingInterrupt(digitalPinToInterrupt(PIN_IRQ_RL));
                    attachInterrupt(digitalPinToInterrupt(PIN_IRQ_RL), DW1000RL.handleInterrupt, RISING);
                    //delay(100);                 
                    transmitPollRL();                   
                    noteActivity();
            } else if (msgId == RANGE_FAILED) {   
                //SerialUSB.println("Received Range_FAILED_RR");                                     
                expectedMsgId = POLL_ACK;
                transmitPollRR();
                noteActivity();
            }
        }
	  }else if (anchorRanging == R_L){
        if (sentAck) {
            sentAck = false;
            byte msgId = data[0];      
            if (msgId == POLL){
                DW1000RL.getTransmitTimestamp(timePollSent);            
                noteActivity();          
            }else if (msgId == RANGE){
                DW1000RL.getTransmitTimestamp(timeRangeSent);              
                noteActivity();
            }
        }
        if (receivedAck) {
            receivedAck = false;
            // get message and parse
            DW1000RL.getData(data, LEN_DATA);
            byte msgId = data[0];
            if (msgId != expectedMsgId) {
                // unexpected message, start over again
                expectedMsgId = POLL_ACK;     
                return;                
            }
            if (msgId == POLL_ACK) { 
                DW1000RL.getReceiveTimestamp(timePollAckReceived);            
                timePollReceived.setTimestamp(data + 1);
                timePollAckSent.setTimestamp(data + 6);                      
                expectedMsgId = RANGE_ACK;
                transmitRangeRL();
                DW1000RL.receivePermanently(false); 
                DW1000FL.receivePermanently(true);                                    
                noteActivity();
            } else if (msgId == RANGE_ACK) {  
                    timeRangeReceived.setTimestamp(data + 1);                   
                    computeRangeAsymmetric();  
                    float distance = timeComputedRange.getAsMeters()*100;
                    ranges[3] = Tracker.filter(distance , R_L, coords);   
                    //if (curMillis - movementPeriod > 200){
                        //Tracker.movement(coords+2, moveto);
                      //  movementPeriod = curMillis;
                    //}            
                    String SerialUSBdata = "0," + String(distance) + "," + String(samplingRate) + "," + String(moveto[0]) + "," + String(moveto[1])
                             + "," + String(ranges[0]) + "," + String(ranges[1]) + "," + String(ranges[2]) + "," + String(ranges[3]) + "," + String(coords[0]) + "," + String(coords[1])
                             + "," + String(coords[2]) + "," + String(coords[3]) + "\n\r";                
                    SerialUSB.print(SerialUSBdata);                                        
                    successRangingCount++;
                    if (curMillis - rangingCountPeriod > 1000) {
                        samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
                        rangingCountPeriod = curMillis;
                        successRangingCount = 0;
                    }                              
                    anchorRanging = F_L;          
                    expectedMsgId = POLL_ACK;   
                    //DW1000FL.clearAllStatus();                               
                    SPI.usingInterrupt(digitalPinToInterrupt(PIN_IRQ_FL));
                    attachInterrupt(digitalPinToInterrupt(PIN_IRQ_FL), DW1000FL.handleInterrupt, RISING);
                    //delay(100);                 
                    transmitPollFL();                   
                    noteActivity();
            } else if (msgId == RANGE_FAILED) {   
                //SerialUSB.println("Received Range_FAILED_RL");                                     
                expectedMsgId = POLL_ACK;
                transmitPollRL();
                noteActivity();
            }
        }
   }   	
 }



