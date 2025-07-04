//
//    FILE: pcf8575_test.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo
//     URL: https://github.com/RobTillaart/PCF8575


#include "PCF8575.h"

//  adjust addresses if needed
PCF8575 PCF_20(0x20);  //  add switches to lines  (used as input)
int pin7 = 7;
int pin7_val = 0;


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("PCF8575_LIB_VERSION:\t");
  Serial.println(PCF8575_LIB_VERSION);

  pinMode(pin7, INPUT_PULLUP);
  Wire.begin();

  PCF_20.begin();


  uint16_t value = PCF_20.read16();
  Serial.print("#38:\t");
  Serial.println(value);

}


void loop()
{
  pin7_val = digitalRead(pin7);

  if (pin7_val == 0)
  {
    uint16_t value = PCF_20.read16();
    Serial.print("#38:\t");
    Serial.println(value, HEX);
    delay(5);
  }

}


