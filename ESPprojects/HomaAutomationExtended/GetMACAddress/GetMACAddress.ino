#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Ensure WiFi is initialized
  WiFi.mode(WIFI_AP_STA);

  // Get Station MAC
  Serial.print("Station MAC: ");
  Serial.println(WiFi.macAddress());

  // Get SoftAP MAC
  Serial.print("SoftAP MAC: ");
  Serial.println(WiFi.softAPmacAddress());
}

void loop() {
  
}
