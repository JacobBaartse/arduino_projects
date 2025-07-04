#include <EEPROM.h>

void setup() {

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {
  byte a_byte = 0;
  // put your main code here, to run repeatedly:
  delay(1000);
  Serial.println("start reading eeprom..");
  for (int address=0; address<100; address++)  
  {
    a_byte = EEPROM.read(address);
    Serial.print(address);
    Serial.print(":");
    Serial.print(a_byte, HEX);
    Serial.print(":");
    Serial.println((char)a_byte);
  }
  Serial.println("done");

  delay(100000);

}
