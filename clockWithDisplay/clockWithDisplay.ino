// now.pde
// Prints a snapshot of the current date and time along with the UNIX time
// Modified by Andy Wickert from the JeeLabs / Ladyada RTC library examples
// 5/15/11

#include <Wire.h>
#include <DS3231.h>

#include <Adafruit_SSD1306.h>  //Adafruit SSD 1306 by Adafruit
#include <Adafruit_GFX.h>
#include "font_16pix_high.h"


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET     -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

RTClib myRTC;

void setup () {
  Serial.begin(115200);
  Wire.begin();
  delay(500);
  Serial.println("Ready!");

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setFont(&font_16_pix);
  display.setTextSize(2);
  display.setTextColor(WHITE);
}

void loop () {
    
  delay(1000);
  
  DateTime now = myRTC.now();
  
  // Serial.print(now.year(), DEC);
  // Serial.print('/');
  // Serial.print(now.month(), DEC);
  // Serial.print('/');
  // Serial.print(now.day(), DEC);
  // Serial.print(' ');
  // Serial.print(now.hour(), DEC);
  // Serial.print(':');
  // Serial.print(now.minute(), DEC);
  // Serial.print(':');
  // Serial.print(now.second(), DEC);
  // Serial.println();
  
  // Serial.print(" since midnight 1/1/1970 = ");
  // Serial.print(now.unixtime());
  // Serial.print("s = ");
  // Serial.print(now.unixtime() / 86400L);
  // Serial.println("d");

  String the_time = "";
  int hour = (now.hour() +2 ) % 24;
  if ( hour < 10) the_time += String(" ");
  the_time += String(hour);
  the_time += String(":") ;
  if (now.minute()<10) the_time += String("0");
  the_time += String(now.minute());
  the_time += String(":");
  if (now.second()<10) the_time += String("0");
  the_time += String(now.second());



  display.clearDisplay();
  display.setCursor(0,32);
  display.print(the_time);  
  display.display();
}