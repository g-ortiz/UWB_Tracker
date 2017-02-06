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
#define POLL_FL 10
#define POLL_ACK_FL 11
#define RANGE_FL 12
#define RANGE_ACK_FL 13
#define RANGE_FAILED_FL 255

// Expected messages FR
#define POLL_FR 20
#define POLL_ACK_FR 21
#define RANGE_FR 22
#define RANGE_ACK_FR 23
#define RANGE_FAILED_FR 127


// Anchor Names
#define F_L 0
#define F_R 1
#define R_R 2
#define R_L 3

// Receiving anchor
uint8_t anchorReceiving = F_L;
// message flow state
volatile byte expectedMsgId = POLL_FL;
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
uint32_t resetPeriod = 1000;
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
    // Begin Serial communication
    //Serial1.begin(9600);
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


    // start receive mode, wait for POLL message
    receiverFL();
    // reset watchdog
    noteActivity();
    // for first time ranging frequency computation
    rangingCountPeriod = millis();

    //Initialize filter
    filtCounter = 0;
    sum = 0;
    for(int i = 0; i < FILTER_LENGTH; i++)
    {
      filt_list[i] = 30;
    }
}

void noteActivity() {
    // reset watchdog
    lastActivity = millis();
}

void resetInactiveFL() {
    // when watchdog times out, reset device
    //expectedMsgId = POLL_FL;
    receiverFL();
    noteActivity();
    //Serial.println("Timeout");
}

void resetInactiveFR() {
    // when watchdog times out, reset device
    //expectedMsgId = POLL_FR;
    receiverFR();
    noteActivity();
    //Serial.println("Timeout");
}

void handleSent() {
    // change state when ACK sent
    sentAck = true;
}

void handleReceived() {
    // change state when ACK received
    receivedAck = true;
}

void transmitPollAckFL() {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    data[0] = POLL_ACK_FL;
    // delay the same amount as ranging tag
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    DW1000FL.setDelay(deltaTime);
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();
}

void transmitPollAckFR() {
    DW1000FR.newTransmit();
    DW1000FR.setDefaults();
    data[0] = POLL_ACK_FR;
    // delay the same amount as ranging tag
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    DW1000FR.setDelay(deltaTime);
    DW1000FR.setData(data, LEN_DATA);
    DW1000FR.startTransmit();
}

void transmitRangeAckFL() {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    data[0] = RANGE_ACK_FL;
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();
}

void transmitRangeAckFR() {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    data[0] = RANGE_ACK_FR;
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();
}

void transmitRangeFailedFL() {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    data[0] = RANGE_ACK_FL;
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();
}

void transmitRangeFailedFR() {
    DW1000FR.newTransmit();
    DW1000FR.setDefaults();
    data[0] = RANGE_ACK_FR;
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
    DW1000Time reply1 = (timePollAckSent - timePollReceived).wrap();
    DW1000Time round2 = (timeRangeReceived - timePollAckSent).wrap();
    DW1000Time reply2 = (timeRangeSent - timePollAckReceived).wrap();
    DW1000Time tof = (round1 * round2 - reply1 * reply2) / (round1 + round2 + reply1 + reply2);
    // set tof timestamp
    timeComputedRange.setTimestamp(tof);
}

//Averaging Filter

float filter(float newDist)
{
  uint16_t delta = 100; //Range in which new reading must be to contribute to average is current average +/- delta
  float first_val;

  if(newDist > 0)
  {
    first_val = filt_list[0];
    filtCounter = filtCounter + 1;
    for(int k = 0; k < FILTER_LENGTH-1; k++)
    {
      filt_list[k] = filt_list[k + 1];
    }
  }
  
  if(filtCounter < FILTER_LENGTH) //If the filter length has not yet been reached don't return anything
  {
    if(newDist > 0)
    {
     sum = sum + newDist;
     avg = sum/filtCounter; 
     filt_list[FILTER_LENGTH-1] = newDist;     
    }
    return 0;
  }
  else
  {
    if(newDist > 0 && newDist >= avg - delta && newDist <= avg + delta)
    {
      filt_list[FILTER_LENGTH-1] = newDist;    
      sum = sum - first_val + newDist;
      avg = sum/(FILTER_LENGTH);  
    }
    else
    {
      filt_list[FILTER_LENGTH-1] = filt_list[FILTER_LENGTH-2];
      avg = avg;
    }

    
    
    return avg;
  }
}


void loop() {
    int32_t curMillis = millis(); // get current time
    if (!sentAck && !receivedAck) {
        // reset if wathcdog timed out
        if (curMillis - lastActivity > resetPeriod) {
            Serial.print("WATCHDOG TIMEOUT \n\r");
            if(anchorReceiving == F_L){
                resetInactiveFL();     
            }else if(anchorReceiving == F_R){
                resetInactiveFR();     
            }

        }
        return;
    }

    if (anchorReceiving == F_L){    
        // SentAck (after receiving first POLL)
        if (sentAck) {
            sentAck = false;
            byte msgId = data[0];
            if (msgId == POLL_ACK_FL) {
                DW1000FL.getTransmitTimestamp(timePollAckSent);
                // reset watchdog
                noteActivity();
            }
        }
        if (receivedAck) {  
            receivedAck = false;
            // get message
            DW1000FL.getData(data, LEN_DATA);
            byte msgId = data[0];
            Serial.print("FRONT-LEFT: "); Serial.println(msgId); 
            if (msgId != expectedMsgId) {
                // unexpected message, start over again (except if already POLL)
                protocolFailed = true;
               Serial.print("Received ERROR --FL-- Expected:"); Serial.println(expectedMsgId);        
            }
            if (msgId == POLL_FL) {
                // get timestamp, change expected message and send POLL_ACK
                //Serial.print("Received POLL --FL-- \n\r");
                protocolFailed = false;
                DW1000FL.getReceiveTimestamp(timePollReceived);
                expectedMsgId = RANGE_FL;
                transmitPollAckFL();
                // reset watchdog
                noteActivity();
            }else if (msgId == RANGE_FL) {
                // get timestamp, change expected message, calculate range and print
                //Serial.print("Received RANGE --FL-- \n\r");
                DW1000FL.getReceiveTimestamp(timeRangeReceived);
                expectedMsgId = POLL_FR;
                if (!protocolFailed) {
                    timePollSent.setTimestamp(data + 1);
                    timePollAckReceived.setTimestamp(data + 6);
                    timeRangeSent.setTimestamp(data + 11);
                    computeRangeAsymmetric();
                    transmitRangeAckFL();
                    float distance = timeComputedRange.getAsMeters()*100;
                    float avg_distance = filter(distance);
                   /* String Serialdata = "New Distance = " + String(distance);
                    Serial.println(Serialdata);                
                    Serialdata = "Average Distance = " + String(avg_distance);
                    Serial.println(Serialdata);  */
                    String Serialdata = "0," + String(distance) + ",0," + String(avg_distance) + "," + String(samplingRate) + "," + String(DW1000FL.getReceivePower()) + "," + String(DW1000FL.getReceiveQuality()) + "\n\r";                
                    Serial.print(Serialdata);
                    //Change Receiver anchor
                    DW1000FL.receivePermanently(false);
                    anchorReceiving = F_R;
                    receiverFR();
                    // update sampling rate (each second)
                    successRangingCount++;
                    if (curMillis - rangingCountPeriod > 1000) {
                        samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
                        rangingCountPeriod = curMillis;
                        successRangingCount = 0;
                    }
                }
                else {
                    Serial.print("RANGE Failed --FL-- \n\r");
                    transmitRangeFailedFL();
                }
                // reset watchdog
                noteActivity();
            }
        }
    }else if (anchorReceiving == F_R) {
        // SentAck (after receiving first POLL)
        if (sentAck) {
            sentAck = false;
            byte msgId = data[0];
            if (msgId == POLL_ACK_FR) {
                DW1000FR.getTransmitTimestamp(timePollAckSent);
                // reset watchdog
                noteActivity();
            }
        }
        if (receivedAck) {  
            receivedAck = false;
            // get message
            DW1000FR.getData(data, LEN_DATA);
            byte msgId = data[0];
            Serial.print("FRONT-RIGHT: "); Serial.println(msgId); 
            if (msgId != expectedMsgId) {
                // unexpected message, start over again (except if already POLL)
                protocolFailed = true;
               Serial.print("Received ERROR --FR-- Expected:"); Serial.println(expectedMsgId);             
            }
            if (msgId == POLL_FR) {
                // get timestamp, change expected message and send POLL_ACK
                //Serial.print("Received POLL --FR-- \n\r");
                protocolFailed = false;
                DW1000FR.getReceiveTimestamp(timePollReceived);
                expectedMsgId = RANGE_FR;
                transmitPollAckFR();
                // reset watchdog
                noteActivity();
            }
            else if (msgId == RANGE_FR) {
                // get timestamp, change expected message, calculate range and print
                //Serial.print("Received RANGE --FR-- \n\r");
                DW1000FR.getReceiveTimestamp(timeRangeReceived);
                expectedMsgId = POLL_FL;
                if (!protocolFailed) {
                    timePollSent.setTimestamp(data + 1);
                    timePollAckReceived.setTimestamp(data + 6);
                    timeRangeSent.setTimestamp(data + 11);
                    computeRangeAsymmetric();
                    transmitRangeAckFR();
                    float distance = timeComputedRange.getAsMeters()*100;
                    float avg_distance = filter(distance);
                   /* String Serialdata = "New Distance = " + String(distance);
                    Serial.println(Serialdata);                
                    Serialdata = "Average Distance = " + String(avg_distance);
                    Serial.println(Serialdata);  */
                    String Serialdata = "0," + String(distance) + ",0," + String(avg_distance) + "," + String(samplingRate) + "," + String(DW1000FL.getReceivePower()) + "," + String(DW1000FL.getReceiveQuality()) + "\n\r";                
                    Serial.print(Serialdata);
                    //Change Receiver anchor
                    DW1000FR.receivePermanently(false); //CHANGE THIS IN THE LIBRARIES, it is not setting the bit back
                    anchorReceiving = F_L;
                    receiverFL();  
                    // update sampling rate (each second)
                    successRangingCount++;
                    if (curMillis - rangingCountPeriod > 1000) {
                        samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
                        rangingCountPeriod = curMillis;
                        successRangingCount = 0;
                    }
                }
                else {
                    Serial.print("RANGE Failed --FR-- \n\r");
                    transmitRangeFailedFR();
                }
                // reset watchdog
                noteActivity();
            }
        }                                            
    }
}
