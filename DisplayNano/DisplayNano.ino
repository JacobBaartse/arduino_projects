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
//#include <Adafruit_GFX.h> // already included from font file
#include "FreeSerif12pt7b_special.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include <Adafruit_SH110X.h>

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

bool displaystatus = DisplayState::Dim;
void display_oled(bool clear, int x, int y, String text) {
  if (displaystatus == DisplayState::Off) return;
  if (clear) display.clearDisplay();
  display.setCursor(x,y);
  display.print(text);
  display.display();
}

DisplayState setDisplay(DisplayState statustoset){
  static DisplayState displaystatus = DisplayState::Dim;
  switch(statustoset){
    case DisplayState::Dim:
      display.setContrast(0); // dim display
      displaystatus = DisplayState::Dim;
      break;
    case DisplayState::On:
      displaystatus = DisplayState::On;
      break;
    //case DisplayState::Off:
    default:
      displaystatus = DisplayState::Off;
  }
  return displaystatus;
}

void clear_display(){
  display.clearDisplay();
  display.display();
}

void setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(i2c_Address, true); // Address 0x3C default
  //display.setContrast(0); // dim display
  displaystatus = setDisplay(DisplayState::Dim);

  display.clearDisplay();
  display.setFont(&FreeSerif12pt7b);
  display.setTextSize(1); // 2 lines of 11 chars
  display.setTextColor(SH110X_WHITE);
  display.display();

}

void loop() {

  float tempval = 24.56;
  display_oled(true, 0, 16, String(tempval, 1) + " \x7F"+"C ");  // } \x7F is converted to degrees in this special font.

    // if (charging) {
    //   display_oled(false, 0, 40,String(humid, 0) + " % " + String(temperature_start_battery, 1));
    // }
    // else {
    //   display_oled(false, 0, 40,String(humid, 0) + " %" );
    // }

    // if (Minutes<10) display_oled(false, 0, 63, String(Hour) + ":0" + String(Minutes));
    // else display_oled(false, 0, 63, String(Hour) + ":" + String(Minutes));
    // display_oled(false, 70, 63, getLight_value());

}
