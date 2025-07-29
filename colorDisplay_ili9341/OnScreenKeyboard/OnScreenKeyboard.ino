/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include "SPI.h"
#include "Adafruit_GFX.h"
#include <Fonts/FreeMono18pt7b.h>
#include "Adafruit_ILI9341.h"
#include "touch.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 6 // 10
#define TFT_RST 5 // 8
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_MISO 12


// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
// If using the breakout, change pins as desired
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

TS_Point p1;
int16_t screen_width;
int16_t screen_height;

void setup() {
  Serial.begin(115200);
  Serial.println("ILI9341 Test!"); 

  ts_setup();
 
  tft.begin();
  tft.setFont(&FreeMono18pt7b);
  tft.setRotation(1);

  // read diagnostics (optional but can help debug problems)
  // uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  // Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  // x = tft.readcommand8(ILI9341_RDMADCTL);
  // Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  // x = tft.readcommand8(ILI9341_RDPIXFMT);
  // Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  // x = tft.readcommand8(ILI9341_RDIMGFMT);
  // Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  // x = tft.readcommand8(ILI9341_RDSELFDIAG);
  // Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 

//Cross for touch calibration
  // tft.drawLine(x1, y1, x2, y2, color);
  tft.fillScreen(ILI9341_BLACK);
  // tft.drawLine(0, 0, 10, 10, ILI9341_WHITE);
  // tft.drawLine(0, 10, 10, 0, ILI9341_WHITE);

  // p1 = get_touch();
  // tft.fillScreen(ILI9341_BLACK);
  // Serial.print("Left top x = ");
  // Serial.print(p1.x);
  // Serial.print(", y = ");
  // Serial.print(p1.y);
  // Serial.println();
  // delay(200);

  // int16_t t_x1=p1.x;
  // int16_t t_y1=p1.y;

  screen_width = tft.width();
  screen_height = tft.height();
  Serial.print("screen width ");
  Serial.println(screen_width);
  Serial.print("screen height ");
  Serial.println(screen_height);

  // tft.drawLine(w-1, h-1, w-11, h-11, ILI9341_WHITE);
  // tft.drawLine(w-1, h-11, w-11, h-1, ILI9341_WHITE);
  // p1 = get_touch();
  // tft.fillScreen(ILI9341_BLACK);
  // Serial.print("Right bottom x = ");
  // Serial.print(p1.x);
  // Serial.print(", y = ");
  // Serial.print(p1.y);
  // Serial.println();
  // delay(200);
  // int16_t t_x2=p1.x;
  // int16_t t_y2=p1.y;
}




struct keyboard_row{
  char keys[14];
  char shift_keys[14];
  int16_t x;
  int16_t y;
  int8_t num_chars;
  int16_t tuch_start_x;
};

const int v_offset = 115;
const int h_offset = 20;
const int num_rows = 4;
const keyboard_row keyboard[num_rows] PROGMEM ={
  {"`1234567890-=", "~!@#$%^&*()_+",  0,  0, 13, 550},
  {"qwertyuiop[]" , "QWERTYUIOP{}" , 20, 28, 12, 670},
  {"asdfghjkl;'"  , "ASDFGHJKL:\"" , 27, 56, 11, 744},
  {"\\zxcvbnm,./" , "|ZXCVBNM<>?"  , 17, 84, 11, 604}
};


String input = "";
String prev_input = "";

void update_input(){
  tft.setTextColor(ILI9341_BLACK);  
  tft.setCursor(0,20);
  tft.write((prev_input + String("_")).c_str());  
  
  tft.setTextColor(ILI9341_WHITE);    
  tft.setCursor(0,20);
  tft.write((input + String("_")).c_str());
  prev_input = input;
}

void get_character(int8_t row, uint16_t x, bool shift){
  Serial.print("row");
  Serial.print(row);
  Serial.print(" x ");
  Serial.println(x);
  if (x > keyboard[row].tuch_start_x){
    x -= keyboard[row].tuch_start_x;
    x /=226;
    Serial.print("position in row: ");
    Serial.println(x);
    if (x < keyboard[row].num_chars){
      Serial.print("Char: ");
      if (shift) input += keyboard[row].shift_keys[x];
      else input += keyboard[row].keys[x];
    }
    update_input();
  }

}

void onScreenKeyboard(bool shift) {
  tft.setTextColor(ILI9341_WHITE);  

  while (true){

    const int font_height = 22;
    tft.fillRect(0, v_offset-font_height, screen_width, screen_height-v_offset + font_height, ILI9341_BLACK);  // x, y, w, h, color

    //print onscreen keyboard.
    for (int i=0; i<num_rows; i++){
      tft.setCursor(h_offset+keyboard[i].x, v_offset+keyboard[i].y);
      if (shift) tft.print(keyboard[i].shift_keys);
      else tft.print(keyboard[i].keys);
    }
    tft.setCursor(h_offset + 4, v_offset + 112);
    tft.print("Aa <-- spa ent");

    update_input();

    // check key pressed.
    bool dowhile = true;
    while (dowhile){
      p1 = get_touch();
      Serial.print("x = ");
      Serial.print(p1.x);
      Serial.print(", y = ");
      Serial.print(p1.y);
      Serial.println();
      delay(200);
      if (p1.y > 3250){
        if (p1.x<881){
          Serial.println("shift");
          shift = !shift;
          dowhile = false;
        } 
        else if (p1.x<1730){
          Serial.println("del");
          input = input.substring(0,input.length()-1);
          update_input();
        } 
        else if (p1.x<2700){
          Serial.println("space");
          input += " ";
          update_input();
        } 
        else{
          Serial.println("enter");
        }
      } 
      else if (p1.y > 2890) get_character(3, p1.x, shift);
      else if (p1.y > 2450) get_character(2, p1.x, shift);
      else if (p1.y > 2030) get_character(1, p1.x, shift);
      else if (p1.y > 1780) get_character(0, p1.x, shift);
      else Serial.println("above");
    }
  }
}


void loop(void) {

    onScreenKeyboard(false);
    delay(100);
}
