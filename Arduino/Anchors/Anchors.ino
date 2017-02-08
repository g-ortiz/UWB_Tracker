/*
 * UWB - Tracker
 * Anchors
 * Gabriel Ortiz 
 * Fredrik Treven
 * Adapted from: Decawave DW1000 library for arduino RangingAchor example.
 */

#include <SPI.h>
#include <DW1000.h>
#include <Tracker.h>

// Pins in Arduino M0 Pro
const uint8_t PIN_RST = 13; // reset pin
const uint8_t PIN_IRQ = 4; // irq pin
const uint8_t PIN_SS = 7; // spi select pin

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
uint32_t resetPeriod = 250;
// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;
// ranging counter (per second)
uint16_t successRangingCount = 0;
uint32_t rangingCountPeriod = 0;
float samplingRate = 0;

void setup() {
    // Setup Code
    // Begin SerialUSB communication
    //SerialUSB1.begin(9600);
    SerialUSB.begin(115200);
    delay(1000);
    // Set pins and start SPI
    DW1000.begin(PIN_IRQ, PIN_RST);
    DW1000.select(PIN_SS);
    // general configuration
    DW1000.newConfiguration();
    DW1000.setDefaults();
    DW1000.setDeviceAddress(1);
    DW1000.setNetworkId(10);
    DW1000.enableMode(DW1000.MODE_LONGDATA_RANGE_LOWPOWER); // Test to see if we get better results with a different mode
    DW1000.commitConfiguration();
    DW1000.enableDebounceClock();
    DW1000.enableLedBlinking();


    // set function callbacks for sent and received messages
    DW1000.attachSentHandler(handleSent);
    DW1000.attachReceivedHandler(handleReceived);
    
    // start receive mode, wait for POLL message
    receiver();
    // reset watchdog
    noteActivity();
    // for first time ranging frequency computation
    rangingCountPeriod = millis();

    //Initialize filter
    Tracker.initFilter();
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
    DW1000.newTransmit();
    DW1000.setDefaults();
    data[0] = POLL_ACK;
    // delay the same amount as ranging tag
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    DW1000.setDelay(deltaTime);
    DW1000.setData(data, LEN_DATA);
    DW1000.startTransmit();
}

void transmitRangeReport(float curRange) {
    DW1000.newTransmit();
    DW1000.setDefaults();
    data[0] = RANGE_REPORT;
    // write final ranging result
    memcpy(data + 1, &curRange, 4);
    DW1000.setData(data, LEN_DATA);
    DW1000.startTransmit();
}

void transmitRangeFailed() {
    DW1000.newTransmit();
    DW1000.setDefaults();
    data[0] = RANGE_FAILED;
    DW1000.setData(data, LEN_DATA);
    DW1000.startTransmit();
}

void receiver() {
    DW1000.newReceive();
    DW1000.setDefaults();
    // Enable receiver
    DW1000.receivePermanently(true);
    DW1000.startReceive();
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
    int32_t curMillis = millis(); // get current time
    if (!sentAck && !receivedAck) {
        // reset if wathcdog timed out
        if (curMillis - lastActivity > resetPeriod) {
            //SerialUSB.print("WATCHDOG TIMEOUT \n\r");
            resetInactive();
        }
        return;
    }
    // SentAck (after receiving first POLL)
    if (sentAck) {
        sentAck = false;
        byte msgId = data[0];
        if (msgId == POLL_ACK) {
            DW1000.getTransmitTimestamp(timePollAckSent);
            // reset watchdog
            noteActivity();
        }
    }
    if (receivedAck) {  
        receivedAck = false;
        // get message
        DW1000.getData(data, LEN_DATA);
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
            DW1000.getReceiveTimestamp(timePollReceived);
            expectedMsgId = RANGE;
            transmitPollAck();
            // reset watchdog
            noteActivity();
        }
        else if (msgId == RANGE) {
            // get timestamp, change expected message, calculate range and print
            //SerialUSB.print("Received RANGE \n\r");
            DW1000.getReceiveTimestamp(timeRangeReceived);
            expectedMsgId = POLL;
            if (!protocolFailed) {
                timePollSent.setTimestamp(data + 1);
                timePollAckReceived.setTimestamp(data + 6);
                timeRangeSent.setTimestamp(data + 11);
                computeRangeAsymmetric();
                //transmitRangeReport(timeComputedRange.getAsMicroSeconds()); // Send range report to TAG, why?
                float distance = timeComputedRange.getAsMeters()*100;
                float avg_distance = Tracker.filter(distance);
               /* String SerialUSBdata = "New Distance = " + String(distance);
                SerialUSB.println(SerialUSBdata);                
                SerialUSBdata = "Average Distance = " + String(avg_distance);
                SerialUSB.println(SerialUSBdata);  */
                String SerialUSBdata = "0," + String(distance) + ",0," + String(avg_distance) + "," + String(samplingRate) + "," + String(DW1000.getReceivePower()) + "," + String(DW1000.getReceiveQuality()) + "\n\r";                
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
