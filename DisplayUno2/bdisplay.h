/*
 *  Experiment with the OLED displays
 *  0.96 inch  SSD1315, using the SSD1306 library
 *  1.3 inch   SH1106 using the SH110X library
 *
 *  https://learn.adafruit.com/creating-custom-symbol-font-for-adafruit-gfx-library
 *  https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
 *  
 */

#include <Wire.h>
#include <Adafruit_GFX.h> // already included from font file
#include "font_16pix_high.h"
#include <Adafruit_SH110X.h> // Adafruit SH110X by Adafruit


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3C //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3D //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define OLED_RESET -1

enum DisplayState {
    Off = 0,
    Dim = 1,
    On = 2,
};

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool displaystatus = DisplayState::Off;
void display_oled(bool clear, int x, int y, String text) {
  if (displaystatus == DisplayState::Off) return;
  if (clear) display.clearDisplay();
  display.setCursor(x, y);
  display.print(text);
  display.display();
}

DisplayState setDisplay(DisplayState statustoset){
  static DisplayState displaystatus = DisplayState::Dim;
  switch(statustoset){
    case DisplayState::Dim:
      display.oled_command(SH110X_DISPLAYON);
      display.setContrast(0); // dim display
      displaystatus = DisplayState::Dim;
      break;
    case DisplayState::On:
      display.oled_command(SH110X_DISPLAYON);
      displaystatus = DisplayState::On;
      break;
    //case DisplayState::Off:
    default:
      display.oled_command(SH110X_DISPLAYOFF);
      displaystatus = DisplayState::Off;
  }
  return displaystatus;
}

void clear_display(){
  display.clearDisplay();
  display.display();
}

void bdisplay_textline(String textline){
  display.setTextWrap(true);
  display_oled(true, 0, 32, textline);  
  display.setTextWrap(false);
}

void bdisplay_setup() {
  display.begin(i2c_Address, true); // Address 0x3C default
  displaystatus = setDisplay(DisplayState::Dim);
  display.clearDisplay();
  display.setFont(&font_16_pix);
  display.setTextSize(1); // 3 lines of 10-12 chars
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);
  display.display();

  bdisplay_textline(F("Welkom")); 
}

char buffer[5] = "";
String bdisplay_readings(float temp1, float temp2, int humid, int pressure, int hours, int minutes){
  display_oled(true, 0, 16, String(temp1, 1) + " \x7F"+"C");  // \x7F is converted to degrees in this special font.
  //display_oled(false, 75, 16, String(temp2, 1) + " \x7F"+"C");  
  display_oled(false, 0, 32, String(humid) + "% rH");  
  display_oled(false, 0, 48, String(pressure) + " hPa");
  sprintf(buffer, "%02d:%02d", hours, minutes);
  String timeformat = String(buffer);
  display_oled(false, 0, 64, timeformat);  
  return timeformat; 
}
