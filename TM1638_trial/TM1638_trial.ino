/*

  https://www.makerguides.com/connecting-arduino-uno-with-tm1638-7-segment-led-driver/

*/

#include <TM1638.h>

//choose digital pins compatibles with your board
#define STB 4 // Strobe digital pin
#define CLK 5 // clock digital pin
#define DIO 6 // data digital pin

TM1638 tm(CLK, DIO, STB);

void setup() {
  Serial.begin(115200);

  tm.reset();
  tm.test();
  
  Serial.print(__FILE__);
  Serial.print(F("\n, creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush();    
}

pulse_t pulse = PULSE1_16;
const uint8_t text[] = {0x7c, 0x1c, 0x78, 0x78, 0x5c, 0x54};
unsigned long timer = 0;

void loop() {

  for (uint8_t i=0;i<sizeof(text);i++) {
    tm.displayDig(7-i, text[i]);
  }

  uint8_t buttons = tm.getButtons();
  tm.writeLeds(buttons);

  if (millis() - timer > 1000){
    timer = millis();
    Serial.print(F('Pulse: '));
    Serial.println(pulse);
    tm.displaySetBrightness(pulse);
    pulse = (pulse==PULSE1_16) ? PULSE14_16 : PULSE1_16;
  }
  
}
