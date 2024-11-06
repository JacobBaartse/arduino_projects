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
  display.setCursor(x, y);
  display.print(text);
  display.display();
}

// move text, write old location in background color
void display_move(int x, int y, int nx, int ny, String text) {
  if (displaystatus == DisplayState::Off) return;
  display.setCursor(x, y);
  display.setTextColor(SH110X_BLACK);
  display.print(text);
  display.setCursor(nx, ny);
  display.setTextColor(SH110X_WHITE);
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

String leftrotate(String str, int d){
   String ans = str.substring(d, str.length() - d) + str.substring(0, d);
   return ans;
}

String Line1 = "Welcome"; 
String Line2 = "George {!}"; 
String Line3 = "Whats up? \x81";  

int prevx, x, minX;
int y1, y2, y3, minY;
bool oncecompleted = false;

void setup() {
  //Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(i2c_Address, true); // Address 0x3C default
  //display.setContrast(0); // dim display
  displaystatus = setDisplay(DisplayState::Dim);

  display.clearDisplay();
  display.setFont(&FreeSerif12pt7b);
  display.setTextSize(1); // 2 lines of 11 chars
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);
  display.display();

  x = display.width();
  y1 = 16;
  y2 = 37;
  y3 = 58;
  minX = -128;
  minY = -22;

  display_oled(true, 0, y1, Line1); 
  display_oled(false, x, y2, Line2); 
  display_oled(false, x, y3, Line3);  
  prevx = x;
}

void loop() {

  // float tempval = 24.567;
  // display_oled(true, 0, 16, String(tempval, 1) + " \x7F"+"C");  // } \x7F is converted to degrees in this special font.

  // delay(2000);

  // display_oled(true, 0, 16, "Welcome"); 
  // display_oled(false, x, 37, "George {!}"); 
  // display_oled(false, x, 58, "Whats up? \x81");

  display_move(prevx, y2, x, y2, Line2);
  if (!oncecompleted){
    display_move(prevx, y3, x, y3, Line3);
  }

  prevx = x;
  x = x - 3;
  if (x < minX) x = display.width();
  if (x < 0) oncecompleted = true;
  //if (--x < minX) x = display.width(); // this line is moving 1 pixel at the time, this can be too slow

}