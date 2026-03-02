#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
}
