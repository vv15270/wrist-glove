#pragma once

// ── I2C ADDRESSES ──────────────────────────
#define LSM9DS1_AG_ADDR   0x6B  // accel + gyro
#define LSM9DS1_M_ADDR    0x1E  // magnetometer
#define TCA9548A_ADDR     0x70  // multiplexer

// ── TCA9548A CHANNELS ──────────────────────
#define INDEX_CHANNEL     0
#define MIDDLE_CHANNEL    1
#define RING_CHANNEL      2
#define THUMB_CHANNEL     3

// ── NRF24L01 PINS ──────────────────────────
#define NRF_CE_PIN        4
#define NRF_CSN_PIN       5
#define NRF_CHANNEL       76

// ── CONDUCTIVE FABRIC GPIO PINS ────────────
#define PINCH_INDEX_PIN   13
#define PINCH_MIDDLE_PIN  14
#define PINCH_RING_PIN    15

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