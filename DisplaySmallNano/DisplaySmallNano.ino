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

// String leftrotate(String str, int d){
//    String ans = str.substring(d, str.length() - d) + str.substring(0, d);
//    return ans;
// }

String Line1 = "Welcome \x81"; 
String Line2 = "Demo \x81{small display} \x81"; 
String Line3 = "Whats \x81 up?";  

int prevx, x, minX;
int y4, y2, y3, minY;
bool oncecompleted = false;

void setup() {
  //Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, i2c_Address);
  displaystatus = setDisplay(DisplayState::Dim);
  display.clearDisplay();
  display.setFont(&FreeSerif12pt7b);
  display.setTextSize(1); // 3 lines of 10-12 chars
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(false); 
  display.display();

  x = display.width();
  y4 = 16;
  y2 = 38;
  y3 = 60;
  // minX = -128;
  minX = -200; // depends on length of the text
  minY = -22;

  display_oled(true, 0, y4, Line1); 
  display_oled(false, x, y2, Line2); 
  display_oled(false, x, y3, Line3);  
  prevx = x;
}

void loop() {

  // float tempval = 24.567;
  // display_oled(true, 0, 16, String(tempval, 1) + " \x7F"+"C");  // } \x7F is converted to degrees in this special font.
  // delay(2000);

  display_move(prevx, y2, x, y2, Line2);
  if (!oncecompleted){
    display_move(prevx, y3, x, y3, Line3);
  }

  prevx = x;
  x = x - 3;
  if (x < minX) x = display.width();
  if (x < 12) oncecompleted = true;
  //if (--x < minX) x = display.width(); // this line is moving 1 pixel at the time, this can be too slow

}
