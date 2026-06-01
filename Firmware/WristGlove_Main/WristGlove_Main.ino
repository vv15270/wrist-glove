#include <Wire.h>
#include <Adafruit_LSM9DS1.h>
#include <SparkFun_BMI270_Arduino_Library.h>
#include <RF24.h>
#include "config.h"

// ── OBJECTS ────────────────────────────────
Adafruit_LSM9DS1 lsm;
BMI270 fingerIMU[4];
RF24 radio(NRF_CE_PIN, NRF_CSN_PIN);

// ── DATA PACKET ────────────────────────────
struct GloveData {
  float wristYaw;
  float wristPitch;
  float wristRoll;
  float fingerCurl[4];
  bool  pinch[3];
  bool  calibrateFlag;
};

GloveData gloveData;

// ── PINCH STATE TRACKING ───────────────────
unsigned long pinchStartTime[3] = {0, 0, 0};
bool pinchActive[3] = {false, false, false};

// ── TCA9548A CHANNEL SELECT ────────────────
void selectChannel(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// ── PINCH DETECTION WITH DEBOUNCE ─────────
void readPinches() {
  int pinchPins[3] = {PINCH_INDEX_PIN, PINCH_MIDDLE_PIN, PINCH_RING_PIN};
  
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
    
    gloveData.pinch[i] = pinchActive[i];
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("WristGlove initializing...");

  // configure pinch input pins
  pinMode(PINCH_INDEX_PIN, INPUT_PULLUP);
  pinMode(PINCH_MIDDLE_PIN, INPUT_PULLUP);
  pinMode(PINCH_RING_PIN, INPUT_PULLUP);

  // LSM9DS1 init placeholder
  // BMI270 init placeholder
  // NRF24L01 init placeholder

  Serial.println("Ready.");
}

void loop() {
  // Read LSM9DS1 placeholder
  // Read BMI270 x4 placeholder
  // Process sensor fusion placeholder
  // Detect gestures placeholder
  readPinches();
  // Transmit placeholder

  delay(1000 / LOOP_HZ);
}