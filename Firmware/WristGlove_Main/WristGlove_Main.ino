#include <Wire.h>
#include <Adafruit_LSM9DS1.h>
#include <SparkFun_BMI270_Arduino_Library.h>
#include "config.h"
#include "Madgwick.h"
#include "LeverArm.h"
#include "SensorData.h"
#include "SensorReader.h"
#include "GestureDetect.h"
#include "Calibration.h"

// ── OBJECTS ────────────────────────────────────────────────────────
Madgwick fingerFusion[5];
Madgwick handFusion;
LeverArmCalibration leverCal;
GloveState gloveState;

// ── PINCH TRACKING ─────────────────────────────────────────────────
unsigned long pinchStartTime[3] = {0, 0, 0};
bool pinchActive[3] = {false, false, false};

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
  Wire.begin(SDA_PIN, SCL_PIN);

  pinMode(PINCH_INDEX_PIN,  INPUT_PULLUP);
  pinMode(PINCH_MIDDLE_PIN, INPUT_PULLUP);
  pinMode(PINCH_RING_PIN,   INPUT_PULLUP);
  pinMode(MOTOR_PIN,        OUTPUT);

  handFusion.begin(LOOP_HZ);
  for (int i = 0; i < NUM_FINGERS; i++) {
    fingerFusion[i].begin(LOOP_HZ);
  }

  initSensors();

  if (!loadCalibration(leverCal)) {
    Serial.println("No calibration — run calibration before use");
  }

  Serial.println("WristGlove ready.");
}

void loop() {
  readAllSensors(gloveState);
  readPinches();

  if (gloveState.calibrateRequested) {
    runCalibration(leverCal, gloveState);
    gloveState.calibrateRequested = false;
  }


  Serial.print("Yaw:"); Serial.print(gloveState.hand.yaw);
  Serial.print(" I:");  Serial.print(gloveState.fingerCurlAngle[0]);
  Serial.print(" M:");  Serial.print(gloveState.fingerCurlAngle[1]);
  Serial.print(" R:");  Serial.print(gloveState.fingerCurlAngle[2]);
  Serial.print(" T:");  Serial.print(gloveState.fingerCurlAngle[3]);
  Serial.print(" P:");  Serial.println(gloveState.fingerCurlAngle[4]);

  delay(1000 / LOOP_HZ);
}