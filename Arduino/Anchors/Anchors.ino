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

// Pins in Arduino M0 Pro
const uint8_t PIN_RST_FL = 12; // reset pin
const uint8_t PIN_RST_FR = 13; // reset pin
const uint8_t PIN_IRQ_FL = 3; // irq pin
const uint8_t PIN_IRQ_FR = 4; // irq pin
const uint8_t PIN_SS_FL = 6; // spi select pin
const uint8_t PIN_SS_FR = 7; // spi select pin

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

uint8_t Gob = 16;
uint8_t Gab = 17;

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
//For Filter
#define FILTER_LENGTH 12
float filt_list[FILTER_LENGTH];
uint16_t filtCounter;
float avg, sum;

void setup() {
    // Setup Code
    // Begin //Serial communication
    ////Serial1.begin(9600);
    Serial.begin(115200);
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

    // ################# FRONT LEFT####################//
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
    Serial.println("start");

    DW1000FR.receivePermanently(false);
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
    ////Serial.println("Timeout");
}

void resetInactiveFR() {
    // when watchdog times out, reset device
    expectedMsgId = POLL_ACK;
    transmitPollFR();
    noteActivity();
    delay(5);
    receiverFR();
    ////Serial.println("Timeout");
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
    //Serial.println("Sent POLL_FL");
    data[0] = POLL;
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();  
}

void transmitPollFR() {
    DW1000FR.newTransmit();
    DW1000FR.setDefaults();
    //Serial.println("Sent POLL_FR");
    data[0] = POLL; 
    DW1000FR.setData(data, LEN_DATA);
    DW1000FR.startTransmit();
}

void transmitRangeFL() {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    //Serial.println("Send Range_FL");
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
    //Serial.println("Send Range_FR");
    data[0] = RANGE;   
    // delay sending the message and remember expected future sent timestamp
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    timeRangeSent = DW1000FR.setDelay(deltaTime);
    timeRangeSent.getTimestamp(data + 1);
    DW1000FR.setDelay(deltaTime);
    DW1000FR.setData(data, LEN_DATA);
    DW1000FR.startTransmit();
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


// Ranging Algorithm
void computeRangeAsymmetric() {
    // asymmetric two-way ranging (more computation intense, less error prone)
    DW1000Time round1 = (timePollAckReceived - timePollSent).wrap();
    //Serial.print("round1:   "); Serial.println(round1);     
    DW1000Time reply1 = (timePollAckSent - timePollReceived).wrap();
    //Serial.print("reply1:   "); Serial.println(reply1); 
    DW1000Time round2 = (timeRangeReceived - timePollAckSent).wrap();
    //Serial.print("round2:   "); Serial.println(round2); 
    DW1000Time reply2 = (timeRangeSent - timePollAckReceived).wrap();
    //Serial.print("reply2:   "); Serial.println(reply2);     
    DW1000Time tof = (round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2);
    //Serial.print("TIME-OF-FLIGHT:   "); Serial.println(tof); 
    // set tof timestamp
    timeComputedRange.setTimestamp(tof);
}

void loop() {
    // reset if wathcdog timed out
    int32_t curMillis = millis(); // get current time
    if (!sentAck && !receivedAck) {
        if (curMillis - lastActivity > resetPeriod) {
            //Serial.println("WATCHDOG TIMEOUT");
            if (anchorRanging == F_L){
                resetInactiveFL();
            }else if(anchorRanging == F_R){
                resetInactiveFR();            
            }
        }
        return;
    }
    if (anchorRanging == F_L){    
        if (sentAck) {               
            sentAck = false;
            byte msgId = data[0];
            //Serial.print("FRONT-LEFT SENDS:   "); Serial.println(msgId);              
            if (msgId == POLL){
                DW1000FL.getTransmitTimestamp(timePollSent);
                //Serial.print("Sent POLL_FL @ "); Serial.println(timePollSent);                   
                noteActivity();          
            }else if (msgId == RANGE){
                DW1000FL.getTransmitTimestamp(timeRangeSent);
                //Serial.print("Sent RANGE_FL @ "); Serial.println(timeRangeSent);                  
                noteActivity();
            }
        }
        if (receivedAck) {            
            receivedAck = false;
            // get message and parse
            DW1000FL.getData(data, LEN_DATA);
            byte msgId = data[0];            
            //Serial.print("FRONT-LEFT Received:   "); Serial.println(msgId);
            if (msgId != expectedMsgId) {
                // unexpected message, start over again
                expectedMsgId = POLL_ACK;                              
                //transmitPollFL();
                //Serial.print("Received:   "); Serial.print(msgId);Serial.print(" ERROR_FL Expected: "); Serial.println(expectedMsgId);
                return;
            }
            if (msgId == POLL_ACK) {                 
                //protocolFailed = false;
                //Serial.println("Received POLL_ACK_FL");
                DW1000FL.getReceiveTimestamp(timePollAckReceived);
                //Serial.print("Received POLL_ACK_FL @ "); Serial.println(timePollAckReceived);                 
                timePollReceived.setTimestamp(data + 1);
                //Serial.print("Received POLL_FL @ "); Serial.println(timePollReceived);
                timePollAckSent.setTimestamp(data + 6);
                //Serial.print("Sent POLL_ACK_FL @ "); Serial.println(timePollAckSent);                        
                expectedMsgId = RANGE_ACK;
                transmitRangeFL();
                DW1000FL.receivePermanently(false);
                DW1000FR.receivePermanently(true);
                noteActivity();
            } else if (msgId == RANGE_ACK) {
                    //Serial.println("Received RANGE_ACK_FL");          
                    timeRangeReceived.setTimestamp(data + 1);
                    //Serial.print("Received RANGE_FL @ "); Serial.println(timeRangeReceived);                      
                    computeRangeAsymmetric();  
                    float distance = timeComputedRange.getAsMeters()*100;
                    float avg_distance = 15;
                    String Serialdata = "FRONT-LEFT: 0," + String(distance) + ",0," + String(avg_distance) + "," + String(samplingRate) + "," + String(DW1000FL.getReceivePower()) + "," + String(DW1000FL.getReceiveQuality()) + "\n\r";                
                    Serial.print(Serialdata);    
                    successRangingCount++;
                    if (curMillis - rangingCountPeriod > 1000) {
                        samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
                        rangingCountPeriod = curMillis;
                        successRangingCount = 0;
                    }                              
                    anchorRanging = F_R;         
                    expectedMsgId = POLL_ACK;
                    //DW1000FR.clearAllStatus();                    
                    //DW1000FL.receivePermanently(false);
                    //DW1000FR.receivePermanently(true);
                    SPI.usingInterrupt(digitalPinToInterrupt(PIN_IRQ_FR));
                    attachInterrupt(digitalPinToInterrupt(PIN_IRQ_FR), DW1000FR.handleInterrupt, RISING);                    
                    //delay(100); 
                    transmitPollFR();                                             
                    noteActivity();   
            } else if (msgId == RANGE_FAILED) {
                //Serial.println("Received Range_FAILED_FL");                       
                expectedMsgId = POLL_ACK;
                transmitPollFL();
                noteActivity();
            }
        }
    }else if (anchorRanging == F_R){
        if (sentAck) {
            sentAck = false;
            byte msgId = data[0];
            //Serial.print("FRONT-RIGHT SENDS:   "); Serial.println(msgId);             
            if (msgId == POLL){
                DW1000FR.getTransmitTimestamp(timePollSent);
                //Serial.print("Sent POLL_FR @ "); Serial.println(timePollSent);                 
                noteActivity();          
            }else if (msgId == RANGE){
                DW1000FR.getTransmitTimestamp(timeRangeSent);
                //Serial.print("Sent RANGE_FR @ "); Serial.println(timeRangeSent);                   
                noteActivity();
            }
        }
        if (receivedAck) {
            receivedAck = false;
            // get message and parse
            DW1000FR.getData(data, LEN_DATA);
            byte msgId = data[0];
            
            //Serial.print("FRONT-RIGHT Received:   "); Serial.println(msgId);
            if (msgId != expectedMsgId) {
                // unexpected message, start over again
                expectedMsgId = POLL_ACK;
                //transmitPollFR();
                //Serial.print("Received:   "); Serial.print(msgId); Serial.print(" ERROR_FR Expected: "); Serial.println(expectedMsgId);            
                return;                
            }
            if (msgId == POLL_ACK) { 
                //protocolFailed = false;
                //Serial.println("Received POLL_ACK_FR");
                DW1000FR.getReceiveTimestamp(timePollAckReceived);
                //Serial.print("Received POLL_ACK_FR @ "); Serial.println(timePollAckReceived);                 
                timePollReceived.setTimestamp(data + 1);
                //Serial.print("Received POLL_FR @ "); Serial.println(timePollReceived);
                timePollAckSent.setTimestamp(data + 6);
                //Serial.print("Sent POLL_ACK_FR @ "); Serial.println(timePollAckSent);                        
                expectedMsgId = RANGE_ACK;
                transmitRangeFR();
                DW1000FR.receivePermanently(false); 
                DW1000FL.receivePermanently(true);                                    
                noteActivity();
            } else if (msgId == RANGE_ACK) {
                    //Serial.println("Received RANGE_ACK_FR");
                    //Serial.print("Second value"); Serial.println(data[2]);            
                    timeRangeReceived.setTimestamp(data + 1);
                    //Serial.print("Received RANGE_FR @ "); Serial.println(timeRangeReceived);                     
                    computeRangeAsymmetric();  
                    float distance = timeComputedRange.getAsMeters()*100;
                    float avg_distance = 15;
                    String Serialdata = "FRONT-RIGHT: 0," + String(distance) + ",0," + String(avg_distance) + "," + String(samplingRate) + "," + String(DW1000FR.getReceivePower()) + "," + String(DW1000FR.getReceiveQuality()) + "\n\r";                
                    Serial.print(Serialdata);    
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
                //Serial.println("Received Range_FAILED_FR");                                     
                expectedMsgId = POLL_ACK;
                transmitPollFR();
                noteActivity();
            }
        }
    }
 }
    
