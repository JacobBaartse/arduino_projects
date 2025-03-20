
#include <Adafruit_SSD1306.h>  //Adafruit SSD 1306 by Adafruit
#include <Adafruit_GFX.h>
#include "font_16pix_high.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET     -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only

  }
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setFont(&font_16_pix);
  display.setTextSize(1);
  display.setTextColor(WHITE);
}



void loop() {

  display.clearDisplay();
  display.setCursor(0,16);
  display.print("0123456789AB");  
  display.setCursor(0,32);
  display.print("CDEFabcdefghij");
  display.display();
  delay(10000);


  display.clearDisplay();
  display.setCursor(0,16);
  display.print("klmnopqrstuvwxy");  
  display.setCursor(0,32);
  display.print("zGHIJKLMNOPQRS");
  display.display();
  delay(10000);

  display.clearDisplay();
  display.setCursor(0,16);
  display.print("TUVWXYZabc");  
  display.setCursor(0,32);
  display.print("AaBbCcDdEe");
  display.display();
  delay(10000);


}
