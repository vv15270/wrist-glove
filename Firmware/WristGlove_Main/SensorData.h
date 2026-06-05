#pragma once

// ── SINGLE IMU READING ─────────────────────────────────────────────
// Raw data from one BMI270 or LSM9DS1 at one moment in time.
// Captured every loop cycle for every chip.

struct IMUReading {
  // Gyroscope — rate of rotation in degrees per second
  // How fast this sensor is currently rotating around each axis
  float gyroX;    // rotation around X axis — roll rate
  float gyroY;    // rotation around Y axis — pitch rate
  float gyroZ;    // rotation around Z axis — yaw rate

  // Accelerometer — linear acceleration in g (1g = 9.8 m/s²)
  // Includes gravity — used to anchor orientation against drift
  float accelX;   // acceleration along X axis
  float accelY;   // acceleration along Y axis
  float accelZ;   // acceleration along Z axis

  // Speed magnitude — total movement speed regardless of direction
  // Calculated from all three gyro axes combined
  float rotationSpeed;   // sqrt(gyroX² + gyroY² + gyroZ²)
  float accelMagnitude;  // sqrt(accelX² + accelY² + accelZ²)

  // Direction — which way this sensor is currently rotating
  // +1.0 means positive direction, -1.0 means negative
  float rotationDirectionX;  // sign of gyroX
  float rotationDirectionY;  // sign of gyroY
  float rotationDirectionZ;  // sign of gyroZ

  // Fused orientation — output from Madgwick filter
  // Stable angles that don't drift — what you actually use for gestures
  float roll;    // tilt side to side in degrees
  float pitch;   // tilt forward back in degrees
  float yaw;     // rotation left right in degrees

  // Relative orientation — angle relative to hand reference frame
  // This is the finger curl angle after hand motion is subtracted
  float relativeRoll;
  float relativePitch;
  float relativeYaw;

  // Timestamp
  unsigned long timestamp;  // millis() when this was captured
};

// ── HAND SENSOR READING ────────────────────────────────────────────
// LSM9DS1 on dorsal plate — same as IMU but also has magnetometer
// This is the reference frame everything else is measured against

struct HandReading {
  // All the same fields as IMUReading
  float gyroX, gyroY, gyroZ;
  float accelX, accelY, accelZ;
  float magX, magY, magZ;       // magnetometer — unique to LSM9DS1
  float rotationSpeed;
  float accelMagnitude;
  float rotationDirectionX;
  float rotationDirectionY;
  float rotationDirectionZ;
  float roll, pitch, yaw;
  float heading;                // absolute compass heading from magnetometer
  unsigned long timestamp;
};

// ── COMPLETE GLOVE STATE ───────────────────────────────────────────
// Everything the glove knows at one moment in time.
// This is what gets transmitted to the PC every radio cycle.

struct GloveState {
  // Five finger sensors — 0=index 1=middle 2=ring 3=thumb 4=pinky
  IMUReading finger[5];

  // Hand dorsal sensor
  HandReading hand;

  // Derived values — calculated from raw readings
  float fingerCurlAngle[5];     // 0=extended 90=fully curled degrees
  float fingerCurlRate[5];      // how fast each finger is curling deg/sec
  bool  fingerCurling[5];       // is this finger actively moving toward curl
  bool  fingerExtending[5];     // is this finger actively moving toward open

  // Pinch detection
  bool  pinch[3];               // index middle ring — from fabric patches

  // Calibration flag
  bool  calibrateRequested;

  // Packet number for detecting dropped transmissions
  uint32_t packetNumber;
};