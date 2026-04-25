/*
 * Nano + NRF extender module, , USB-C, using RF24network library
 */

#include "RF24.h"
#include <RF24Network.h>
#include <SPI.h>
#include "distance.h" // here IIC is used, pins A4 and A5

#define radioChannel 98 // dit wordt mogelijk instelbaar
#define CE_PIN  10
#define CSN_PIN  9

/* one button or two buttons can be connected to trigger human presence
 */
#define BUTTON1_PIN 2
//#define BUTTON2_PIN 3

#define PIR1_PIN 7


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

uint16_t detectornode = 01; // Address of this node in Octal format (01 ... 05), maximum 5 detector nodes
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

  // PINs for sensor inputs
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  //pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(PIR1_PIN, INPUT);

  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.print(F("Detectornode: "));
  Serial.println(detectornode);
  Serial.flush(); 

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  // RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 
  radiolevel = RF24_PA_LOW;
  radio.setPALevel(radiolevel, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, detectornode);
  Serial.print(F("radioChannel: "));
  Serial.print(radioChannel);
  Serial.print(F(", level: "));
  Serial.println(radiolevel);

  setupDistance();

  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), buttonPress, FALLING); // trigger when button is pressed
  //attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), buttonPress, FALLING); // trigger when button is pressed
}

unsigned long currentmilli = 0;
uint8_t detectionValue = 0;
uint8_t sw1Value = 0;
uint8_t sw2Value = 0;
bool activePIR = false;
bool PIRconfirmed = false;
bool activeBUTTON = false;
bool pressBUTTON = false;

bool trackDetectionAndButton(unsigned long currentDetectMillis){
  static unsigned long activationTime = 0;
  static bool alarming = false;
  bool fresh = false;

  if (alarming){
    if ((unsigned long)(currentDetectMillis - activationTime) > 60000){ // 60 seconds no new alarming
      PIRconfirmed = false;
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
    if (PIRconfirmed){
      sw1Value = 0xa5;
      Serial.print(F("PIR detection "));
      Serial.println(currentDetectMillis);
      alarming = true;
    }
    if (activeBUTTON){
      sw2Value = 0x5a;
      activationTime = currentDetectMillis;
      Serial.print(F("BUTTON detection "));
      Serial.println(currentDetectMillis);
      alarming = true;
    }
    if (alarming){
      activationTime = currentDetectMillis;
      detectionValue = 0xff;
      fresh = true;
    }
  }
  return fresh;
}

//===== Receiving =====//
bool receiveRFnetwork(unsigned long currentRFmilli){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    detector_payload Rxdata;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if ((header.from_node != basenode)||(header.type != 'B')) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.print(header.from_node);
      Serial.print(F(", type: "));
      Serial.println(header.type);
      break;
    }
    if (Rxdata.keyword == keywordvalD){
      Serial.print(F("Data received from base/collector "));
      Serial.println(currentRFmilli);
      // in case a message is received, with specific data, the detector could be 'reset' (for example)


    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
}

//===== Sending =====//
//bool transmitRFnetwork(bool pfresh, unsigned long currentRFmilli, bool pping){
bool transmitRFnetwork(bool pfresh, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  //static unsigned long pingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;
  bool fresh = pfresh;

  // Every 60 seconds, or on new data
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 60000)){
    sendingTimer = currentRFmilli;

    detector_payload Txdata;
    Txdata.keyword = keywordvalD;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    Txdata.dvalue = detectionValue;
    Txdata.sw1value = sw1Value;
    Txdata.sw2value = sw2Value;

    Serial.print(F("Message dvalue: "));
    //Serial.print(F(" dvalue: "));
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

void watchdogprinting(unsigned long nowmilli){
  static unsigned long nowtiming = 0;
  if (nowtiming + 30000 < nowmilli){
    Serial.print(F("WD: "));
    Serial.println(nowmilli);
    nowtiming = nowmilli;
  }
}

uint8_t remPIR1 = 3;
uint8_t curPIR1 = 3;
//uint8_t remPIR2 = 3;
//uint8_t curPIR2 = 3;
unsigned long difPIR = 3;
unsigned long difPIRtime1 = 0;
//unsigned long difPIRtime2 = 0;
uint16_t mloop = 0;
uint16_t objectdistance = 0;

void loop() {

  network.update();

  currentmilli = millis();

  receiveRFnetwork(currentmilli);

  //************************ sensors ****************//

  curPIR1 = digitalRead(PIR1_PIN);
  if (curPIR1 != remPIR1){
    difPIR = (unsigned long)(currentmilli - difPIRtime1);
    Serial.print(currentmilli);
    Serial.print(F(" PIR 1 change "));
    Serial.print(difPIR);
    Serial.print(F(" to "));
    Serial.println(curPIR1);
    remPIR1 = curPIR1;
    difPIRtime1 = currentmilli;
  }
  if (activePIR){
    if (curPIR1 == LOW){
      activePIR = false;
    }
  }
  else{
    if (curPIR1 == HIGH){
      mloop = 0;
      activePIR = true;
      //newdata = true;
    }
  }
  if (pressBUTTON){
    activeBUTTON = true;
    //newdata = true;
    pressBUTTON = false;
  } 

  // this is limited bij the PIR active signal (2-3 seconds)
  if ((activePIR)&&(!PIRconfirmed)){
    //startDistance(currentmilli);
    //objectdistance = readDistance(currentmilli);
    objectdistance = measureDistance(currentmilli);
    
    if (objectdistance < 0xff00){
      mloop++;
      Serial.print(F(" distance "));
      Serial.print(objectdistance);
      Serial.print(F(" cm, loop: "));
      Serial.println(mloop);
      PIRconfirmed = objectdistance < 200; // smaller than 200 cm
    }
  }

  //************************ sensors ****************//

  newdata = trackDetectionAndButton(currentmilli);
  
  newdata = transmitRFnetwork(newdata, currentmilli);
  // if (currentmilli < collectorpresenttime){ // for a certain amount of time after the base node was 'heared'
  //   newdata = transmitRFnetwork(newdata, currentmilli, false);
  // }
  // else{ // ping for base node
  //   newdata = transmitRFnetwork(newdata, currentmilli, true);
  // }

  watchdogprinting(currentmilli);

}

void buttonPress(){
  if (!activeBUTTON){
    pressBUTTON = true;
    Serial.print(F("Button press: "));
    Serial.println(millis());
  }
}
