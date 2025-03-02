/*
  Arduino Wireless Network - Multiple NRF24L01 Tutorial
          == Base/Master Node 00 ==
  by Dejan, www.HowToMechatronics.com
  Libraries:
  nRF24/RF24, https://github.com/nRF24/RF24
  nRF24/RF24Network, https://github.com/nRF24/RF24Network


  https://howtomechatronics.com/tutorials/arduino/how-to-build-an-arduino-wireless-network-with-multiple-nrf24l01-modules/#h-base-00-source-code

*/

#include "matrix.h"
#include "networking.h"
// #include "WiFiS3.h" // already included in networking.h
#include "RTC.h"
#include "clock.h"
#include "website.h"
#include "bdisplay.h"


IPAddress IPhere;

void setup() {
  Serial.begin(115200);
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 
  
  RTC.begin();
  matrix.begin();

  Serial.println(F("Starting up UNO R4 WiFi"));
  Serial.flush();

  // String fv = WiFi.firmwareVersion();
  // if (fv < WIFI_FIRMWARE_LATEST_VERSION){
  //   Serial.println("Please upgrade the firmware for the WiFi module");
  // }

  // attempt to connect to WiFi network:
  int wifistatus = WifiConnect();
  if (wifistatus == WL_CONNECTED){
    server.begin();
    IPhere = printWifiStatus(connection);
  }
  else{ // stop the wifi connection
    WiFi.disconnect();
  }
  startupscrollingtext(String("-->: ") + IPhere.toString());

  Serial.println(F("\nStarting connection to get actual time from the internet"));
  get_time_from_hsdesign();
  // Retrieve the date and time from the RTC and print them
  RTCTime currentTime;
  RTC.getTime(currentTime); 
  Serial.println(F("The RTC is: "));
  Serial.println(currentTime);

  bdisplay_setup();

  Serial.println();  
  Serial.println(F(" **************"));  
  Serial.println();  
  Serial.flush(); 
}

// void restart_arduino(){
//   Serial.println("Restart the Arduino UNO R4 board...");
//   delay(2000);
//   NVIC_SystemReset();
// }


bool alarming = true; // should become: false;
bool sendDirect = false;
unsigned int readaction = 0;
unsigned int writeaction = 0;

void loop() {

  // show something on the LED matrix 
  if (alarming) {
    alarming = alarmingsequence();
  }
  else {
    loadsequencepicture();
  }

  bdisplay_loop();

  websitehandling();

}
