
// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#define TFT_RST 8
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_MISO 12

#define DEBUG 0
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif


// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
// If using the breakout, change pins as desired
// Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

TS_Point p1;
int16_t screen_width;
int16_t screen_height;

void onScreen_keyboard_setup() {
  debugln("ILI9341 Test!"); 

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

  screen_width = tft.width();
  screen_height = tft.height();
  debug("screen width ");
  debugln(screen_width);
  debug("screen height ");
  debugln(screen_height);
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
  tft.setCursor(0,50);
  tft.write((prev_input + String("_")).c_str());  
  
  tft.setTextColor(ILI9341_WHITE);    
  tft.setCursor(0,50);
  tft.write((input + String("_")).c_str());
  prev_input = input;
}

void get_character(int8_t row, uint16_t x, bool shift){
  debug("row");
  debug(row);
  debug(" x ");
  debugln(x);
  if (x > keyboard[row].tuch_start_x){
    x -= keyboard[row].tuch_start_x;
    x /=226;
    debug("position in row: ");
    debugln(x);
    if (x < keyboard[row].num_chars){
      debug("Char: ");
      if (shift) input += keyboard[row].shift_keys[x];
      else input += keyboard[row].keys[x];
    }
    update_input();
  }

}

String onScreenKeyboard_get_string(bool shift, String value, String label) {
  tft.fillScreen(ILI9341_BLACK);
  input = value;
  prev_input = "";
  tft.setTextColor(ILI9341_WHITE);  
  tft.setCursor(0,20);
  tft.write(label.c_str());
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
    tft.print("Aa <-- spa ok");

    update_input();

    // check key pressed.
    bool dowhile = true;
    while (dowhile){
      p1 = get_touch();
      delay(200);
      debug("x = ");
      debug(p1.x);
      debug(", y = ");
      debug(p1.y);
      debugln();
      delay(200);
      if (p1.y > 3250){
        if (p1.x<1057){
          debugln("shift");
          shift = !shift;
          dowhile = false;
        } 
        else if (p1.x<1974){
          debugln("del");
          input = input.substring(0,input.length()-1);
          update_input();
        } 
        else if (p1.x<2856){
          debugln("space");
          input += " ";
          update_input();
        } 
        else{
          debugln("ok");
          tft.fillScreen(ILI9341_BLACK);
          return input;
        }
      } 
      else if (p1.y > 2890) get_character(3, p1.x, shift);
      else if (p1.y > 2450) get_character(2, p1.x, shift);
      else if (p1.y > 2030) get_character(1, p1.x, shift);
      else if (p1.y > 1780) get_character(0, p1.x, shift);
      else debugln("above");
    }
  }
}

String prev_text = "";
void show_text_on_screen(String text){
  tft.setTextColor(ILI9341_BLACK);  
  tft.setCursor(0,20);
  tft.write(prev_text.c_str());  

  tft.setTextColor(ILI9341_WHITE);  
  tft.setCursor(0,20);
  tft.write(text.c_str());  
  prev_text = text;
}

void show_text_on_screen_time(String text, long duration){
  show_text_on_screen(text);
  delay(duration);  
}


int show_menu_on_screen(String menu_text){
  show_text_on_screen(menu_text);  
  delay(300);
  p1 = get_touch();
  debug("x = ");
  debug(p1.x);
  debug(", y = ");
  debug(p1.y);
  debugln();
  return (p1.y-494)/450;
}


#undef debug
#undef debugln