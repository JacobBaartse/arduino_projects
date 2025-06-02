#include <SoftwareSerial.h>
#include "ps2_keyboard.h"

#include <Adafruit_SH110X.h>  //Adafruit SH110X by Adafruit
#include <Adafruit_GFX.h>
#include "font_16pix_high.h"  //https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include "sh1106_display.h"



void setup()  {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  setup_ps2_keyboard();
  setup_oled_display();
}

String some_text = "";

void loop() {
  char keypress = get_keyboard_char();
  if (keypress > 0){
    // Serial.print(keypress);
    if (keypress == 0x08) some_text.remove(some_text.length()-1);
    else some_text += keypress;
    display_oled(true, 0,16, some_text, true);
  } 
  delay(5);

}