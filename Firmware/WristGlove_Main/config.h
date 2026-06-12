#pragma once

// ── I2C PINS ───────────────────────────────
#define SDA_PIN           7
#define SCL_PIN           8


// ── CONDUCTIVE FABRIC GPIO PINS ────────────
#define PINCH_INDEX_PIN   13
#define PINCH_MIDDLE_PIN  14
#define PINCH_RING_PIN    15

// ── MOTOR ──────────────────────────────────
#define MOTOR_PIN         10

// ── I2C ADDRESSES ──────────────────────────
#define LSM9DS1_AG_ADDR   0x6B
#define LSM9DS1_M_ADDR    0x1E
#define TCA9548A_ADDR     0x70

// ── TCA9548A CHANNELS ──────────────────────
#define INDEX_CHANNEL     2
#define MIDDLE_CHANNEL    3
#define RING_CHANNEL      4
#define THUMB_CHANNEL     5
#define PINKY_CHANNEL     6

// ── FINGER COUNT ───────────────────────────
#define NUM_FINGERS       5

// ── SENSITIVITY ────────────────────────────
#define DEADZONE          2.0f
#define SENSITIVITY       15.0f
#define FAST_THRESHOLD    80.0f

// ── TIMING ─────────────────────────────────
#define LOOP_HZ           200
#define RADIO_HZ          100

// ── DEBOUNCE ───────────────────────────────
#define PINCH_HOLD_MS     80
#define CURL_THRESHOLD    45.0f