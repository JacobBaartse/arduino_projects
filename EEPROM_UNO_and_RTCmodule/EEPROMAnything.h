#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

// template <class T> int EEPROM_writeAnything(int ee, const T& value)
// {
//     const byte* p = (const byte*)(const void*)&value;
//     unsigned int i;
//     for (i = 0; i < sizeof(value); i++)
//       EEPROM.write(ee++, *p++);
//     //return i;
//     return ee;
// }

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++){
      if (EEPROM.read(ee) != *p) EEPROM.write(ee, *p);
      ee++, *p++;
    }
    //return i;
    return ee;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
      *p++ = EEPROM.read(ee++);
    //return i;
    return ee;
}
