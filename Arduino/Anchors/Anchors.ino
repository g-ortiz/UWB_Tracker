/*
   UWB - Tracker
   Anchors
   Gabriel Ortiz
   Fredrik Treven
   Adapted from: Decawave DW1000 library for arduino RangingAchor example.
*/

#include <SPI.h>
#include <DW1000FL.h>
#include <DW1000FR.h>
#include <DW1000RR.h>
#include <DW1000RL.h>
#include <Tracker.h>
#include <Wire.h>


// Pins in Arduino M0 Pro for anchors
const uint8_t PIN_RST_FL = 13; // reset pin
const uint8_t PIN_IRQ_FL = 11; // irq pin
const uint8_t PIN_SS_FL = 12; // spi select pin
const uint8_t PIN_RST_FR = 7; // reset pin
const uint8_t PIN_IRQ_FR = 5; // irq pin
const uint8_t PIN_SS_FR = 6; // spi select pin
const uint8_t PIN_RST_RR = A3; // reset pin
const uint8_t PIN_IRQ_RR = A5; // irq pin
const uint8_t PIN_SS_RR = A4; // spi select pin
const uint8_t PIN_RST_RL = A0; // reset pin
const uint8_t PIN_IRQ_RL = A2; // irq pin
const uint8_t PIN_SS_RL = A1; // spi select pin


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

// Positioning and Ranging variables
float coords[4];
float rawcoords[2];
float ranges[4];
float powers[4];

// Initial states
uint8_t anchorRanging = F_L;
volatile byte expectedMsgId = POLL_ACK;

// message sent/received state
volatile boolean sentAck = false;
volatile boolean receivedAck = false;

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
uint32_t resetPeriod = 200;

// ranging counter (per second)
uint16_t successRangingCount = 0;
uint32_t rangingCountPeriod = 0;
float samplingRate = 0;

uint16_t kalman_buf = 0; //Allow kalman to setup prior to starting movement

void setup() {

  // Begin serialcommunication
  SerialUSB.begin(115200);
  Serial1.begin(115200);
  Wire.begin(); // start I2C
  delay(1000);

  // Setup Anchors
  // ################# FRONT LEFT####################//
  DW1000FL.begin(PIN_IRQ_FL, PIN_RST_FL);
  DW1000FL.select(PIN_SS_FL); //
  DW1000FL.newConfiguration();
  DW1000FL.setDefaults();
  DW1000FL.setDeviceAddress(1);
  DW1000FL.setNetworkId(12);
  DW1000FL.enableMode(DW1000FL.MODE_SHORTDATA_FAST_LOWPOWER);
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
  DW1000FR.setDeviceAddress(2);
  DW1000FR.setNetworkId(12);
  DW1000FR.enableMode(DW1000FR.MODE_SHORTDATA_FAST_LOWPOWER);
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
  DW1000RR.setDeviceAddress(3);
  DW1000RR.setNetworkId(12);
  DW1000RR.enableMode(DW1000RR.MODE_SHORTDATA_FAST_LOWPOWER);
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
  DW1000RL.setDeviceAddress(4);
  DW1000RL.setNetworkId(12);
  DW1000RL.enableMode(DW1000RL.MODE_SHORTDATA_FAST_LOWPOWER);
  DW1000RL.commitConfiguration();
  DW1000RL.enableDebounceClock();
  DW1000RL.enableLedBlinking();
  // set function callbacks for sent and received messages
  DW1000RL.attachSentHandler(handleSent);
  DW1000RL.attachReceivedHandler(handleReceived);

  //Initialize filter and multilateration
  Tracker.initTracker();
  coords[0] = 0;
  coords[1] = 0;

  //attach Interrupts
  attachInterrupt(digitalPinToInterrupt(PIN_IRQ_FL), DW1000FL.handleInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_IRQ_FR), DW1000FR.handleInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_IRQ_RR), DW1000RR.handleInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_IRQ_RL), DW1000RL.handleInterrupt, RISING);

  // start receive mode, wait for POLL message
  receiverFL();
  transmitPollFL();
  // Set the other anchors to idle and not receive permanently
  DW1000FR.receivePermanently(false);
  DW1000RR.receivePermanently(false);
  DW1000RL.receivePermanently(false);

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
}

void resetInactiveFR() {
  // when watchdog times out, reset device
  expectedMsgId = POLL_ACK;
  transmitPollFR();
  noteActivity();
  delay(5);
  receiverFR();
}

void resetInactiveRR() {
  // when watchdog times out, reset device
  expectedMsgId = POLL_ACK;
  transmitPollRR();
  noteActivity();
  delay(5);
  receiverRR();

}

void resetInactiveRL() {
  // when watchdog times out, reset device
  expectedMsgId = POLL_ACK;
  transmitPollRL();
  noteActivity();
  delay(5);
  receiverRL();
}

void handleSent() {
  // change state when ACK sent
  sentAck = true;
}

void handleReceived() {
  // change state when ACK received
  receivedAck = true;
}


// The following functions are used for sending and receiving the messages in the ranging protocol

void transmitPollFL() {
  DW1000FL.newTransmit();
  DW1000FL.setDefaults();
  //    SerialUSB.println("Sent POLL_FL");
  data[0] = POLL;
  DW1000FL.setData(data, LEN_DATA);
  DW1000FL.startTransmit();
}

void transmitPollFR() {
  DW1000FR.newTransmit();
  DW1000FR.setDefaults();
  //    SerialUSB.println("Sent POLL_FR");
  data[0] = POLL;
  DW1000FR.setData(data, LEN_DATA);
  DW1000FR.startTransmit();
}

void transmitPollRR() {
  DW1000RR.newTransmit();
  DW1000RR.setDefaults();
  //    SerialUSB.println("Sent POLL_RR");
  data[0] = POLL;
  DW1000RR.setData(data, LEN_DATA);
  DW1000RR.startTransmit();
}

void transmitPollRL() {
  DW1000RL.newTransmit();
  DW1000RL.setDefaults();
  //    SerialUSB.println("Sent POLL_RL");
  data[0] = POLL;
  DW1000RL.setData(data, LEN_DATA);
  DW1000RL.startTransmit();
}

void transmitRangeFL() {
  DW1000FL.newTransmit();
  DW1000FL.setDefaults();
  //SerialUSB.println("Send Range_FL");
  data[0] = RANGE;
  DW1000FL.setData(data, LEN_DATA);
  DW1000FL.startTransmit();
}

void transmitRangeFR() {
  DW1000FR.newTransmit();
  DW1000FR.setDefaults();
  //SerialUSB.println("Send Range_FR");
  data[0] = RANGE;
  DW1000FR.setData(data, LEN_DATA);
  DW1000FR.startTransmit();
}

void transmitRangeRR() {
  DW1000RR.newTransmit();
  DW1000RR.setDefaults();
  //SerialUSB.println("Send Range_RR");
  data[0] = RANGE;
  DW1000RR.setData(data, LEN_DATA);
  DW1000RR.startTransmit();
}

void transmitRangeRL() {
  DW1000RL.newTransmit();
  DW1000RL.setDefaults();
  //SerialUSB.println("Send Range_RL");
  data[0] = RANGE;
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
  // reset if wathcdog timed out. It resets the ancor that is currently ranging and start over the protocol at that anchor.
  int32_t curMillis = millis(); // get current time
  if (!sentAck && !receivedAck) {
    if (curMillis - lastActivity > resetPeriod) {
      //SerialUSB.println("WATCHDOG TIMEOUT");
      if (anchorRanging == F_L) {
        resetInactiveFL();
      } else if (anchorRanging == F_R) {
        resetInactiveFR();
      } else if (anchorRanging == R_R) {
        resetInactiveRR();
      } else if (anchorRanging == R_L) {
        resetInactiveRL();
      }
    }
    return;
  }

// The anchorRanging contains the value of the anchor that is currently ranging.
// This value is changed after every completed ranging and the next anchor is assigned to it.
// The same protocol is repeated for each anchors, the only thing that change is the libraries that are used.

  if (anchorRanging == F_L) {
    if (sentAck) { // If sentAck flag has been risen in the interrupt handler
      sentAck = false;
      byte msgId = data[0];
      if (msgId == POLL) {
        // If a POLL message was sent, save timestampt
        DW1000FL.getTransmitTimestamp(timePollSent);
        noteActivity();
      } else if (msgId == RANGE) {
        // If a RANGE message was sent, save timestampt        
        DW1000FL.getTransmitTimestamp(timeRangeSent);
        noteActivity();
      }
    }
    if (receivedAck) { // If receivdAck flag has been risen in the interrupt handler (a message was received)
      receivedAck = false;
      // get message
      DW1000FL.getData(data, LEN_DATA);
      byte msgId = data[0];
      if (msgId != expectedMsgId) {
        expectedMsgId = POLL_ACK;
        return;
      }
      // Depending on the type of message the processor executes different functions:
      if (msgId == POLL_ACK) {
        DW1000FL.getReceiveTimestamp(timePollAckReceived); // get Poll Acknowledge Received Timestamp
        timePollReceived.setTimestamp(data + 1); // get Poll Received Timestamp
        expectedMsgId = RANGE_ACK; // change expected message
        transmitRangeFL(); // Send range message
        DW1000FL.receivePermanently(false);
        DW1000FR.receivePermanently(true);
        noteActivity();
      } else if (msgId == RANGE_ACK) {
        timeRangeReceived.setTimestamp(data + 1); // get Range Received Timestamp
        timePollAckSent.setTimestamp(data + 6); // get Poll Acknowledge Sent Timestamp
        computeRangeAsymmetric(); // compute TOF
        float distance = timeComputedRange.getAsMeters() * 100; // get distance as meters
        distance = (distance - 24.8) / 1.146; // Offset correction equation (see report)
        Tracker.filter(distance , F_L, coords); // Execute positioning 
        powers[0] = DW1000FL.getReceivePower(); // get received power
        ranges[0] = Tracker.smoothing(distance , F_L, coords); // execute filter (not active)
        rawcoords[0] = coords[2]; 
        rawcoords[1] = coords[3];
        Tracker.kalman(coords + 2); // execute kalman filter
        kalman_buf = kalman_buf + 1;
        anchorRanging = F_R; //change ranging anchor
        expectedMsgId = POLL_ACK; // change expected message
        
        // Send data to arduino mini using I2C        
        if (kalman_buf > 3)
        {
          byte *bvalX;
          byte *bvalY;
          bvalX = (byte *)&coords[2];
          bvalY = (byte *)&coords[3];
          Wire.beginTransmission(4); // transmit to device #4
          Wire.write((int)bvalX[0]);              // sends one byte
          Wire.write((int) bvalX[1]);              // sends one byte
          Wire.write((int)bvalX[2]);              // sends one byte
          Wire.write((int) bvalX[3]);              // sends one byte
          Wire.write((int)bvalY[0]);              // sends one byte
          Wire.write((int) bvalY[1]);              // sends one byte
          Wire.write((int)bvalY[2]);              // sends one byte
          Wire.write((int) bvalY[3]);              // sends one byte
          Wire.endTransmission();    // stop transmitting
          successRangingCount++;
        }
        // Calculate Sampling Frequency
        if (curMillis - rangingCountPeriod > 1000) {
          samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
          rangingCountPeriod = curMillis;
          successRangingCount = 0;
        }
        transmitPollFR();
        noteActivity();
      }
    }
  } else if (anchorRanging == F_R) {
    if (sentAck) {
      sentAck = false;
      byte msgId = data[0];
      if (msgId == POLL) {
        DW1000FR.getTransmitTimestamp(timePollSent);
        noteActivity();
      } else if (msgId == RANGE) {
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
        expectedMsgId = RANGE_ACK;
        transmitRangeFR();
        DW1000FR.receivePermanently(false);
        DW1000RR.receivePermanently(true);
        noteActivity();
      } else if (msgId == RANGE_ACK) {
        timeRangeReceived.setTimestamp(data + 1);
        timePollAckSent.setTimestamp(data + 6);
        computeRangeAsymmetric();
        float distance = timeComputedRange.getAsMeters() * 100;
        distance = (distance - 24.8) / 1.146;
        powers[1] = DW1000FR.getReceivePower();
        Tracker.filter(distance , F_R, coords);
        ranges[1] = Tracker.smoothing(distance , F_R, coords);
        rawcoords[0] = coords[2];
        rawcoords[1] = coords[3];
        Tracker.kalman(coords + 2);
        kalman_buf = kalman_buf + 1;
        if (kalman_buf > 3)
        {
          byte *bvalX;
          byte *bvalY;
          bvalX = (byte *)&coords[2];
          bvalY = (byte *)&coords[3];
          Wire.beginTransmission(4); // transmit to device #4
          Wire.write((int)bvalX[0]);              // sends one byte
          Wire.write((int) bvalX[1]);              // sends one byte
          Wire.write((int)bvalX[2]);              // sends one byte
          Wire.write((int) bvalX[3]);              // sends one byte
          Wire.write((int)bvalY[0]);              // sends one byte
          Wire.write((int) bvalY[1]);              // sends one byte
          Wire.write((int)bvalY[2]);              // sends one byte
          Wire.write((int) bvalY[3]);              // sends one byte
          Wire.endTransmission();    // stop transmitting
          successRangingCount++;
        }
        if (curMillis - rangingCountPeriod > 1000) {
          samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
          rangingCountPeriod = curMillis;
          successRangingCount = 0;
        }
        anchorRanging = R_R;
        expectedMsgId = POLL_ACK;
        transmitPollRR();
        noteActivity();
      }
    }
  } else if (anchorRanging == R_R) {
    if (sentAck) {
      sentAck = false;
      byte msgId = data[0];
      if (msgId == POLL) {
        DW1000RR.getTransmitTimestamp(timePollSent);
        noteActivity();
      } else if (msgId == RANGE) {
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
        expectedMsgId = RANGE_ACK;
        transmitRangeRR();
        DW1000RR.receivePermanently(false);
        DW1000RL.receivePermanently(true);
        noteActivity();
      } else if (msgId == RANGE_ACK) {
        timeRangeReceived.setTimestamp(data + 1);
        timePollAckSent.setTimestamp(data + 6);
        computeRangeAsymmetric();
        float distance = timeComputedRange.getAsMeters() * 100;
        distance = (distance - 24.8) / 1.146;
        powers[2] = DW1000RR.getReceivePower();
        Tracker.filter(distance , R_R, coords);
        ranges[2] = Tracker.smoothing(distance , R_R, coords);
        rawcoords[0] = coords[2];
        rawcoords[1] = coords[3];
        Tracker.kalman(coords + 2);
        kalman_buf = kalman_buf + 1;
        anchorRanging = R_L;
        expectedMsgId = POLL_ACK;
        if (kalman_buf > 3)
        {
          byte *bvalX;
          byte *bvalY;
          bvalX = (byte *)&coords[2];
          bvalY = (byte *)&coords[3];
          Wire.beginTransmission(4); // transmit to device #4
          Wire.write((int)bvalX[0]);              // sends one byte
          Wire.write((int) bvalX[1]);              // sends one byte
          Wire.write((int)bvalX[2]);              // sends one byte
          Wire.write((int) bvalX[3]);              // sends one byte
          Wire.write((int)bvalY[0]);              // sends one byte
          Wire.write((int) bvalY[1]);              // sends one byte
          Wire.write((int)bvalY[2]);              // sends one byte
          Wire.write((int) bvalY[3]);              // sends one byte
          Wire.endTransmission();    // stop transmitting
          successRangingCount++;
        }
        if (curMillis - rangingCountPeriod > 1000) {
          samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
          rangingCountPeriod = curMillis;
          successRangingCount = 0;
        }
        transmitPollRL();
        noteActivity();
      }
    }
  } else if (anchorRanging == R_L) {
    if (sentAck) {
      sentAck = false;
      byte msgId = data[0];
      if (msgId == POLL) {
        DW1000RL.getTransmitTimestamp(timePollSent);
        noteActivity();
      } else if (msgId == RANGE) {
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
        expectedMsgId = RANGE_ACK;
        transmitRangeRL();
        DW1000RL.receivePermanently(false);
        DW1000FL.receivePermanently(true);
        noteActivity();
      } else if (msgId == RANGE_ACK) {
        timeRangeReceived.setTimestamp(data + 1);
        timePollAckSent.setTimestamp(data + 6);
        computeRangeAsymmetric();
        float distance = timeComputedRange.getAsMeters() * 100;
        distance = (distance - 24.8) / 1.146;
        powers[3] = DW1000RL.getReceivePower();
        Tracker.filter(distance , R_L, coords);
        ranges[3] = Tracker.smoothing(distance , R_L, coords);
        rawcoords[0] = coords[2];
        rawcoords[1] = coords[3];
        Tracker.kalman(coords + 2);
        kalman_buf = kalman_buf + 1;

        //Send data to GUI via SerialPort
        String SerialUSBdata = "m" + String(samplingRate) + "," + String(ranges[0]) + "," + String(ranges[1]) + "," + String(ranges[2]) + "," + String(ranges[3]) + "," + String(powers[0]) + "," + String(powers[1]) + "," + String(powers[2]) + "," + String(powers[3]) 
                               + "," + String(coords[2]) + "," + String(coords[3]) + "," + String(rawcoords[0]) + "," + String(rawcoords[1]) + "b\n\r";
        SerialUSB.print(SerialUSBdata);
        Serial1.print(SerialUSBdata);
        //Send movement to mini if kaman buffer amount reached
        if (kalman_buf > 3)
        {
          byte *bvalX;
          byte *bvalY;
          bvalX = (byte *)&coords[2];
          bvalY = (byte *)&coords[3];
          Wire.beginTransmission(4); // transmit to device #4
          Wire.write((int)bvalX[0]);              // sends one byte
          Wire.write((int) bvalX[1]);              // sends one byte
          Wire.write((int)bvalX[2]);              // sends one byte
          Wire.write((int) bvalX[3]);              // sends one byte
          Wire.write((int)bvalY[0]);              // sends one byte
          Wire.write((int) bvalY[1]);              // sends one byte
          Wire.write((int)bvalY[2]);              // sends one byte
          Wire.write((int) bvalY[3]);              // sends one byte
          Wire.endTransmission();    // stop transmitting
          successRangingCount++;
        }
        if (curMillis - rangingCountPeriod > 1000) {
          samplingRate = (1000.0f * successRangingCount) / (curMillis - rangingCountPeriod);
          rangingCountPeriod = curMillis;
          successRangingCount = 0;
        }
        anchorRanging = F_L;
        expectedMsgId = POLL_ACK;
        transmitPollFL();
        noteActivity();
      }
    }
  }
}



