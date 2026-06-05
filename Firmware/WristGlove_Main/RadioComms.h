#pragma once

#include <RF24.h>
#include "SensorData.h"
#include "config.h"

#define GLOVE_ADDRESS "GLOVE"

bool transmitGloveState(GloveState &state, RF24 &radio) {
  return radio.write(&state, sizeof(GloveState));
}

void initRadioTransmitter(RF24 &radio) {
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(NRF_CHANNEL);
  radio.openWritingPipe((const uint8_t*)GLOVE_ADDRESS);
  radio.stopListening();
  Serial.println("Radio transmitter ready");
}

void initRadioReceiver(RF24 &radio) {
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(NRF_CHANNEL);
  radio.openReadingPipe(1, (const uint8_t*)GLOVE_ADDRESS);
  radio.startListening();
  Serial.println("Radio receiver ready");
}

bool receiveGloveState(GloveState &state, RF24 &radio) {
  if (radio.available()) {
    radio.read(&state, sizeof(GloveState));
    return true;
  }
  return false;
}