#pragma once

struct LeverArmCalibration {
    float ratio[5];
    bool  calibrated;
};

float correctFingerRate(float measuredRate,
                        float wristRate,
                        float fingerRatio) {
    float apparentRate  = wristRate * fingerRatio;
    float correctedRate = measuredRate - apparentRate;
    return correctedRate;
}

void calibrateLeverArm(LeverArmCalibration &cal,
                       float wristRate,
                       float fingerRates[5]) {
    if (wristRate == 0.0f) {
        Serial.println("Calibration failed — wrist not moving");
        return;
    }
    for (int i = 0; i < 5; i++) {
        cal.ratio[i] = fingerRates[i] / wristRate;
    }
    cal.calibrated = true;
    Serial.println("Lever arm calibration complete");
}

void correctAllFingers(float measuredRates[5],
                       float wristRate,
                       LeverArmCalibration &cal,
                       float correctedRates[5]) {
    if (!cal.calibrated) {
        for (int i = 0; i < 5; i++) {
            correctedRates[i] = measuredRates[i];
        }
        return;
    }
    for (int i = 0; i < 5; i++) {
        correctedRates[i] = correctFingerRate(
            measuredRates[i],
            wristRate,
            cal.ratio[i]
        );
    }
}