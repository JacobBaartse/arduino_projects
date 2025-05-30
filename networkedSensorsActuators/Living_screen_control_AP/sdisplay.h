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
#include <Adafruit_GFX.h> 
#include "font_16pix_high.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include <Adafruit_SSD1306.h> // Adafruit SSD 1306 by Adafruit

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

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool displaystatus = DisplayState::Off;
void display_oled(bool clear, int x, int y, String text) {
  if (displaystatus == DisplayState::Off) return;
  if (clear) display.clearDisplay();
  display.setCursor(x, y);
  display.print(text);
  display.display();
}

// move text, write old location in background color
void display_move(int x, int y, int nx, int ny, String text) {
  if (displaystatus == DisplayState::Off) return;
  display.setCursor(x, y);
  display.setTextColor(SSD1306_BLACK);
  display.print(text);
  display.setCursor(nx, ny);
  display.setTextColor(SSD1306_WHITE);
  display.print(text);
  display.display();
}

DisplayState setDisplay(DisplayState statustoset){
  static DisplayState displaystatus = DisplayState::Dim;
  switch(statustoset){
    case DisplayState::Dim:
      display.ssd1306_command(SSD1306_DISPLAYON);
      //display.dim(true); // dim display
      //displaystatus = DisplayState::Dim; // this display does not support dim
      //displaystatus = DisplayState::On;
      display.ssd1306_command(SSD1306_SETCONTRAST);
      display.ssd1306_command(1);
      displaystatus = DisplayState::Dim; // this display does not support dim ??
      break;
    case DisplayState::On:
      display.ssd1306_command(SSD1306_DISPLAYON);
      display.dim(false);
      displaystatus = DisplayState::On;
      break;
    //case DisplayState::Off:
    default:
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      displaystatus = DisplayState::Off;
  }
  return displaystatus;
}

void clear_display(){
  display.clearDisplay();
  display.display();
}

String Line1 = "Welcome"; 
String Line2 = "{small display}"; 
String Line3 = "Whats up?";  

int yline1, yline2, yline3;
bool oncecompleted = false;

void sdisplay_setup() {
  //Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, i2c_Address);
  displaystatus = setDisplay(DisplayState::Dim);
  display.clearDisplay();
  display.setFont(&font_16_pix);
  display.setTextSize(1); // 3 lines of 10-12 chars
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false); 
  display.display();

  //x = display.width();
  yline1 = 16;
  yline2 = 38;
  yline3 = 60;

  display_oled(true, 0, yline1, Line1); 
  display_oled(false, 10, yline2, Line2); 
  display_oled(false, 10, yline3, Line3);  
}
