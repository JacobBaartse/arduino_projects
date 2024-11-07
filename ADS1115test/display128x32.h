#include "Adafruit_SSD1306.h"
#ifndef DISPLAY_128x32_H
#define DISPLAY_128x32_H


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET     -1 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool display_found = false;


void display_setup() {  

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display_found = false;
  Wire.beginTransmission(SCREEN_ADDRESS);
  byte result = Wire.endTransmission(1);
  if (result == 0) display_found = true;


  if (display_found){
      display.clearDisplay();
      display.setFont(&FreeMono9pt7b);
      display.setTextSize(1);
      display.setTextColor(WHITE);

      // dim the display
      display.ssd1306_command(SSD1306_SETCONTRAST);
      display.ssd1306_command(1);
  }
}


void oled_display(String toprow, String bottomrow){ 
    if (display_found) {
      display.clearDisplay();
      display.setCursor(0,12);
      display.print(toprow);
      display.setCursor(0,27);
      display.print(bottomrow);
      display.display();
    }
}



#endif


