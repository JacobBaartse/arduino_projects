#include <SoftwareSerial.h>

// Use D5 (RX) and D6 (TX)
SoftwareSerial mySerial(D5, D6); 

void setup() {
  Serial.begin(115200);   // USB Serial
  mySerial.begin(9600);   // Software Serial to other D1 Mini
  Serial.println("Sender Initialized");
}

void loop() {
  mySerial.println("Hello from Board 1");
  Serial.println("Sent message...");
  delay(2000);
}
