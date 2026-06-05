#pragma once

#include "config.h"
#include "SensorData.h"
#include "LeverArm.h"
#include "Madgwick.h"
#include <Wire.h>
#include <math.h>

// ── MADGWICK INSTANCES ─────────────────────────────────────────────
// One filter per sensor — each maintains its own orientation state
// Five fingers plus one hand = six total

extern Madgwick fingerFusion[5];
extern Madgwick handFusion;

// ── LEVER ARM CALIBRATION ──────────────────────────────────────────
extern LeverArmCalibration leverCal;

// ── SELECT TCA9548A CHANNEL ────────────────────────────────────────
void selectChannel(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// ── CALCULATE DERIVED VALUES FROM RAW GYRO ────────────────────────
// Speed magnitude — total rotation speed regardless of axis
float calcRotationSpeed(float gx, float gy, float gz) {
  return sqrtf(gx*gx + gy*gy + gz*gz);
}

// Total acceleration magnitude
float calcAccelMagnitude(float ax, float ay, float az) {
  return sqrtf(ax*ax + ay*ay + az*az);
}

// Direction as +1 or -1 for each axis
float calcDirection(float value) {
  if (value > 0.1f)  return  1.0f;
  if (value < -0.1f) return -1.0f;
  return 0.0f;  // effectively zero — no clear direction
}

// ── READ ONE FINGER BMI270 ─────────────────────────────────────────
// Reads raw data from one finger chip through TCA9548A.
// Runs Madgwick update on it.
// Calculates speed magnitude and direction.
// fingerIndex 0-4, channel is TCA channel for that finger.
// Result written into provided IMUReading struct.

void readFinger(int fingerIndex,
                uint8_t channel,
                IMUReading &reading,
                HandReading &handRef) {

  // Select this finger's channel on multiplexer
  selectChannel(channel);

  // ── READ RAW DATA FROM BMI270 ──────────────────────────────
  // Placeholder — replaced with real BMI270 library calls
  // when hardware arrives. Structure stays identical.
  float gx = 0.0f, gy = 0.0f, gz = 0.0f;
  float ax = 0.0f, ay = 0.0f, az = 1.0f;  // 1g gravity default

  // Store raw gyroscope readings
  reading.gyroX = gx;
  reading.gyroY = gy;
  reading.gyroZ = gz;

  // Store raw accelerometer readings
  reading.accelX = ax;
  reading.accelY = ay;
  reading.accelZ = az;

  // ── CALCULATE SPEED AND DIRECTION ─────────────────────────
  reading.rotationSpeed = calcRotationSpeed(gx, gy, gz);
  reading.accelMagnitude = calcAccelMagnitude(ax, ay, az);
  reading.rotationDirectionX = calcDirection(gx);
  reading.rotationDirectionY = calcDirection(gy);
  reading.rotationDirectionZ = calcDirection(gz);

  // ── UPDATE MADGWICK FILTER ─────────────────────────────────
  // No magnetometer on finger chips — use IMU only update
  fingerFusion[fingerIndex].updateIMU(gx, gy, gz, ax, ay, az);

  // Get fused orientation angles
  reading.roll  = fingerFusion[fingerIndex].getRoll();
  reading.pitch = fingerFusion[fingerIndex].getPitch();
  reading.yaw   = fingerFusion[fingerIndex].getYaw();

  // ── CALCULATE RELATIVE ORIENTATION ────────────────────────
  // Subtract hand orientation to get finger angle relative to hand
  // This is the actual curl angle independent of hand movement
  reading.relativeRoll  = reading.roll  - handRef.roll;
  reading.relativePitch = reading.pitch - handRef.pitch;
  reading.relativeYaw   = reading.yaw   - handRef.yaw;

  // Timestamp
  reading.timestamp = millis();
}

// ── READ HAND LSM9DS1 ──────────────────────────────────────────────
// Reads the dorsal plate sensor — hand reference frame.
// Must be called BEFORE reading fingers each loop cycle
// because finger readings subtract the hand orientation.

void readHand(HandReading &reading) {

  // ── READ RAW DATA FROM LSM9DS1 ─────────────────────────────
  // Placeholder — replaced with real Adafruit LSM9DS1 calls
  float gx = 0.0f, gy = 0.0f, gz = 0.0f;
  float ax = 0.0f, ay = 0.0f, az = 1.0f;
  float mx = 0.0f, my = 0.0f, mz = 0.0f;

  // Store all raw readings
  reading.gyroX = gx; reading.gyroY = gy; reading.gyroZ = gz;
  reading.accelX = ax; reading.accelY = ay; reading.accelZ = az;
  reading.magX = mx; reading.magY = my; reading.magZ = mz;

  // Speed and direction
  reading.rotationSpeed = calcRotationSpeed(gx, gy, gz);
  reading.accelMagnitude = calcAccelMagnitude(ax, ay, az);
  reading.rotationDirectionX = calcDirection(gx);
  reading.rotationDirectionY = calcDirection(gy);
  reading.rotationDirectionZ = calcDirection(gz);

  // ── UPDATE MADGWICK WITH MAGNETOMETER ─────────────────────
  // Hand sensor has magnetometer — use full 9DOF update
  // This gives absolute heading so right always means right
  handFusion.update(gx, gy, gz, ax, ay, az, mx, my, mz);

  reading.roll    = handFusion.getRoll();
  reading.pitch   = handFusion.getPitch();
  reading.yaw     = handFusion.getYaw();
  reading.heading = reading.yaw;  // magnetometer anchors true heading

  reading.timestamp = millis();
}

// ── READ ALL SENSORS INTO GLOVE STATE ─────────────────────────────
// Call this once every loop cycle.
// Reads hand first, then all five fingers in sequence.
// Populates complete GloveState with all measurements.

void readAllSensors(GloveState &state) {

  // ── STEP 1 — READ HAND FIRST ───────────────────────────────
  // Hand must come first — finger readings subtract hand orientation
  readHand(state.hand);

  // ── STEP 2 — READ ALL FIVE FINGERS ────────────────────────
  uint8_t channels[5] = {
    INDEX_CHANNEL,
    MIDDLE_CHANNEL,
    RING_CHANNEL,
    THUMB_CHANNEL,
    PINKY_CHANNEL
  };

  for (int i = 0; i < NUM_FINGERS; i++) {
    readFinger(i, channels[i], state.finger[i], state.hand);
  }

  // ── STEP 3 — CALCULATE CURL ANGLES ────────────────────────
  // Primary curl axis is pitch — forward back rotation
  // Corrected for lever arm geometry using calibrated ratios
  for (int i = 0; i < NUM_FINGERS; i++) {
    // Raw curl angle from relative pitch
    float rawCurlAngle = state.finger[i].relativePitch;

    // Curl rate — how fast this finger is currently curling
    // Use lever arm corrected gyro rate on primary axis
    float correctedRate = correctFingerRate(
      state.finger[i].gyroY,      // measured rate on curl axis
      state.hand.gyroY,            // wrist rate same axis
      leverCal.ratio[i < 4 ? i : 3]  // pinky uses thumb ratio as fallback
    );

    state.fingerCurlAngle[i] = rawCurlAngle;
    state.fingerCurlRate[i]  = correctedRate;

    // Is this finger actively moving
    state.fingerCurling[i]   = correctedRate >  5.0f;   // curling toward fist
    state.fingerExtending[i] = correctedRate < -5.0f;   // opening back up
  }

  // Increment packet counter
  state.packetNumber++;
}