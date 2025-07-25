
// #include <Adafruit_SH110X.h>  //Adafruit SH110X by Adafruit
// #include <Adafruit_GFX.h>
// #include "font_16pix_high.h"  //https://tchapi.github.io/Adafruit-GFX-Font-Customiser/


// if only this display is connected to i2c pullup resistors are needed of about 4k7 ohm

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define display_i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define display_i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define OLED_RESET     -1

#define DEBUG 0
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void display_oled( bool clear, int x, int y, String text, bool activate){
  delay(2);
  debugln(text);
  if (clear) display.clearDisplay();
  display.setCursor(x,y);
  display.print(text);
  if (activate) display.display();
}



void setup_oled_display() {
  display.begin(display_i2c_Address, true);
  display.setContrast (0); // dim display

  display.clearDisplay();
  display.setFont(&font_16_pix);
  display.setTextSize(1);  // 3 lines of 11 chars
  display.setTextColor(SH110X_WHITE);
  display.display();
}

#undef debug
#undef debugln
