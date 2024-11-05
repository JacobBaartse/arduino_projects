/*
 *  Experiment with the OLED display
 *  
 *  
 *  https://learn.adafruit.com/creating-custom-symbol-font-for-adafruit-gfx-library
 *  https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
 *  
 */

#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>
#include "FreeSerif12pt7b_special.h"  //https://tchapi.github.io/Adafruit-GFX-Font-Customiser/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define OLED_RESET     -1


void setup() {
  Serial.begin(115200);

}

void loop() {

}
