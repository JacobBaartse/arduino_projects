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
  RTC.begin();
  matrix.begin();
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network.begin(radioChannel, base_node); // (channel, node address)
  radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);

  //pinMode(TFT_BL, OUTPUT);
  //tft_off();

  Serial.print(F("Starting up UNO R4 WiFi"));
  Serial.flush();

  String timestamp = __TIMESTAMP__;
  Serial.print(F("Creation/build time: "));
  Serial.println(timestamp);
  Serial.flush(); 

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION){
    Serial.println("Please upgrade the firmware for the WiFi module");
  }

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

  network.update();

  if(sendDirect){
    Serial.print(F(" send direct about to happen")); 
  }
  else {
    // Receive a message from base if available
    readaction = receiveRFnetwork();
    if(readaction > 0){


    }
  }

  // Send to the base node every x seconds or immediate
  writeaction = transmitRFnetwork(sendDirect);

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
