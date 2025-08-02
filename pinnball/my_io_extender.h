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
int pin4 = 4;  // connect to int pin of pcf8575
int pin4_val = 0;
int bit_zero = 0x0001;

uint16_t xor_value = 0b1111111111110010;
void setup_io_extender()
{
  debug("PCF8575_LIB_VERSION:\t");
  debugln(PCF8575_LIB_VERSION);
  pinMode(pin4, INPUT_PULLUP);
  Wire.begin();
  PCF_20.begin();
}

uint8_t io_extender_check_switches()
{
  pin4_val = digitalRead(pin4);
  uint8_t pos = 0;
  if (pin4_val == 0)
  {
    uint16_t value = PCF_20.read16();
    value ^= xor_value;  // 1 for normaly open,  0 for normaly closed.
    debugln(value);
    
    //key combination for entering menu red and green button together.
    if (value == 0b0000001100000000) return 17;

    while (value > 0)
    {
      pos +=1;
      value  = value >> 1;
    }
    debug("button pushed: ");
    debugln(pos);
    if (pos == 1) xor_value ^= 0b0000000000000001; // prevent spinner to give points to next player keep track of last value of spinner
  }
  return pos;
}

#undef debug
#undef debugln