#pragma once

class Madgwick {
public:
  float beta;
  float q0, q1, q2, q3;

  Madgwick();
  void begin(float sampleFrequency);
  void update(float gx, float gy, float gz,
              float ax, float ay, float az,
              float mx, float my, float mz);
  void updateIMU(float gx, float gy, float gz,
                 float ax, float ay, float az);
  float getRoll();
  float getPitch();
  float getYaw();

private:
  float invSqrt(float x);
  float sampleFreq;
};