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
bool wifiactive = false;

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

  bdisplay_setup();

  sensors_setup();

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
    bdisplay_textline(ipaddresstext);
    startupscrollingtext(String("-->: ") + ipaddresstext);

    Serial.println(F("\nStarting connection to get actual time from the internet"));
    get_time_from_hsdesign();
  }
  // Retrieve the date and time from the RTC and print them
  RTC.getTime(currentTime); 
  bdisplay_textline(currentTime);

  Serial.print(F("The time is now: "));
  Serial.println(currentTime);

  Serial.println();  
  Serial.println(F(" **************"));  
  Serial.println();  
  Serial.flush(); 
  
  delay(2000); // give time to read the time of the display
  bdisplay_textline(""); // clear display
}


bool alarming = true; // should become: false;
bool sendDirect = false;
int readaction = 0;
int writeaction = 0;
bool new_sensing = false;
bool doshow0 = true;
bool doshow1 = true;
int secs = 0;
int remsecs = 60;
int showdata = 0;
String timeinformation = "-";

void loop() {

  // show something on the LED matrix 
  if (alarming) {
    alarming = alarmingsequence();

    bdisplay_loop(); // demo on display
  }
  else {
    loadsequencepicture();

    read_sensors();
    
    showdata = toggle_data(1, 5000);
    if (showdata == 0){
      if (doshow0){
        bdisplay_readings((float)sensor1_temp/10, (float)sensor2_temp/10, sensor1_humi, sensor2_pres);
        doshow0 = false;
      }
      doshow1 = true;
      remsecs = 60;
    }
    else if (showdata == 1){
      RTC.getTime(currentTime); 
      secs = currentTime.getSeconds();
      doshow1 = (secs != remsecs); // count the seconds on display
      if (doshow1){
        timeinformation = bdisplay_readingtime((float)sensor1_temp/10, currentTime.getHour(), currentTime.getMinutes(), secs);
        remsecs = secs;
        doshow1 = false;
      }
      doshow0 = true;
    }
    else{
      Serial.print(showdata);  
      Serial.println(F(" ERROR xxxxxxxxxxxxxxxxxxxxxxxxxxxx"));  
    }
  }

  if (wifiactive){
    websitehandling((float)sensor1_temp/10, (float)sensor2_temp/10, sensor1_humi, sensor2_pres, timeinformation);
  }
}
