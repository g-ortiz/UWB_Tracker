/*
 * UWB - Tracker
 * Target
 * Gabriel Ortiz 
 * Fredrik Treven
 * Adapted from: Decawave DW1000 library for arduino RangingAnchor example.
 */

#include <SPI.h>
#include <DW1000.h>

// Pins in Arduino Pro Mini 3.3V
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = 3; // spi select pin
const uint8_t LEDPIN = 10; // reset pin

// Anchor Names
#define F_L 0
#define F_R 1
#define R_R 2
#define R_L 3
//Anchors ranging
uint8_t anchorRanging = F_L;


// Expected messages FL
#define POLL 0
#define POLL_ACK 1
#define RANGE 2
#define RANGE_ACK 3
#define RANGE_FAILED 255

// Initial expected message
volatile byte expectedMsgId = POLL;
// message sent/received state
volatile boolean sentAck = false;
volatile boolean receivedAck = false;

// timestamps
DW1000Time timePollAckReceived;
DW1000Time timePollReceived;
DW1000Time timePollAckSent;
DW1000Time timeRangeReceived;

// data buffer
#define LEN_DATA 16
byte data[LEN_DATA];

// watchdog and reset period
uint32_t lastActivity;
uint32_t resetPeriod = 200;

// protocol error state
boolean protocolFailed = false;

void setup() {
    // Setup Code
    // Begin serial communication
    Serial.begin(115200);
    pinMode(LEDPIN, OUTPUT); // Leflt Forward
    // Set pins and start SPI
    DW1000.begin(PIN_IRQ, PIN_RST);
    DW1000.select(PIN_SS);
    // general configuration
    DW1000.newConfiguration();
    DW1000.setDefaults();
    DW1000.setDeviceAddress(5);
    DW1000.setNetworkId(12);
    DW1000.enableMode(DW1000.MODE_SHORTDATA_FAST_LOWPOWER);
    DW1000.commitConfiguration();
    DW1000.enableDebounceClock();
    DW1000.enableLedBlinking();   
    DW1000.attachSentHandler(handleSent);
    DW1000.attachReceivedHandler(handleReceived);
    
    // Target starts waiting for POLL
    receiver();
    // reset watchdog
    noteActivity();
}

void noteActivity() {
    // reset watchdog
    lastActivity = millis();
}

void resetInactive() {
    // when watchdog times out, restart communication
    expectedMsgId = POLL;
    receiver();
    noteActivity();
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
    //Serial.println("Send POLL_ACK");
    data[0] = POLL_ACK;
    timePollReceived.getTimestamp(data + 1);
    timePollAckSent.getTimestamp(data + 6);
    DW1000.setData(data, LEN_DATA);
    DW1000.startTransmit();   
}

void transmitRangeAck() {
    DW1000.newTransmit();
    DW1000.setDefaults();
    //Serial.println("Send RANGE_ACK");
    data[0] = RANGE_ACK;
    // delay the same amount as ranging tag  
    timeRangeReceived.getTimestamp(data + 1);
    timePollAckSent.getTimestamp(data + 6);
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

void loop() {
    if (!sentAck && !receivedAck) {
        // reset if wathcdog timed out
        if (millis() - lastActivity > resetPeriod) {
                //Serial.println("WATCHDOG TIMEOUT");
                resetInactive();
        }
        return;
    }
    if (sentAck) {
        sentAck = false;
        byte msgId = data[0];
        //Serial.print("Sent:   "); Serial.println(msgId);        
        if (msgId == POLL_ACK) {
            DW1000.getTransmitTimestamp(timePollAckSent);
            noteActivity(); // reset watchdog
        }
    }
    if (receivedAck) {  
        receivedAck = false;
        // get message
        DW1000.getData(data, LEN_DATA);
        byte msgId = data[0];
        //Serial.print("Received:   "); Serial.println(msgId); 
        if (msgId != expectedMsgId) {
            // unexpected message, start over again
            protocolFailed = true;
            expectedMsgId = POLL;
            //Serial.print("Received ERROR Expected:"); Serial.println(expectedMsgId);        
        }
        if (msgId == POLL) {
            DW1000.getReceiveTimestamp(timePollReceived);
            //Serial.print("Received POLL\n\r");
            protocolFailed = false;
            expectedMsgId = RANGE;
            transmitPollAck();
            noteActivity(); // reset watchdog
        }else if (msgId == RANGE) {
            DW1000.getReceiveTimestamp(timeRangeReceived);
           // Serial.print("Received RANGE\n\r");
            expectedMsgId = POLL;           
            transmitRangeAck();
            digitalWrite(LEDPIN, !digitalRead(LEDPIN));          
            noteActivity(); // reset watchdog
        }
    }                             
}

