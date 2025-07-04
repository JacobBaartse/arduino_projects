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


//  adjust addresses if needed
PCF8575 PCF_20(0x20);  //  add switches to lines  (used as input)
int pin7 = 7;  // connect to int pin of pcf8575
int pin7_val = 0;
int bit_zero = 0x0001;

uint16_t xor_value = 0b1111111111110010;
void setup_io_extender()
{
  debug("PCF8575_LIB_VERSION:\t");
  debugln(PCF8575_LIB_VERSION);
  pinMode(pin7, INPUT_PULLUP);
  Wire.begin();
  PCF_20.begin();
}

uint8_t io_extender_check_switches()
{
  pin7_val = digitalRead(pin7);
  uint8_t pos = 0;
  if (pin7_val == 0)
  {
    uint16_t value = PCF_20.read16();
    value ^= xor_value;  // 1 for normaly open,  0 for normaly closed.
    debugln(value);

    while (value > 0)
    {
      pos +=1;
      value  = value >> 1;
    }
    debug("button pushed: ");
    debugln(pos);
//    delay(5);
  }
  return pos;
}

#undef debug
#undef debugln