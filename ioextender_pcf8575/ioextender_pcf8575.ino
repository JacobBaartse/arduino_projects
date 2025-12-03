#include "PCF8575.h"


#define DEBUG 1
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif


//  adjust addresses if needed
PCF8575 PCF_23(0x23);
int pin2 = 2;  // connect to int pin of pcf8575
int pin2_val = 0;
int bit_zero = 0x0001;
uint16_t xor_value = 0b1111111111111111;
void setup()
{
  Serial.begin(115200);
  debug("PCF8575_LIB_VERSION:\t");
  debugln(PCF8575_LIB_VERSION);
  pinMode(pin2, INPUT_PULLUP);
  Wire.begin();
  PCF_23.begin();
}



void loop()
{
  pin2_val = digitalRead(pin2);
  if (pin2_val == 0)
  {
    uint16_t value = PCF_23.read16();
    value ^= xor_value;  // 1 for normaly open,  0 for normaly closed.
    debugln(value);
  }
  delay(100);
}

#undef debug
#undef debugln