/*
 * RF-Nano, no headers, USB-C with keypad connected, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>
#include <Keypad.h>

#define radioChannel 106

const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {5, 4, 3, 2}; 
byte colPins[COLS] = {A0, 8, 7, 6}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node01 = 02; // Address of this node in Octal format (04, 031, etc.)
const uint16_t node00 = 00; // Address of the home/host/controller node in Octal format

unsigned long const keywordvalM = 0xfeedbeef; 
unsigned long const keywordvalK = 0xbeeffeed; 

struct keypad_payload{
  uint32_t keyword;
  uint32_t timing;
  uint32_t count;
  char keys[11];
};

struct network_payload {
  unsigned long keyword;
  unsigned long counter;
  unsigned long timing;
  unsigned long command;
  unsigned long response;
  unsigned long data1;
  unsigned long data2;
  unsigned long data3;
};

bool newdata = false;
const uint8_t maxkeys = 10;
char keytracking[11]; // 10 characters + room for the null terminator
uint8_t keyindex = 0;
const uint16_t maxtime = 4000; // maximum time to type a multidigit number

void clearkeypadcache(){
  for (int i=0;i<=maxkeys;i++){
    keytracking[i] = 0; // place null character
  }
  keyindex = 0;
  newdata = false;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, node01);

  clearkeypadcache();
}
 
unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;
unsigned long keyingtime = 0;

//===== Receiving =====//
void receiveRFnetwork(unsigned long currentRFmilli){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    network_payload Rxdata;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node != node00) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    if (Rxdata.keyword == keywordvalM){
      Serial.println(F("new data received"));


    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
}

//===== Sending =====//
void transmitRFnetwork(bool fresh, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static unsigned long counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 10 seconds, or on new data
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 10000)){
    sendingTimer = currentRFmilli;

    keypad_payload Txdata;
    Txdata.keyword = keywordvalK;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    for (int i=0;i<=maxkeys;i++){
      Txdata.keys[i] = keytracking[i];
    }  
    
    RF24NetworkHeader header0(node00, 'K'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    Serial.print(F("Message send ")); 
    if (w_ok){
      clearkeypadcache();
      failcount = 0;
    }    
    else{
      Serial.print(F("failed "));
      failcount++;
    }
    Serial.println(currentRFmilli);
  }

  if (failcount > 10){
    failcount = 0;
    clearkeypadcache();
  }
}

void loop() {

  network.update();

  currentmilli = millis();

  receiveRFnetwork(currentmilli);

  //************************ sensors ****************//

  char customKey = customKeypad.getKey();
  if (customKey){
    if (keyindex == 0){ // start timing
      keyingtime = currentmilli + maxtime;
    }
    Serial.println(customKey);
    if (keyindex < maxkeys){
      keytracking[keyindex++] = customKey;
    }
  }

  if (keyindex > 0)
    newdata = currentmilli > keyingtime;
  else 
    newdata = keyindex > (maxkeys - 3);

  //************************ sensors ****************//

  transmitRFnetwork(newdata, currentmilli);

}
