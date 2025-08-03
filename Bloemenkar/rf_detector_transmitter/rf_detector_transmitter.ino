/*
 * RF-Nano, headers, USB-C with joystick connected, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#define radioChannel 98 // dit wordt mogelijk instelbaar

#define CFG_PIN0 A0
#define CFG_PIN1 A1
#define CFG_PIN2 A2
#define CFG_PIN3 A3
// #define CFG_PIN4 6
// #define CFG_PIN5 7
// #define CFG_PIN6 8
// #define CFG_PIN7 9

#define PIR_PIN 4


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

uint16_t detectornode = 01; // Address of this node in Octal format (04, 031, etc.)
const uint16_t basenode = 00; // Address of the home/host/controller node in Octal format

unsigned long const keywordvalD = 0xdeedbeeb; 

struct detector_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t dvalue;
  uint8_t sw1value;
  uint8_t sw2value;
};

bool newdata = false;

void setup() {
  Serial.begin(115200);

  // multiple PINs for reading the config
  pinMode(CFG_PIN0, INPUT_PULLUP);
  pinMode(CFG_PIN1, INPUT_PULLUP);
  pinMode(CFG_PIN2, INPUT_PULLUP);
  pinMode(CFG_PIN3, INPUT_PULLUP);
  // pinMode(CFG_PIN4, INPUT_PULLUP);
  // pinMode(CFG_PIN5, INPUT_PULLUP);
  // pinMode(CFG_PIN6, INPUT_PULLUP);
  // pinMode(CFG_PIN7, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);

  if (digitalRead(CFG_PIN0) == LOW){ // PIN active
    detectornode++;
  }
  if (digitalRead(CFG_PIN1) == LOW){ // PIN active
    detectornode = detectornode + 2;
  }
  if (digitalRead(CFG_PIN2) == LOW){ // PIN active

  }
  if (digitalRead(CFG_PIN3) == LOW){ // PIN active

  }

  // if (digitalRead(CFG_PIN4) == LOW){ // PIN active

  // }
  // if (digitalRead(CFG_PIN5) == LOW){ // PIN active

  // }
  // if (digitalRead(CFG_PIN6) == LOW){ // PIN active

  // }
  // if (digitalRead(CFG_PIN7) == LOW){ // PIN active

  // }

  // detectornode = depending on settings
  // radioChannel = depending on settings
  // PA level can be depending on settings

  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.print(F("Detectornode: "));
  Serial.println(detectornode);
  Serial.flush(); 

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, detectornode);
}

unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;
uint8_t detectionValue = 0;

bool trackPIR(bool pirStatus, unsigned long currentmillis){
  static unsigned long startPIRtime = 0;
  static bool PIRactive = false;
  bool info = false;

  if (PIRactive){
    if ((unsigned long)(currentmillis - startPIRtime) > 5000){
      PIRactive = false;
      detectionValue = 0;
    }
  }
  else {
    if (pirStatus){
      startPIRtime = currentmillis;
      info = true;
      detectionValue = 0x0f;
    }
    if (detectionValue < 0xff)
      detectionValue++;
  }
  return info;
}

//===== Receiving =====//
void receiveRFnetwork(unsigned long currentRFmilli){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    detector_payload Rxdata;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node != basenode) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    if (Rxdata.keyword == keywordvalD){
      Serial.println(F("new data received"));

    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
}

//===== Sending =====//
bool transmitRFnetwork(bool fresh, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 15 seconds, or on new data
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 15000)){
    sendingTimer = currentRFmilli;

    detector_payload Txdata;
    Txdata.keyword = keywordvalD;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    Txdata.dvalue = detectionValue;
    // Txdata.sw1value = sw1Value;
    // Txdata.sw2value = sw2Value;

    Serial.print(F("Message: "));
    Serial.print(F(" dvalue: "));
    Serial.print(Txdata.dvalue);
    // Serial.print(F(", sw1value: "));
    // Serial.print(Txdata.sw1value);        
    // Serial.print(F(", sw2value: "));
    // Serial.println(Txdata.sw2value);

    RF24NetworkHeader header0(basenode, 'D'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    Serial.print(F("Message send ")); 
    if (w_ok){
      // bValue = 0; 
      // sw1Value = 0;
      // sw2Value = 0;
      // fresh = false;
      failcount = 0;
    }    
    else{
      Serial.print(F("failed "));
      failcount++;
    }
    Serial.print(Txdata.count);
    Serial.print(F(", "));
    Serial.println(currentRFmilli);

    if (failcount > 10){
      fresh = false; // do not send a lot of messages continously
    }

    if(!fresh){ // clear buttons status always after 5 seconds
      // bValue = 0; 
      // sw1Value = 0;
      // sw2Value = 0;
    }
  }

  return fresh;
}

bool activePIR = false;

void loop() {

  network.update();

  currentmilli = millis();

  receiveRFnetwork(currentmilli);

  //************************ sensors ****************//

  activePIR = (digitalRead(PIR_PIN) == LOW);

  //************************ sensors ****************//
  newdata = trackPIR(activePIR, currentmilli);

  newdata = transmitRFnetwork(newdata, currentmilli);

}
