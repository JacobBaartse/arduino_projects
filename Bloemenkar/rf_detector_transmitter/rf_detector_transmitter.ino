/*
 * RF-Nano, headers, USB-C, using RF24network library
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

#define BUTTON_PIN 2

#define PIR_PIN 4


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

uint16_t detectornode = 01; // Address of this node in Octal format (04, 031, etc.)
const uint16_t basenode = 00; // Address of the home/host/controller node in Octal format
uint8_t radiolevel = RF24_PA_MIN;

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

  // PINs for sensor inputs
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);

  if (digitalRead(CFG_PIN0) == LOW){ // PIN active
    detectornode = 2;
  }
  if (digitalRead(CFG_PIN1) == LOW){ // PIN active
    detectornode = detectornode + 2;
  }
  if (digitalRead(CFG_PIN2) == LOW){ // PIN active
    radiolevel = 1;
  }
  if (digitalRead(CFG_PIN3) == LOW){ // PIN active
    radiolevel = radiolevel + 2;
  }

  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  // detectornode = depending on settings
  Serial.print(F("Detectornode: "));
  Serial.println(detectornode);
  Serial.flush(); 

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  // RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 
  //radio.setPALevel(RF24_PA_MIN, 0);
  radio.setPALevel(radiolevel, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, detectornode);
  Serial.print(F("radioChannel: "));
  Serial.print(radioChannel);
  Serial.print(F(", level: "));
  Serial.println(radiolevel);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPress, FALLING); // trigger when button is pressed
}

unsigned long currentmilli = 0;
uint8_t detectionValue = 0;
uint8_t sw1Value = 0;
uint8_t sw2Value = 0;
bool activePIR = false;
bool activeBUTTON = false;
bool pressBUTTON = false;


void trackDetectionAndButton(unsigned long currentDetectMillis){
  static unsigned long activationTime = 0;
  static bool alarming = false;

  if (alarming){
    if ((unsigned long)(currentDetectMillis - activationTime) > 60000){ // 60 seconds no new detection
      activePIR = false;
      activeBUTTON = false;
      detectionValue = 0;
      sw1Value = 0;
      sw2Value = 0;
      Serial.print(F("Reset detections "));
      Serial.println(currentDetectMillis);
      alarming = false;
    }
  }
  else {
    if (activePIR){
      sw1Value = 0x5a;
      Serial.print(F("PIR detection "));
      Serial.println(currentDetectMillis);
      alarming = true;
    }
    if (activeBUTTON){
      sw2Value = 0xa5;
      activationTime = currentDetectMillis;
      Serial.print(F("BUTTON detection "));
      Serial.println(currentDetectMillis);
      alarming = true;
    }
    if (alarming){
      activationTime = currentDetectMillis;
      detectionValue = 0xff;
    }
  }
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

      // in case a message is received, with specific data, the detector could be 'reset'


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

  // Every 10 seconds, or on new data
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 10000)){
    sendingTimer = currentRFmilli;

    detector_payload Txdata;
    Txdata.keyword = keywordvalD;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    Txdata.dvalue = detectionValue;
    Txdata.sw1value = sw1Value;
    Txdata.sw2value = sw2Value;

    Serial.print(F("Message: "));
    Serial.print(F(" dvalue: "));
    Serial.print(Txdata.dvalue);
    Serial.print(F(", sw1value (PIR): "));
    Serial.print(Txdata.sw1value);        
    Serial.print(F(", sw2value (BUTTON): "));
    Serial.println(Txdata.sw2value);

    RF24NetworkHeader header0(basenode, 'D'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    Serial.print(F("Message send ")); 
    if (w_ok){
      fresh = false;
      failcount = 0;
    }    
    else{
      Serial.print(F("failed "));
      failcount++;
    }
    Serial.print(Txdata.count);
    Serial.print(F(", "));
    Serial.println(currentRFmilli);

    if (failcount > 4){
      fresh = false; // do not send a lot of messages continously
    }
  }

  return fresh;
}


void loop() {

  network.update();

  currentmilli = millis();

  receiveRFnetwork(currentmilli);

  //************************ sensors ****************//

  if (!activePIR){
    if (digitalRead(PIR_PIN) == LOW){
      activePIR = true;
      newdata = true;
    }
  }
  if (pressBUTTON){
    activeBUTTON = true;
    newdata = true;
    pressBUTTON = false;
  }  

  //************************ sensors ****************//

  trackDetectionAndButton(currentmilli);

  newdata = transmitRFnetwork(newdata, currentmilli);
}

void buttonPress(){
  if (!activeBUTTON){
    pressBUTTON = true;
    Serial.print(F("Button press: "));
    Serial.println(millis());
  }
}
