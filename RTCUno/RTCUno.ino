/*

DS3231 RTC Module on a UNO R4 WiFi

https://www.jackyb.be/arduino/arduino/real-time-clock-ds3231.php

https://howtomechatronics.com/tutorials/arduino/arduino-ds3231-real-time-clock-tutorial/

https://www.instructables.com/How-to-Create-a-Clock-Using-Arduino-DS3231-RTC-Mod/

https://www.vdrelectronics.com/ds3231-precisie-rtc-module-gebruiken-met-arduino

https://adafruit.github.io/RTClib/html/class_date_time.html


*/

#include "matrix.h"
#include "networking.h"
#include "RTC.h"
#include "clock.h"
#include "webpage.h"


IPAddress IPhere;
bool wifiactive = false;

void setup() {
  Serial.begin(115200);
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 
  
  matrix.begin();
  RTC.begin();
  Wire.begin();
  myRTC.begin();

  Serial.println(F("Starting up UNO R4 WiFi"));
  Serial.flush();

  // String fv = WiFi.firmwareVersion();
  // if (fv < WIFI_FIRMWARE_LATEST_VERSION){
  //   Serial.println("Please upgrade the firmware for the WiFi module");
  // }

  // attempt to connect to WiFi network:
  int wifistatus = WifiConnect();
  if (wifistatus == WL_CONNECTED){
    IPhere = printWifiStatus(connection);
    wifiactive = true;
  }
  else{ // stop the wifi connection
    WiFi.disconnect();
    Serial.println(F("\nWifi disconnected"));
  }
  if (wifiactive){
    server.begin();

    String ipaddresstext = IPhere.toString();
    startupscrollingtext(String("-->: ") + ipaddresstext);

    Serial.println(F("\nStarting connection to get actual time from the internet"));
    get_time_from_hsdesign();
  }
  // Retrieve the date and time from the RTC and print them
  RTC.getTime(currentTime); 

  Serial.print(F("The time is now: "));
  Serial.println(currentTime); 

  Serial.println(F("\n ********"));  
  Serial.println();  
  Serial.flush(); 
}


bool alarming = true; // should become: false;
String timeinformation = "-";
unsigned long runningtiming = 0;
unsigned long gettiming = 0;

void loop() {

  runningtiming = millis();

  // show something on the LED matrix 
  if (alarming) {
    alarming = alarmingsequence();
  }
  else {
    loadsequencepicture();
  }
  
  if ((unsigned long)(runningtiming - gettiming) > 10000){
    gettiming = runningtiming;
    timeinformation = get_clock();
  }

  if (wifiactive){
    webpagehandling(timeinformation);
  }
}
