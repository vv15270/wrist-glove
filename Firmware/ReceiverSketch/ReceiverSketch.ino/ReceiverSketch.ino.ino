#include <SPI.h>
#include <RF24.h>

// ── NRF24L01 PINS FOR ARDUINO NANO ────────────────────────────────
#define CE_PIN  9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);

// ── GLOVE DATA PACKET ──────────────────────────────────────────────
struct GloveState {
  float fingerCurlAngle[5];
  float fingerCurlRate[5];
  bool  fingerCurling[5];
  bool  fingerExtending[5];
  bool  pinch[3];
  bool  calibrateRequested;
  uint32_t packetNumber;
};

GloveState receivedState;

#define GLOVE_ADDRESS "GLOVE"

void setup() {
  Serial.begin(115200);

  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(76);
  radio.openReadingPipe(1, (const uint8_t*)GLOVE_ADDRESS);
  radio.startListening();

  Serial.println("Receiver ready");
}

void loop() {
  if (radio.available()) {
    radio.read(&receivedState, sizeof(GloveState));

    Serial.print("PKT:");
    Serial.print(receivedState.packetNumber);
    Serial.print(" I:");
    Serial.print(receivedState.fingerCurlAngle[0]);
    Serial.print(" M:");
    Serial.print(receivedState.fingerCurlAngle[1]);
    Serial.print(" R:");
    Serial.print(receivedState.fingerCurlAngle[2]);
    Serial.print(" T:");
    Serial.print(receivedState.fingerCurlAngle[3]);
    Serial.print(" P:");
    Serial.print(receivedState.fingerCurlAngle[4]);
    Serial.print(" PIN:");
    Serial.print(receivedState.pinch[0]);
    Serial.print(receivedState.pinch[1]);
    Serial.println(receivedState.pinch[2]);
  }
}