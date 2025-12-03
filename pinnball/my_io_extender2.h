#include "api/Common.h"
//
//    FILE: pcf8575_test.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
//     URL: https://github.com/RobTillaart/PCF8575

#define DEBUG 0
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// #define ROTARY_1 1
// #define HIGH_RAMP_SWITCH 2
// #define TUBE_END_SWITCH 3
// #define BALL_OUT_SWITCH 4
// #define RIGHT_MIDDLE_SWITCH_CLOVER_NINE 5
// #define RIGHT_TOP_SWITCH_CLOVER_EIGHT 6
// #define RIGHT_BOTTOM_SWITCH_JOKER 7
// #define RIGHT_MIDDLE_SWITCH_CLOVER_TEN 8
// #define GREEN_PUSH_BUTTON 9
// #define RED_PUSH_BUTTON 10
// #define LEFT_EDGE_TOP_BUTTON 11
// #define LEFT_EDGE_MIDDLE_BUTTON 12
// #define LEFT_EDGE_BOTTOM_BUTTON 13
// #define TOP_BALL_FUNNEL 14
// #define LEFT_TOP_SWITCH 15
// #define ROTARY_2 16

#define NUM_SWITCHES2 16


//  adjust addresses if needed
PCF8575 PCF_23(0x23);  //  add switches to lines  (used as input)
int pin2 = 2;  // connect to int pin of pcf8575
int pin2_val = 0;

uint16_t xor_value2 = 0b1111111111111111;
void setup_io_extender2()
{
  debug("PCF8575_LIB_VERSION:\t");
  debugln(PCF8575_LIB_VERSION);
  pinMode(pin2, INPUT_PULLUP);
  // Wire.begin();
  PCF_23.begin();
}

unsigned long contact_timing2[NUM_SWITCHES2 + 1];


uint8_t io_extender_check_switches2()
{
  pin2_val = digitalRead(pin2);
  uint8_t pos = 0;
  if (pin2_val == 0)
  {

    uint16_t value = PCF_23.read16();
    value ^= xor_value2;  // 1 for normaly open,  0 for normaly closed.
    debugln(value);
    

    while (value > 0)
    {
      pos +=1;
      value  = value >> 1;
    }
    debug("button pushed: ");
    debugln(pos);

    unsigned long cur_millis = millis();
    if (true){ //pos != ROTARY_1 && pos != ROTARY_2  && pos !=GREEN_PUSH_BUTTON && pos !=RED_PUSH_BUTTON){
      if ((cur_millis - contact_timing2[pos])< 1000){
        // contact bounce detected.
        contact_timing2[pos] = cur_millis;
        return 0;
      }
    }
    contact_timing2[pos] = cur_millis;

    

    // if (pos == ROTARY_1) xor_value2 ^= 0b0000000000000001; // toggle so next change will trigger again.
    // if (pos == ROTARY_2) xor_value2 ^= 0b1000000000000000; // toggle so next change will trigger again.

  }
  return pos;
}

#undef debug
#undef debugln