/*
  
  WebClient receiving/fetching data from WebServer

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>

ESP8266WiFiMulti WiFiMulti;

const int led = LED_BUILTIN;
int led_val = 2;
int clientid = 1;
unsigned long runningtime = 0;

bool timelapsed(unsigned long timestamp, unsigned long duration){
  static unsigned long tracktime = 0;
  if(timestamp < tracktime) return false;
  tracktime = millis() + duration;
  Serial.print(F("Setting next request "));
  Serial.println(tracktime);
  return true;
}

void setup() {
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

  Serial.println();
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "PASSWORD");

  Serial.println("HTTP client started");
  digitalWrite(led, 1); // turn onboard LED off
}

bool dorequest = false; // request updates regurarly
WiFiClient client;
HTTPClient http;
String ledrequest = "http://192.168.4.1/led?led1=a";

void loop() {

  runningtime = millis();

  dorequest = timelapsed(runningtime, 10000);

  if (dorequest){

    // wait for WiFi connection
    if ((WiFiMulti.run() == WL_CONNECTED)) {

      Serial.print("[HTTP] begin...\n");
      // configure traged server and url

      http.begin(client, ledrequest);

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          Serial.print(F("Received payload: '"));
          Serial.print(payload);
          Serial.println(F("'"));
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();

    }
  }


}
