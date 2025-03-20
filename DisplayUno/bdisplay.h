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

// String leftrotate(String str, int d){
//    String ans = str.substring(d, str.length() - d) + str.substring(0, d);
//    return ans;
// }

String Line1 = "Welcome \x81"; 
String Line2 = "Demo \x81{characters} \x81"; 
String Line3 = "Whats up \x81?";  

int bprevx, bx, bminX;
int by1, by2, by3, bminY;
bool oncecompleted = false;
bool sensorvalues = false;

void bdisplay_setup() {
  //Serial.begin(115200);

  display.begin(i2c_Address, true); // Address 0x3C default
  displaystatus = setDisplay(DisplayState::Dim);
  display.clearDisplay();
  display.setFont(&FreeSerif12pt7b);
  display.setTextSize(1); // 3 lines of 10-12 chars
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);
  display.display();

  bx = display.width();
  by1 = 16;
  by2 = 38;
  by3 = 60;
  // minX = -128;
  bminX = -200; // depends on length of the text
  bminY = -22;

  display_oled(true, 0, by1, Line1); 
  display_oled(false, bx, by2, Line2); 
  display_oled(false, bx, by3, Line3);  
  bprevx = bx;
}

void bdisplay_loop() {
  if(!sensorvalues){
    display_move(bprevx, by2, bx, by2, Line2);
    if (!oncecompleted){
      display_move(bprevx, by3, bx, by3, Line3);
    }

    bprevx = bx;
    bx = bx - 3;
    if (bx < bminX) bx = display.width();
    if (bx < 12) oncecompleted = true;
    //if (--bx < bminX) bx = display.width(); // this line is moving 1 pixel at the time, this can be too slow
  }
}

void bdisplay_textline(String textline){
  display.setTextWrap(true);
  display_oled(true, 30, 16, textline);  
  display.setTextWrap(false);
}

void bdisplay_readings(float temp1, float temp2, int humid, int pressure){
  sensorvalues = true;
  display_oled(true, 0, 16, String(temp1, 1) + " \x7F"+"C");  // \x7F is converted to degrees in this special font.
  //display_oled(false, 75, 16, String(temp2, 1) + " \x7F"+"C");  
  display_oled(false, 0, 38, String(humid) + "% rH");  
  display_oled(false, 0, 60, String(pressure) + " hPa");  
}

String bdisplay_readingtime(float temp1, int hours, int minutes, int secondes){
  display_oled(true, 0, 16, String(temp1, 1) + " \x7F"+"C");  // \x7F is converted to degrees in this special font.
  //display.setTextWrap(true);
  char buffer[8] = "";
  sprintf(buffer, "%02d:%02d:%02d", hours, minutes, secondes);
  //dateTime = buffer;
  //String timestampnow = sprintf("%02d:%02d:%02d", hours, minutes, secondes);
  //display_oled(false, 0, 38, timestampnow);  
  String timeformat = String(buffer);
  display_oled(false, 0, 49, timeformat);  
  //display_oled(false, 0, 38, String(hours) + ":" + String(minutes) + ":" + String(secondes));  
  //display_oled(false, 100, 38, minutes);  
  return timeformat;
}
