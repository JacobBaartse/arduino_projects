/*

Scan Wifi scannels

 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const int led = LED_BUILTIN;

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0); // turn onboard LED on
  Serial.begin(115200);

  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.println(F("Starting ESP device"));
  Serial.flush();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  digitalWrite(led, 1); // turn onboard LED off
}

void loop(void) {

  Serial.println(millis());
  int n = WiFi.scanNetworks();
  for (int i=0; i<n; i++) {
    Serial.print(F("SSID "));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.print(WiFi.SSID(i));
    Serial.print(F(", "));
    Serial.print(WiFi.RSSI(i)); // Power level
    Serial.print(F(", channel "));
    Serial.println(WiFi.channel(i));
  }
  Serial.println(F("----------------------"));

  delay(10000);
}
