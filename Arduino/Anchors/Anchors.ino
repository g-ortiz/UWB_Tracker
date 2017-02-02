/*
 * UWB - Tracker
 * Anchors
 * Gabriel Ortiz 
 * Fredrik Treven
 * Adapted from: Decawave DW1000 library for arduino RangingAchor example.
 */

#include <SPI.h>
#include <DW1000FL.h>
#include <DW1000FL.h>

// Pins in Arduino M0 Pro
const uint8_t PIN_RST = 12; // reset pin
const uint8_t PIN_IRQ = 3; // irq pin
const uint8_t PIN_SS_FL = 6; // spi select pin
const uint8_t PIN_SS_FR = 7; // spi select pin

// Expected messages
#define POLL 0
#define POLL_ACK 1
#define RANGE 2
#define RANGE_REPORT 3
#define RANGE_FAILED 255
// message flow state
volatile byte expectedMsgId = POLL;
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
uint32_t resetPeriod = 500;
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
    // Begin SerialUSB communication
    //SerialUSB1.begin(9600);
    SerialUSB.begin(115200);
    delay(1000);
    // Set pins and start SPI
    DW1000FL.begin(PIN_IRQ, PIN_RST);    
    //DW1000FL.select(PIN_SS_FL,PIN_SS_FR); //Lots of things to modify, pass ss to writebytes
    DW1000FL.select(PIN_SS_FL); //    
    // general configuration
    DW1000FL.newConfiguration();
    DW1000FL.setDefaults();
    DW1000FL.setDeviceAddress(1);
    DW1000FL.setNetworkId(12);
    DW1000FL.enableMode(DW1000FL.MODE_LONGDATA_RANGE_LOWPOWER); // Test to see if we get better results with a different mode
    DW1000FL.commitConfiguration();
    DW1000FL.enableDebounceClock();
    DW1000FL.enableLedBlinking();


    // set function callbacks for sent and received messages
    DW1000FL.attachSentHandler(handleSent);
    DW1000FL.attachReceivedHandler(handleReceived);
    
    // start receive mode, wait for POLL message
    receiver();
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

void resetInactive() {
    // when watchdog times out, reset device
    expectedMsgId = POLL;
    receiver();
    noteActivity();
    //SerialUSB.println("Timeout");
}

void handleSent() {
    // change state when ACK sent
    sentAck = true;
}

void handleReceived() {
    // change state when ACK received
    receivedAck = true;
}

void transmitPollAck() {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    data[0] = POLL_ACK;
    // delay the same amount as ranging tag
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    DW1000FL.setDelay(deltaTime);
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();
}

void transmitRangeReport(float curRange) {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    data[0] = RANGE_REPORT;
    // write final ranging result
    memcpy(data + 1, &curRange, 4);
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();
}

void transmitRangeFailed() {
    DW1000FL.newTransmit();
    DW1000FL.setDefaults();
    data[0] = RANGE_FAILED;
    DW1000FL.setData(data, LEN_DATA);
    DW1000FL.startTransmit();
}

void receiver() {
    DW1000FL.newReceive();
    DW1000FL.setDefaults();
    // Enable receiver
    DW1000FL.receivePermanently(true);
    DW1000FL.startReceive();
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
            SerialUSB.print("WATCHDOG TIMEOUT \n\r");
            resetInactive();
        }
        return;
    }
    // SentAck (after receiving first POLL)
    if (sentAck) {
        sentAck = false;
        byte msgId = data[0];
        if (msgId == POLL_ACK) {
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
        //SerialUSB.println(msgId); 
        if (msgId != expectedMsgId) {
            // unexpected message, start over again (except if already POLL)
            protocolFailed = true;
            //SerialUSB.print("Received ERROR \n\r");            
        }
        if (msgId == POLL) {
            // get timestamp, change expected message and send POLL_ACK
            //SerialUSB.print("Received POLL \n\r");
            protocolFailed = false;
            DW1000FL.getReceiveTimestamp(timePollReceived);
            expectedMsgId = RANGE;
            transmitPollAck();
            // reset watchdog
            noteActivity();
        }
        else if (msgId == RANGE) {
            // get timestamp, change expected message, calculate range and print
            //SerialUSB.print("Received RANGE \n\r");
            DW1000FL.getReceiveTimestamp(timeRangeReceived);
            expectedMsgId = POLL;
            if (!protocolFailed) {
                timePollSent.setTimestamp(data + 1);
                timePollAckReceived.setTimestamp(data + 6);
                timeRangeSent.setTimestamp(data + 11);
                computeRangeAsymmetric();
                //transmitRangeReport(timeComputedRange.getAsMicroSeconds()); // Send range report to TAG, why?
                float distance = timeComputedRange.getAsMeters()*100;
                float avg_distance = filter(distance);
               /* String SerialUSBdata = "New Distance = " + String(distance);
                SerialUSB.println(SerialUSBdata);                
                SerialUSBdata = "Average Distance = " + String(avg_distance);
                SerialUSB.println(SerialUSBdata);  */
                String SerialUSBdata = "0," + String(distance) + ",0," + String(avg_distance) + "," + String(samplingRate) + "," + String(DW1000FL.getReceivePower()) + "," + String(DW1000FL.getReceiveQuality()) + "\n\r";                
                SerialUSB.print(SerialUSBdata);
                // update sampling rate (each second)
                successRangingCount++;
                if (curMillis - rangingCountPeriod > 1000) {
                    samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
                    rangingCountPeriod = curMillis;
                    successRangingCount = 0;
                }
            }
            else {
                //SerialUSB.print("RANGE Failed \n\r");
                transmitRangeFailed();
            }
            // reset watchdog
            noteActivity();
        }
    }
}
