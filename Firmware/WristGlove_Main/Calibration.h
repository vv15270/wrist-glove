#pragma once

#include <Preferences.h>
#include "LeverArm.h"
#include "SensorData.h"

// Preferences is the ESP32 built in flash storage library
// Works like a key-value database saved permanently on chip
Preferences prefs;

// ── SAVE CALIBRATION TO FLASH ──────────────────────────────────────
// Call after calibration completes
// Data survives power off and reboot

void saveCalibration(LeverArmCalibration &cal) {
  prefs.begin("glove", false);  // open namespace "glove" read/write
  for (int i = 0; i < 5; i++) {
    String key = "ratio" + String(i);
    prefs.putFloat(key.c_str(), cal.ratio[i]);
  }
  prefs.putBool("calibrated", cal.calibrated);
  prefs.end();
  Serial.println("Calibration saved to flash");
}

// ── LOAD CALIBRATION FROM FLASH ────────────────────────────────────
// Call once in setup()
// Returns true if valid calibration data exists
// Returns false if first boot or data was cleared

bool loadCalibration(LeverArmCalibration &cal) {
  prefs.begin("glove", true);  // open read only
  bool exists = prefs.getBool("calibrated", false);

  if (!exists) {
    prefs.end();
    Serial.println("No calibration found — needs calibration");
    return false;
  }

  for (int i = 0; i < 5; i++) {
    String key = "ratio" + String(i);
    cal.ratio[i] = prefs.getFloat(key.c_str(), 0.0f);
  }

  cal.calibrated = true;
  prefs.end();
  Serial.println("Calibration loaded from flash");
  return true;
}

// ── CLEAR CALIBRATION ──────────────────────────────────────────────
// Call to force recalibration on next boot

void clearCalibration() {
  prefs.begin("glove", false);
  prefs.clear();
  prefs.end();
  Serial.println("Calibration cleared");
}

// ── RUN CALIBRATION SEQUENCE ───────────────────────────────────────
// Call when user triggers calibration gesture
// Collects sensor readings over a short window
// Calculates and saves lever arm ratios
// gloveState must be actively updating when this runs

void runCalibration(LeverArmCalibration &cal,
                    GloveState &state) {

  Serial.println("Starting calibration — hold fingers straight");
  Serial.println("Rotate wrist slowly side to side for 3 seconds");

  // Collect readings over 3 seconds at LOOP_HZ
  int samples = LOOP_HZ * 3;
  float wristRateSum = 0.0f;
  float fingerRateSum[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

  for (int s = 0; s < samples; s++) {
    // Accumulate wrist yaw rate
    wristRateSum += fabsf(state.hand.gyroZ);

    // Accumulate each finger apparent rate
    for (int i = 0; i < 5; i++) {
      fingerRateSum[i] += fabsf(state.finger[i].gyroZ);
    }

    delay(1000 / LOOP_HZ);
  }

  // Average over all samples
  float wristRateAvg = wristRateSum / samples;
  float fingerRateAvg[5];
  for (int i = 0; i < 5; i++) {
    fingerRateAvg[i] = fingerRateSum[i] / samples;
  }

  // Calculate ratios and save
  calibrateLeverArm(cal, wristRateAvg, fingerRateAvg);
  saveCalibration(cal);

  Serial.println("Calibration complete");
}