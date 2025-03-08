/*

AHT20   0x38
Display 0x3C
BMP280  0x77

*/

#include "matrix.h"
#include "networking.h"
// #include "WiFiS3.h" // already included in networking.h
#include "RTC.h"
#include "clock.h"
#include "website.h"
#include "bdisplay.h"
#include "temppress.h"


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

  sensors_setup();

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
bool new_sensing = false;

void loop() {

  // show something on the LED matrix 
  if (alarming) {
    alarming = alarmingsequence();

    bdisplay_loop();
  }
  else {
    loadsequencepicture();

    new_sensing = read_sensors();
    if (new_sensing){
      bdisplay_readings((float)sensor1_temp/10, (float)sensor2_temp/10, sensor1_humi, sensor2_pres);
    }
  }

  websitehandling();

}
