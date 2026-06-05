#include <Wire.h>
#include <Adafruit_LSM9DS1.h>
#include <SparkFun_BMI270_Arduino_Library.h>
#include <RF24.h>
#include "config.h"
#include "Madgwick.h"
#include "LeverArm.h"
#include "SensorData.h"
#include "SensorReader.h"

// ── MADGWICK INSTANCES ─────────────────────────────────────────────
Madgwick fingerFusion[5];
Madgwick handFusion;

// ── LEVER ARM CALIBRATION ──────────────────────────────────────────
LeverArmCalibration leverCal;

// ── NRF RADIO ─────────────────────────────────────────────────────
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);

// ── COMPLETE GLOVE STATE ───────────────────────────────────────────
GloveState gloveState;

// ── PINCH STATE TRACKING ───────────────────────────────────────────
unsigned long pinchStartTime[3] = {0, 0, 0};
bool pinchActive[3] = {false, false, false};

// ── PINCH DETECTION ────────────────────────────────────────────────
void readPinches() {
  int pinchPins[3] = {
    PINCH_INDEX_PIN,
    PINCH_MIDDLE_PIN,
    PINCH_RING_PIN
  };

  for (int i = 0; i < 3; i++) {
    bool contact = !digitalRead(pinchPins[i]);

    if (contact && pinchStartTime[i] == 0) {
      pinchStartTime[i] = millis();
    }
    if (!contact) {
      pinchStartTime[i] = 0;
      pinchActive[i] = false;
    }
    if (contact && pinchStartTime[i] > 0) {
      if (millis() - pinchStartTime[i] >= PINCH_HOLD_MS) {
        pinchActive[i] = true;
      }
    }

    gloveState.pinch[i] = pinchActive[i];
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("WristGlove initializing...");

  // Initialize pinch pins
  pinMode(PINCH_INDEX_PIN,  INPUT_PULLUP);
  pinMode(PINCH_MIDDLE_PIN, INPUT_PULLUP);
  pinMode(PINCH_RING_PIN,   INPUT_PULLUP);

  // Initialize Madgwick filters
  handFusion.begin(LOOP_HZ);
  for (int i = 0; i < NUM_FINGERS; i++) {
    fingerFusion[i].begin(LOOP_HZ);
  }

  // LSM9DS1 init placeholder
  // BMI270 x5 init placeholder
  // NRF24L01 init placeholder

  Serial.println("Ready.");
}

void loop() {
  // ── READ ALL SENSORS ─────────────────────────────────────
  readAllSensors(gloveState);

  // ── READ PINCH INPUTS ────────────────────────────────────
  readPinches();

  // ── DEBUG OUTPUT ─────────────────────────────────────────
  Serial.print("Hand yaw: ");
  Serial.print(gloveState.hand.yaw);
  Serial.print(" | Index curl: ");
  Serial.print(gloveState.fingerCurlAngle[0]);
  Serial.print(" | Middle curl: ");
  Serial.print(gloveState.fingerCurlAngle[1]);
  Serial.print(" | Ring curl: ");
  Serial.print(gloveState.fingerCurlAngle[2]);
  Serial.print(" | Thumb curl: ");
  Serial.print(gloveState.fingerCurlAngle[3]);
  Serial.print(" | Pinky curl: ");
  Serial.println(gloveState.fingerCurlAngle[4]);

  // ── TRANSMIT PLACEHOLDER ─────────────────────────────────
  // Radio transmission goes here when hardware arrives

  delay(1000 / LOOP_HZ);
}