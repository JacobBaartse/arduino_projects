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

/* one button on the collector to disable the alarm(s) as a local acknowledge that the detection is noticed
 * another button to enable the alarming (when a fresh detection is arriving)
 */
#define BUTTON_PIN1 2
#define BUTTON_PIN2 3

#define BUZZER_PIN 6


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

uint16_t detectornode = 00; // Address of this node in Octal format (04, 031, etc.)
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
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as an output

  // if (digitalRead(CFG_PIN0) == LOW){ // PIN active
  // }
  // if (digitalRead(CFG_PIN1) == LOW){ // PIN active
  // }
  if (digitalRead(CFG_PIN2) == LOW){ // PIN active
    radiolevel = 1;
  }
  if (digitalRead(CFG_PIN3) == LOW){ // PIN active
    radiolevel = radiolevel + 2;
  }

  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  // RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 
  //radio.setPALevel(RF24_PA_MIN, 0);
  radiolevel = RF24_PA_LOW;
  radio.setPALevel(radiolevel, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, basenode);
  Serial.print(F("radioChannel: "));
  Serial.print(radioChannel);
  Serial.print(F(", level: "));
  Serial.println(radiolevel);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN1), buttonPress1, FALLING); // trigger when button1 pressed
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN2), buttonPress2, FALLING); // trigger when button2 pressed
}
 
unsigned long currentmilli = 0;
uint16_t detectorscount = 0;
bool pressBUTTON1 = false;
bool pressBUTTON2 = false;
bool activeBUTTON1 = false;
bool activeBUTTON2 = false;

void drivebuzzer(bool buzzerstatus){
static uint16_t buzzertone = 2000;

  if (buzzerstatus){
    if (digitalRead(CFG_PIN0) == LOW){ // PIN active
      buzzertone = 1000;
    }
    if (digitalRead(CFG_PIN1) == LOW){ // PIN active
      buzzertone += 2000;
    }
    tone(BUZZER_PIN, buzzertone);
  }
  else {
    noTone(BUZZER_PIN); 
  }
}

unsigned long reportingTime = 0;

void trackDetectionsAndButtons(unsigned long currentDetectMillis){
  static unsigned long activationTime = 0;
  static uint8_t reportingdog = 0;
  static bool alarming = false;

  if (detectorscount > 0){
    // activate LED and sound
    drivebuzzer(true);
    
  }
  else {
    // turn off LED and sound
    drivebuzzer(false);

  }

  if (alarming){ // show LED and sound (buzzer)
    if (!activeBUTTON1){ // button 1 means alarm acknowledged, do not buzz
      drivebuzzer(false);

    }
    // activate alarm LED

  }

  // if nothing happened
  if (false){
    // at least print for debugging something to know the software is still running
    if ((unsigned long)(currentDetectMillis - reportingTime) > 60000){
      Serial.print(F("Running detection tracking: "));
      Serial.println(currentDetectMillis);
      reportingTime = currentDetectMillis;
      reportingdog += 1;
    }
  }
  else{
    reportingdog = 0;
  }

}

bool pingreceived = false;

//===== Receiving =====//
uint16_t receiveRFnetwork(unsigned long currentRFmilli){
  static unsigned long receivingTime = 0;
  unsigned long diffTime = 0;
  uint16_t nodereceived = 00;

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    detector_payload Rxdata;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node == basenode) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    nodereceived = header.from_node;
    if (Rxdata.keyword == keywordvalD){
      diffTime = (unsigned long)((currentRFmilli - receivingTime));
      receivingTime = currentRFmilli;
      reportingTime = currentRFmilli;
      Serial.print(F("new data received, time diff: "));
      Serial.print(diffTime);
      Serial.print(F(", dvalue: "));
      Serial.print(Rxdata.dvalue);
      Serial.print(F(", sw1 (PIR): "));
      Serial.print(Rxdata.sw1value);
      Serial.print(F(", sw2 (BUTTON): "));
      Serial.println(Rxdata.sw2value);

      pingreceived = ((Rxdata.dvalue==0xff)&&(Rxdata.sw1value==0xff)&&(Rxdata.sw2value==0xff));

      if (pingreceived){
        Serial.print(nodereceived);
        Serial.print(F(" PING received: "));
      }
      else{
        if (detectorscount < 0xff00)
          detectorscount += Rxdata.dvalue;
        Serial.print(F("detectorscount: "));
        Serial.print(detectorscount);
        Serial.print(F(", timing: "));
      }
      Serial.println(currentRFmilli);
    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
  return nodereceived;
}

//===== Sending =====//
bool transmitRFnetwork(bool fresh, uint16_t node_id, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  //if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
  if (fresh){
    sendingTimer = currentRFmilli;

    detector_payload Txdata;
    Txdata.keyword = keywordvalD;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    // Txdata.bvalue = bValue;
    // Txdata.sw1value = sw1Value;
    // Txdata.sw2value = sw2Value;

    Serial.print(F("Message: "));
    // Serial.print(F(", xvalue: "));
    // Serial.print(Txdata.xvalue);
    // Serial.print(F(", yvalue: "));
    // Serial.print(Txdata.yvalue);
    // Serial.print(F(", bvalue: "));
    // Serial.print(Txdata.bvalue);
    // Serial.print(F(", sw1value: "));
    // Serial.print(Txdata.sw1value);        
    // Serial.print(F(", sw2value: "));
    // Serial.println(Txdata.sw2value);

    RF24NetworkHeader header0(node_id, 'B'); // address where the data is going
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

    if (failcount > 2){
      fresh = false; // do not send a lot of messages continously
    }
  }

  return fresh;
}


void loop() {

  network.update();

  currentmilli = millis();

  pingreceived = false;
  detectornode = receiveRFnetwork(currentmilli);

  //************************ sensors ****************//

  // detectionnode is nodereceived, if > 0
  if (detectornode > 0)
    newdata = true; // received a message from a detector

  if (pressBUTTON1){
    activeBUTTON1 = true;
    newdata = true;
    pressBUTTON1 = false;
  }  
  if (pressBUTTON2){
    activeBUTTON2 = true;
    newdata = true;
    pressBUTTON2 = false;
  }  

  //************************ sensors ****************//

  trackDetectionsAndButtons(currentmilli);

  if (pingreceived){
    newdata = true;
  }
  // possible to send acknowledge to the detector node
  newdata = transmitRFnetwork(newdata, detectornode, currentmilli);

}

void buttonPress1(){
  if (!activeBUTTON1){
    pressBUTTON1 = true;
    Serial.print(F("Button press 1: "));
    Serial.println(millis());
  }
}

void buttonPress2(){
  if (!activeBUTTON2){
    pressBUTTON2 = true;
    Serial.print(F("Button press 2: "));
    Serial.println(millis());
  }
}
