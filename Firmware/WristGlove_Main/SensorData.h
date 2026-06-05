#pragma once

struct IMUReading {
  float gyroX, gyroY, gyroZ;
  float accelX, accelY, accelZ;
  float rotationSpeed;
  float accelMagnitude;
  float rotationDirectionX;
  float rotationDirectionY;
  float rotationDirectionZ;
  float roll, pitch, yaw;
  float relativeRoll, relativePitch, relativeYaw;
  unsigned long timestamp;
};

struct HandReading {
  float gyroX, gyroY, gyroZ;
  float accelX, accelY, accelZ;
  float magX, magY, magZ;
  float rotationSpeed;
  float accelMagnitude;
  float rotationDirectionX;
  float rotationDirectionY;
  float rotationDirectionZ;
  float roll, pitch, yaw;
  float heading;
  unsigned long timestamp;
};

struct GloveState {
  IMUReading  finger[5];
  HandReading hand;
  float       fingerCurlAngle[5];
  float       fingerCurlRate[5];
  bool        fingerCurling[5];
  bool        fingerExtending[5];
  bool        pinch[3];
  bool        calibrateRequested;
  uint32_t    packetNumber;
};