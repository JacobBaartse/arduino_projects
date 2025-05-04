#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

byte* ReadBytesEEPROM(int ee, const int DataSize){
  static byte returnArray[20];
  for(int i=0; i < DataSize; i++) 
    returnArray[i] = EEPROM.read(ee + i);
  return returnArray;
}

int WriteBytesEEPROM(int ee, const byte* DataValue, const int DataSize){
  byte checkval;
  int i = 0;
  for(; i < DataSize; i++){
    checkval = EEPROM.read(ee + i);
    if (checkval != DataValue[i]){ // only wrrite when changing EEPROM contents
      EEPROM.write(ee + i, DataValue[i]);
    }
  }
  return ee + i;
}

void printchararray(const byte* chars){
  int i = 0;
  char sepstring[] = "  ";
  while (chars[i]){
    Serial.print(sepstring);
    Serial.print("0x");
    Serial.print(chars[i++], HEX);
    sepstring[0] = ',';
  }
  Serial.println();
}
