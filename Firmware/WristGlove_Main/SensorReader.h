#pragma once

#include "config.h"
#include "SensorData.h"
#include "LeverArm.h"
#include "Madgwick.h"
#include <Wire.h>
#include <math.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>
#include <SparkFun_BMI270_Arduino_Library.h>

// ── SENSOR OBJECTS ─────────────────────────────────────────────────
Adafruit_LSM9DS1 lsm;
BMI270 fingerIMU;

extern Madgwick fingerFusion[5];
extern Madgwick handFusion;
extern LeverArmCalibration leverCal;

// ── SENSOR READY FLAGS ─────────────────────────────────────────────
bool lsmReady = false;
bool bmiReady[5] = {false, false, false, false, false};

// ── CHANNEL MAP ────────────────────────────────────────────────────
uint8_t channelMap[5] = {
  INDEX_CHANNEL,
  MIDDLE_CHANNEL,
  RING_CHANNEL,
  THUMB_CHANNEL,
  PINKY_CHANNEL
};

// ── TCA9548A CHANNEL SELECT ────────────────────────────────────────
void selectChannel(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// ── INIT ALL SENSORS ───────────────────────────────────────────────
void initSensors() {
  // LSM9DS1
  if (lsm.begin()) {
    lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
    lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
    lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);
    lsmReady = true;
    Serial.println("LSM9DS1 ready");
  } else {
    Serial.println("LSM9DS1 not found");
  }

  // BMI270s
  for (int i = 0; i < NUM_FINGERS; i++) {
    selectChannel(channelMap[i]);
    if (fingerIMU.beginI2C() == BMI2_OK) {
      bmiReady[i] = true;
      Serial.print("BMI270 ready ch");
      Serial.println(channelMap[i]);
    } else {
      Serial.print("BMI270 not found ch");
      Serial.println(channelMap[i]);
    }
  }
}

// ── HELPER FUNCTIONS ───────────────────────────────────────────────
float calcRotationSpeed(float gx, float gy, float gz) {
  return sqrtf(gx*gx + gy*gy + gz*gz);
}

float calcAccelMagnitude(float ax, float ay, float az) {
  return sqrtf(ax*ax + ay*ay + az*az);
}

float calcDirection(float value) {
  if (value >  0.1f) return  1.0f;
  if (value < -0.1f) return -1.0f;
  return 0.0f;
}

// ── READ HAND LSM9DS1 ──────────────────────────────────────────────
void readHand(HandReading &reading) {
  if (!lsmReady) return;

  lsm.read();
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp);

  float gx = g.gyro.x;
  float gy = g.gyro.y;
  float gz = g.gyro.z;
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;
  float mx = m.magnetic.x;
  float my = m.magnetic.y;
  float mz = m.magnetic.z;

  reading.gyroX  = gx; reading.gyroY  = gy; reading.gyroZ  = gz;
  reading.accelX = ax; reading.accelY = ay; reading.accelZ = az;
  reading.magX   = mx; reading.magY   = my; reading.magZ   = mz;

  reading.rotationSpeed      = calcRotationSpeed(gx, gy, gz);
  reading.accelMagnitude     = calcAccelMagnitude(ax, ay, az);
  reading.rotationDirectionX = calcDirection(gx);
  reading.rotationDirectionY = calcDirection(gy);
  reading.rotationDirectionZ = calcDirection(gz);

  handFusion.update(gx, gy, gz, ax, ay, az, mx, my, mz);

  reading.roll    = handFusion.getRoll();
  reading.pitch   = handFusion.getPitch();
  reading.yaw     = handFusion.getYaw();
  reading.heading = reading.yaw;
  reading.timestamp = millis();
}

// ── READ ONE FINGER BMI270 ─────────────────────────────────────────
void readFinger(int fingerIndex,
                IMUReading &reading,
                HandReading &handRef) {

  if (!bmiReady[fingerIndex]) return;

  selectChannel(channelMap[fingerIndex]);

  if (fingerIMU.getSensorData() != BMI2_OK) return;

  float gx = fingerIMU.data.gyroX;
  float gy = fingerIMU.data.gyroY;
  float gz = fingerIMU.data.gyroZ;
  float ax = fingerIMU.data.accelX;
  float ay = fingerIMU.data.accelY;
  float az = fingerIMU.data.accelZ;

  reading.gyroX  = gx; reading.gyroY  = gy; reading.gyroZ  = gz;
  reading.accelX = ax; reading.accelY = ay; reading.accelZ = az;

  reading.rotationSpeed      = calcRotationSpeed(gx, gy, gz);
  reading.accelMagnitude     = calcAccelMagnitude(ax, ay, az);
  reading.rotationDirectionX = calcDirection(gx);
  reading.rotationDirectionY = calcDirection(gy);
  reading.rotationDirectionZ = calcDirection(gz);

  fingerFusion[fingerIndex].updateIMU(gx, gy, gz, ax, ay, az);

  reading.roll  = fingerFusion[fingerIndex].getRoll();
  reading.pitch = fingerFusion[fingerIndex].getPitch();
  reading.yaw   = fingerFusion[fingerIndex].getYaw();

  reading.relativeRoll  = reading.roll  - handRef.roll;
  reading.relativePitch = reading.pitch - handRef.pitch;
  reading.relativeYaw   = reading.yaw   - handRef.yaw;

  reading.timestamp = millis();
}

// ── READ ALL SENSORS ───────────────────────────────────────────────
void readAllSensors(GloveState &state) {
  readHand(state.hand);

  for (int i = 0; i < NUM_FINGERS; i++) {
    readFinger(i, state.finger[i], state.hand);
  }

  for (int i = 0; i < NUM_FINGERS; i++) {
    float rawCurlAngle = state.finger[i].relativePitch;
    float correctedRate = correctFingerRate(
      state.finger[i].gyroY,
      state.hand.gyroY,
      leverCal.calibrated ? leverCal.ratio[i] : 0.0f
    );
    state.fingerCurlAngle[i]  = rawCurlAngle;
    state.fingerCurlRate[i]   = correctedRate;
    state.fingerCurling[i]    = correctedRate >  5.0f;
    state.fingerExtending[i]  = correctedRate < -5.0f;
  }

  state.packetNumber++;
}