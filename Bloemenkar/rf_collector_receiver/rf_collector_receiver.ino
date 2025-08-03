/*
 * RF-Nano, no headers, USB-C with joystick connected, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>
//#include "Arduino.h"

#define radioChannel 104 // dit wordt instelbaar


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

uint16_t detectornode = 00; // Address of this node in Octal format (04, 031, etc.)
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
  // pinMode(SW_PIN, INPUT_PULLUP);
  // pinMode(SW_PIN1, INPUT_PULLUP);
  // pinMode(SW_PIN2, INPUT_PULLUP);

  // digitalRead(SW_PIN);

  // radioChannel = depending on settings
  // PA level can be depending on settings


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
  network.begin(radioChannel, basenode);
}
 
unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;
uint16_t detectorscount = 0;

void trackDetections(unsigned long currentmillis){

  if (detectorscount > 0){
    // activate LED and sound

  }
  else {
    // turn off LED and sound

  }
}

//===== Receiving =====//
uint16_t receiveRFnetwork(unsigned long currentRFmilli){
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
      Serial.println(F("new data received"));

      if (detectorscount < 0xff00)
        detectorscount += Rxdata.dvalue;
      Serial.print(F("detectorscount: "));
      Serial.print(detectorscount);
      Serial.print(F(", timing: "));
      Serial.print(currentRFmilli);
    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
  return nodereceived;
}

//===== Sending =====//
void transmitRFnetwork(bool fresh, uint16_t node_id, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
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

    RF24NetworkHeader header0(node_id, 'D'); // address where the data is going
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

  }

}

uint16_t snode = 00;

void loop() {

  network.update();

  currentmilli = millis();

  snode = receiveRFnetwork(currentmilli);

  //************************ sensors ****************//

  // snode is nodereceived  if > 0
  newdata = (snode > 0);

  //************************ sensors ****************//

  trackDetections(currentmilli);

  transmitRFnetwork(newdata, snode, currentmilli);

}
