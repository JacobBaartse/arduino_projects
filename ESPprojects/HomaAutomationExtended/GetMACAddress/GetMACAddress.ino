#include <ESP8266WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.println(F("Starting ESP device"));
  Serial.flush();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

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
