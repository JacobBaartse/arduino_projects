/*
 * get actual time information from https://www.tdic.nl/localtime.php
 * or from https://www.spoorstra.nl/bcso/localtime.php
 *
 */
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTC.h"
#include "Wire.h"
#include "RTClib.h"

RTCTime currentTime;

RTC_DS3231 myRTC;

void set_clock(unsigned long unix_time){
  currentTime.setUnixTime(unix_time);
  RTC.setTime(currentTime);
  // if (myRTC.lostPower()) {
  //   Serial.println(F("RTC lost power, lets set the time!"));
  //   //myRTC.setTime(unix_time);
  // }
  myRTC.adjust(unix_time);
  Serial.print(F("set_clock: ")); 
  Serial.println(unix_time);
}

char buffer[8] = "";
String timeformatting(int hours, int minutes, int secs){
  sprintf(buffer, "%02d:%02d:%02d", hours, minutes, secs);
  String timeformat = String(buffer);
  return timeformat; 
}

String get_clock(){
    RTC.getTime(currentTime); 
    DateTime rtcnow =	myRTC.now();
    String timeinformation = timeformatting(currentTime.getHour(), currentTime.getMinutes(), currentTime.getSeconds());
    
    // byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
    // // retrieve data from DS3231
    // readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
    // String mytimeinformation = timeformatting(hour, minute, second);
    String mytimeinformation = timeformatting(rtcnow.hour(), rtcnow.minute(), rtcnow.second());

    //String mytimeinformation = myRTC.dateFormat("U", rtcnow);
    Serial.print(F("The time is: "));
    Serial.print(timeinformation); 
    Serial.print(F(", ")); 
    Serial.print(mytimeinformation);
    Serial.print(F(", ")); 
    Serial.println(rtcnow.unixtime());
    return timeinformation;
}

void restart_uno(){
  Serial.println("Restart the UNO R4 WiFi...");
  delay(2000);
  NVIC_SystemReset();
}

bool update_time(unsigned long ftime, unsigned long checkinterval){
  static unsigned long timetime = 0;
  if (ftime < timetime) return false;
  timetime = (unsigned long) ftime + checkinterval;
  return true;
}

int toggle_data(unsigned long ftime, int choises, unsigned long toggle_time){
  static int datatracking = 0;
  if (update_time(ftime, toggle_time)){
    if (++datatracking > choises){ 
      datatracking = 0;
    }
  }
  return datatracking;
}
