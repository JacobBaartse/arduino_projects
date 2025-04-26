#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

byte* ReadBytesEEPROM(int ee, const int DataSize){
  static byte returnArray[20];
  for(int i=0; i < DataSize; i++) 
    returnArray[i] = EEPROM.read(ee + i);
  return returnArray;
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
