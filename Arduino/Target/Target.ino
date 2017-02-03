/*
 * UWB - Tracker
 * Target
 * Gabriel Ortiz 
 * Fredrik Treven
 * Adapted from: Decawave DW1000 library for arduino RangingAchor example.
 */

#include <SPI.h>
#include <DW1000.h>

// Pins in Arduino Pro Mini 3.3V
const uint8_t PIN_RST = 9; // reset pin
const uint8_t PIN_IRQ = 2; // irq pin
const uint8_t PIN_SS = 7; // spi select pin



// Expected messages
// Expected messages FL
#define POLL_FL 10
#define POLL_ACK_FL 11
#define RANGE_FL 12
#define RANGE_REPORT_FL 13
#define RANGE_FAILED 255

// Expected messages FR
#define POLL_FR 20
#define POLL_ACK_FR 21
#define RANGE_FR 22
#define RANGE_REPORT_FR 23



// message flow state
volatile byte expectedMsgId = POLL_ACK_FL;
// message sent/received state
volatile boolean sentAck = false;
volatile boolean receivedAck = false;
// timestamps
DW1000Time timePollSent;
DW1000Time timePollAckReceived;
DW1000Time timeRangeSent;
// data buffer
#define LEN_DATA 16
byte data[LEN_DATA];
// watchdog and reset period
uint32_t lastActivity;
uint32_t resetPeriod = 250;
// reply times (same on both sides for symm. ranging)
uint16_t replyDelayTimeUS = 3000;

void setup() {
    // Setup Code
    // Begin serial communication
    Serial.begin(115200);
    // Set pins and start SPI
    DW1000.begin(PIN_IRQ, PIN_RST);
    DW1000.select(PIN_SS);
    // general configuration
    DW1000.newConfiguration();
    DW1000.setDefaults();
    DW1000.setDeviceAddress(2);
    DW1000.setNetworkId(12);
    DW1000.enableMode(DW1000.MODE_LONGDATA_RANGE_LOWPOWER);
    DW1000.commitConfiguration();
    DW1000.enableDebounceClock();
    DW1000.enableLedBlinking();   
    DW1000.attachSentHandler(handleSent);
    DW1000.attachReceivedHandler(handleReceived);
    // Target transmitting a POLL message
    receiver();
    transmitPollFL();
    // reset watchdog
    noteActivity();
}

void noteActivity() {
    // reset watchdog
    lastActivity = millis();
}

void resetInactive() {
    // when watchdog times out, reset device
    expectedMsgId = POLL_ACK_FL;
    transmitPollFL();
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

void transmitPollFL() {
    DW1000.newTransmit();
    DW1000.setDefaults();
    Serial.println("Sent POLL_FR");
    data[0] = POLL_FL;
    DW1000.setData(data, LEN_DATA);
    DW1000.startTransmit();
}


void transmitPollFR() {
    DW1000.newTransmit();
    DW1000.setDefaults();
    Serial.println("Sent POLL_FR");
    data[0] = POLL_FR;
    DW1000.setData(data, LEN_DATA);
    DW1000.startTransmit();
}


void transmitRangeFL() {
    DW1000.newTransmit();
    DW1000.setDefaults();
    Serial.println("Send Range_FL");
    data[0] = RANGE_FL;
    // delay sending the message and remember expected future sent timestamp
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    timeRangeSent = DW1000.setDelay(deltaTime);
    timePollSent.getTimestamp(data + 1);
    timePollAckReceived.getTimestamp(data + 6);
    timeRangeSent.getTimestamp(data + 11);
    DW1000.setData(data, LEN_DATA);
    DW1000.startTransmit();
}

void transmitRangeFR() {
    DW1000.newTransmit();
    DW1000.setDefaults();
    Serial.println("Send Range_FR");
    data[0] = RANGE_FR;
    // delay sending the message and remember expected future sent timestamp
    DW1000Time deltaTime = DW1000Time(replyDelayTimeUS, DW1000Time::MICROSECONDS);
    timeRangeSent = DW1000.setDelay(deltaTime);
    timePollSent.getTimestamp(data + 1);
    timePollAckReceived.getTimestamp(data + 6);
    timeRangeSent.getTimestamp(data + 11);
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
            resetInactive();
        }
        return;
    }
    // SentAck (after transmitting POLL and RANGE)
    if (sentAck) {
        sentAck = false;
        byte msgId = data[0];
        if (msgId == POLL_FL || msgId == POLL_FR) {
            DW1000.getTransmitTimestamp(timePollSent);
        } else if (msgId == RANGE_FL || msgId == RANGE_FR) {
            DW1000.getTransmitTimestamp(timeRangeSent);
            noteActivity();
        }
    }
    if (receivedAck) {
        receivedAck = false;
        // get message and parse
        DW1000.getData(data, LEN_DATA);
        byte msgId = data[0];
        Serial.println(msgId);
        if (msgId != expectedMsgId) {
            // unexpected message, start over again
            Serial.print("Received ERROR Expected:"); Serial.println(expectedMsgId);
            expectedMsgId = POLL_ACK_FL;
            transmitPollFL();
            return;
        }
        if (msgId == POLL_ACK_FL) {
            Serial.println("Received POLL_ACK_FL");
            DW1000.getReceiveTimestamp(timePollAckReceived);
            expectedMsgId = POLL_ACK_FR;
            transmitRangeFL();
            noteActivity();
            delay(6);
            transmitPollFR();
            noteActivity();
        }else if (msgId == POLL_ACK_FR) {
            Serial.println("Received POLL_ACK_FR");
            DW1000.getReceiveTimestamp(timePollAckReceived);
            expectedMsgId = POLL_ACK_FL;
            transmitRangeFR();
            noteActivity();
            delay(6);
            transmitPollFL();
            noteActivity();            
        } else if (msgId == RANGE_FAILED) {
            expectedMsgId = POLL_ACK_FL;
            transmitPollFL();
            noteActivity();
        }
    }
}

