#pragma once

bool isFingerCurled(float curlAngle, float threshold) {
  if (curlAngle > threshold) {
    return true;
  }
  return false;
}

bool isWristAiming(float rotationSpeed, float deadzone) {
  if (rotationSpeed > deadzone) {
    return true;
  }
  return false;
}

bool isFingerCurling(float curlRate, float rateThreshold) {
  if (curlRate > rateThreshold) {
    return true;
  }
  return false;
}

bool isFingerExtending(float curlRate, float rateThreshold) {
  if (curlRate < -rateThreshold) {
    return true;
  }
  return false;
}

bool isPinchHeld(bool pinchState) {
  if (pinchState) {
    return true;
  }
  return false;
}

float getAimSpeed(float rotationSpeed,
                  float deadzone,
                  float sensitivity) {
  if (rotationSpeed < deadzone) {
    return 0.0f;
  }
  return (rotationSpeed - deadzone) * sensitivity;
}