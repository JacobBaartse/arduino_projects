/*
 *  https://www.circuitbasics.com/how-to-set-up-a-keypad-on-an-arduino/
 *
 *  connect to RF-NANO and send button data over RF to UNO controller
 */

#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

void setup(){
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  delay(1000);
  Serial.println(F("Starting 16 button keypad controller"));
  Serial.println();
  Serial.print(__FILE__);
  Serial.print(F("\n, creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

}
  
void loop(){
  char customKey = customKeypad.getKey();
  
  if (customKey){
    Serial.println(customKey);
  }
}