/*
  EEPROM check UNO board and RTC module board
  
  https://docs.arduino.cc/learn/programming/eeprom-guide/ on board EEPROM
  https://www.vdrelectronics.com/ds3231-precisie-rtc-module-gebruiken-met-arduino EEPROM on RTC module

  See also:
  https://docs.arduino.cc/tutorials/uno-r4-wifi/eeprom
 */

// Include the EEPROM library
#include <EEPROM.h>

#include <Wire.h>
#include <DS3231.h>

#define RTC_EEPROM_ADDRESS 0x57 // EEPROM address on the RTC module


struct MyObject {
  float field1;
  byte field2;
  char name[50];
};

RTClib myRTC;

void setup () {
  Serial.begin(115200);
  Wire.begin();
  delay(500);
  Serial.println("Ready!");
  Serial.flush();

  byte readvalue = EEPROM.read(0);
  if (readvalue == 0){ // write only once
    MyObject customVar = {
      3.14f,
      65,
      "UNO EEPROM content!\0"
    };
    EEPROM.put(1, customVar);
    EEPROM.write(0, 0xfc); // indicate EEPROM is written
    Serial.println("Written data to UNO EEPROM");
  }
  else {
    Serial.println("UNO EEPROM contains already data");
  }

  byte readByte = i2c_eeprom_read_byte(RTC_EEPROM_ADDRESS, 0);
  if (readByte == 0){ // write only once
    char data[] = "Hallo Wereld!";
    // Write the data to the first byte (0) in the EEPROM.
    i2c_eeprom_write_page(RTC_EEPROM_ADDRESS, 0, (byte *)data, sizeof(data));
    Serial.println("Written data to RTC module EEPROM");
  }
  else {
    Serial.println("RTC module EEPROM already contains data");
  }
}

void loop() {

  delay(1000);
  
  DateTime now = myRTC.now();

  int internalEEPROM = EEPROM.length();
  int externalEEPROM = 4096; // bytes

  Serial.print(F("\nUNO EEPROM length: "));
  Serial.println(internalEEPROM);  

  MyObject customVar2; // Variable to store custom object read from EEPROM.
  EEPROM.get(1, customVar2);
  Serial.println(customVar2.field1);
  Serial.println(customVar2.field2);
  Serial.println(customVar2.name);

  Serial.print(F("\nRTC module EEPROM length: "));
  Serial.println(externalEEPROM);

  Serial.print("\nReading memory: ");
  int addressByte = 0; // which byte from the EEPROM to access

  // Read the first byte in the EEPROM
  byte readByte = i2c_eeprom_read_byte(RTC_EEPROM_ADDRESS, addressByte);
  // Keep reading the EEPROM until an empty cell (value = 0) is detected
  while (readByte != 0)
  {
      Serial.print((char)readByte ); // Print the content of this cell
      addressByte++; // increase addressByte by 1
      readByte = i2c_eeprom_read_byte(RTC_EEPROM_ADDRESS, addressByte); // Read 1 byte from the memory
  }
  Serial.println("\n");

  while (1 != 2){
    delay(1000);
  }
}

// Write a single byte to the EEPROM
void i2c_eeprom_write_byte(int deviceAddress, unsigned int eeAddress, byte data){
  int writeData = data;
  Wire.beginTransmission(deviceAddress);
  Wire.write((int)(eeAddress >> 8)); // MSB
  Wire.write((int)(eeAddress & 0xFF)); // LSB
  Wire.write(writeData);
  Wire.endTransmission();
}

// Write a string to the EEPROM
void i2c_eeprom_write_page(int deviceAddress, unsigned int eeAddressPage, byte* data, byte length){
  Wire.beginTransmission(deviceAddress);
  Wire.write((int)(eeAddressPage >> 8)); // MSB
  Wire.write((int)(eeAddressPage & 0xFF)); // LSB
  for (byte i = 0; i < length; i++)
    Wire.write(data[i]);
  Wire.endTransmission();
}

// Read a single byte from the EEPROM
byte i2c_eeprom_read_byte(int deviceAddress, unsigned int eeAddress){
  byte readData = 0x00;
  Wire.beginTransmission(deviceAddress);
  Wire.write((int)(eeAddress >> 8)); // MSB
  Wire.write((int)(eeAddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress,1);
  if (Wire.available()) readData = Wire.read();
  return readData;
}
