/*
 *  https://www.circuitbasics.com/how-to-set-up-a-keypad-on-an-arduino/
 *
 *
 * Install the Keypad library by Mark Stanley and Alexander Brevig
 *
 * Idea: - connect to RF-NANO, battery powered and send button data over RF to UNO controller
 *       - feedback LED that the information is received (ACK)
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

// byte rowPins[ROWS] = {9, 8, 7, 6}; 
// byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 


const uint8_t maxkeys = 10;
char keytracking[11]; // 10 characters + room for the null terminator
const uint16_t maxtime = 3000; // maximum time to type a multidigit number
unsigned long keyingtime = 0;
unsigned long runningtime = 0;
uint8_t keyindex = 0;
bool transmit = false;

void clearkeypadcache(){
  for (int i=0;i<=maxkeys;i++){
    keytracking[i] = 0; // place null character
  }
  keyindex = 0;
}

void setup(){
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  delay(1000);
  Serial.println(F("Starting 16 button keypad experiment"));
  Serial.println();
  Serial.print(__FILE__);
  Serial.print(F("\n, creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  clearkeypadcache();
}

void loop(){

  runningtime = millis();

  char customKey = customKeypad.getKey();
  if (customKey){
    if (keyindex == 0){ // start timing
      keyingtime = runningtime + maxtime;
    }
    Serial.println(customKey);
    if (keyindex < maxkeys){
      keytracking[keyindex++] = customKey;
    }
  }

  if (keyindex > 0){
    transmit = runningtime > keyingtime;
  }

  if (transmit){
    if (keyindex > 0){
      Serial.print(F("Data collected: '"));
      Serial.print(keytracking);
      Serial.println(F("'"));
      clearkeypadcache();
    }
    transmit = false;
  }
}
