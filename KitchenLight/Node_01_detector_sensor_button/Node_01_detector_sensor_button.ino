/*
 * RF-Nano, headers, transmitter for PIR/distance sensor/button
 */

#include "RF24.h"
#include <RF24Network.h>
#include <SPI.h>

#include "distance.h"

#define radioChannel 110
#define CE_PIN 10
#define CSN_PIN 9

// one button or more buttons can be connected to trigger human presence manually
#define BUTTON_PIN 3
#define PIR_PIN A3

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t detectornode = 01; // Address of this node in Octal format (04, 031, etc.)
const uint16_t basenode = 00; // Address of the home/host/controller node in Octal format
const uint8_t radiolevel = RF24_PA_LOW; // RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 

const unsigned long keywordvalB = 0xbeeffeed; 
const unsigned long keywordvalD = 0xdeeffeeb; 

struct base_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t bvalue;
};

struct detector_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t dvalue;
  uint8_t cvalue;
};

bool newdata = false;
unsigned long currentmilli = 0;
uint8_t detectionValue = 0;
uint8_t commandValue = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { // some boards need this because of native USB capability
    delay(10);
  }

  // setup for sensors
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);
  setupDistance();

  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.print(F("Detectornode: "));
  Serial.println(detectornode);
  Serial.flush(); 

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio HW error."));
    while (true) delay(1000);
  }
  radio.setPALevel(radiolevel, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, detectornode);
  Serial.print(F("radioChannel: "));
  Serial.print(radioChannel);
  Serial.print(F(", level: "));
  Serial.println(radiolevel);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPress, FALLING); // trigger when button is pressed

  Serial.println(F(" ----"));
}

bool activePIR = false;
bool PIRconfirmed = false;
bool activeBUTTON = false;
uint16_t objectdistance = 0;

//bool trackDetectionAndButton(bool pfresh, unsigned long currentDetectMillis){
bool trackSensors(bool pfresh, unsigned long currentDetectMillis){
  static unsigned long activationTime = 0;
  static unsigned long clearingTime = 0;
  static bool activated = false;
  bool fresh = pfresh;

  if (activated){

    if (activeBUTTON){ // turn off, when PIR clear
      if ((unsigned long)(currentDetectMillis - activationTime) > 3000){ // 3 seconds
        if (!activePIR){ // if PIR cleared after button press
          commandValue = 85;
          clearingTime = currentDetectMillis;
          Serial.print(F("BUTTON detection (OFF) "));
          Serial.println(currentDetectMillis);
          fresh = true;
          activated = false;
          activePIR = false;
          PIRconfirmed = false;
          activeBUTTON = false;
        }
      }
      else { // pressed button to quickly or too long
        activeBUTTON = false;
      }
    }
    else {    
      if ((unsigned long)(currentDetectMillis - activationTime) > 60000){ // 60 seconds no new activation
        PIRconfirmed = false;
        activeBUTTON = false;
        detectionValue = 0;
        commandValue = 0;
        Serial.print(F("Reset detections "));
        Serial.println(currentDetectMillis);
        activated = false;
      }
    }

  }
  else {
    if (PIRconfirmed){
      commandValue = 95;
      Serial.print(F("PIR detection confirmed "));
      Serial.println(currentDetectMillis);
      activated = true;
    }
    else { // if PIR has to be confirmed from distance measurement
      if (activePIR){
        objectdistance = measureDistance(currentDetectMillis);
        if (objectdistance < 0xff00){
          Serial.print(F(" distance "));
          Serial.print(objectdistance);
          Serial.println(F(" cm"));
          PIRconfirmed = objectdistance < 50; // binnen 50 cm
        }
      }
    }
    if (activeBUTTON){ // turn on in case this is not done yet
      commandValue = 90;
      activationTime = currentDetectMillis;
      Serial.print(F("BUTTON detection (ON) "));
      Serial.println(currentDetectMillis);
      activated = true;
      activeBUTTON = false;
    }
    if (activated){
      activationTime = currentDetectMillis;
      detectionValue = 0xff;
      fresh = true;
    }
  }
  return fresh;
}

//===== Receiving =====//
bool receiveRFnetwork(){
  // unsigned long currentRFmilli = millis();
  bool mreceived = false;

  if (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    network.peek(header);
    if ((header.from_node != basenode)||(header.type != 'B')) {
      Serial.print(F("Received unexpected message, from_node: "));
      Serial.print(header.from_node);
      Serial.print(F(", type: "));
      Serial.print(header.type);
      Serial.println(char(header.type));
    }
    else{
      base_payload Rxdata;
      network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
      if (Rxdata.keyword == keywordvalB){
        Serial.print(F("Data received from base/collector "));
        Serial.println(millis());
        mreceived = true;
        // in case a message is received, with specific data, the detector could be 'reset' (for example)


      }
      else{
        Serial.println(F("Keyword failure"));
      }
    }
  }

  return mreceived;
}

//===== Sending =====//
bool transmitRFnetwork(bool pfresh){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool fresh = pfresh;
  unsigned long currentRFmilli = millis();
  bool w_ok;

  // Every x seconds, or on new data
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 60000)){
    sendingTimer = currentRFmilli;

    detector_payload Txdata;
    Txdata.keyword = keywordvalD;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    Txdata.dvalue = detectionValue;
    Txdata.cvalue = commandValue;

    Serial.print(F("Message dvalue: "));
    Serial.print(Txdata.dvalue);
    Serial.print(F(", cvalue: "));
    Serial.print(Txdata.cvalue);        

    RF24NetworkHeader header0(basenode, 'K'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    Serial.print(F(", send ")); 
    if (w_ok){
      fresh = false;
      failcount = 0;
    }    
    else{
      Serial.print(F("failed "));
      failcount++;
    }
    Serial.print(F("m # "));
    Serial.print(Txdata.count);
    Serial.print(F(", "));
    Serial.println(currentRFmilli);

    if (failcount > 4){
      fresh = false; // do not send a lot of messages continously
    }
  }

  return fresh;
}

bool pressBUTTON = false;
uint8_t remPIR1 = 3;
uint8_t curPIR1 = 3;
unsigned long difPIR = 3;
unsigned long difPIRtime1 = 0;

void loop() {

  network.update();

  currentmilli = millis();

  //newdata = receiveRFnetwork(currentmilli);

  //************************ sensors ****************//
  newdata = false;

  curPIR1 = digitalRead(PIR_PIN);
  if (curPIR1 != remPIR1){
    difPIR = (unsigned long)(currentmilli - difPIRtime1);
    Serial.print(currentmilli);
    Serial.print(F(" PIR 1 change "));
    Serial.print(difPIR);
    Serial.print(F(" to "));
    Serial.println(curPIR1);
    remPIR1 = curPIR1;
    difPIRtime1 = currentmilli;

    // // if change in PIR, refresh globals
    // if (activePIR){
    //   if (curPIR1 == LOW){
    //     activePIR = false;
    //   }
    // }
    // else{
    //   if (curPIR1 == HIGH){
    //     activePIR = true;
    //   }
    // }

    // if change in PIR, refresh globals
    activePIR = curPIR1 == HIGH;
  }

  if (pressBUTTON){
    activeBUTTON = true;
    newdata = true;
    pressBUTTON = false;
  } 

  //************************ sensors ****************//

  newdata = trackSensors(newdata, currentmilli);
  
  transmitRFnetwork(newdata);

}

void buttonPress(){
  if (!activeBUTTON){
    pressBUTTON = true;
    Serial.print(F("Button press: "));
    Serial.println(millis());
  }
  else {
    Serial.println(F("Button already activated! "));
  }
}
