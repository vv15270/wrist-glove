#include <Wire.h>

void testPin(int pin) {
  pinMode(pin, INPUT_PULLUP);
  delay(10);
  int high = digitalRead(pin);
  
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(10);
  int low = digitalRead(pin);
  
  pinMode(pin, INPUT);
  
  Serial.print("GPIO"); Serial.print(pin);
  Serial.print(" pullup="); Serial.print(high);
  Serial.print(" drive_low="); Serial.println(low);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Testing GPIO pins...");
  
  for (int i = 1; i <= 12; i++) {
    testPin(i);
  }
  Serial.println("Done");
}

void loop() {}