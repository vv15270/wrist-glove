#pragma once

#include "config.h"
#include "SensorData.h"
#include "LeverArm.h"
#include "Madgwick.h"
#include <Wire.h>
#include <math.h>

extern Madgwick fingerFusion[5];
extern Madgwick handFusion;
extern LeverArmCalibration leverCal;

void selectChannel(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

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

void readHand(HandReading &reading) {
  float gx = 0.0f, gy = 0.0f, gz = 0.0f;
  float ax = 0.0f, ay = 0.0f, az = 1.0f;
  float mx = 0.0f, my = 0.0f, mz = 0.0f;

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

void readFinger(int fingerIndex,
                uint8_t channel,
                IMUReading &reading,
                HandReading &handRef) {
  selectChannel(channel);

  float gx = 0.0f, gy = 0.0f, gz = 0.0f;
  float ax = 0.0f, ay = 0.0f, az = 1.0f;

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

void readAllSensors(GloveState &state) {
  readHand(state.hand);

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