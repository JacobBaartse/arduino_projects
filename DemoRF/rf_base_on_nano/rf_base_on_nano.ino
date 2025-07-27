/*
 * RF-Nano, base node for demo purposes
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#define radioChannel 104

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node00 = 00; // Address of the home/host/controller node in Octal format

unsigned long const keywordvalM = 0xfeedbeef; 
unsigned long const keywordvalS = 0xbeeffeed; 

struct joystick_payload{
  uint32_t keyword;
  uint32_t timing;
  uint16_t xvalue;
  uint16_t yvalue;
  uint8_t count;
  uint8_t bvalue;
  uint8_t sw1value;
  uint8_t sw2value;
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
  network.begin(radioChannel, node00);
}

unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;

//===== Receiving =====//
void receiveRFnetwork(){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    network_payload Rxdata;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node != node00) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    if (Rxdata.keyword == keywordvalS){

      Serial.println(F("new data received"));
    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
}

//===== Sending =====//
bool transmitRFnetwork(bool fresh){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  unsigned long currentRFmilli = millis();
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
    sendingTimer = currentRFmilli;

    joystick_payload Txdata;
    Txdata.keyword = keywordvalM;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    Txdata.xvalue = xValue;
    Txdata.yvalue = yValue;
    Txdata.bvalue = bValue;
    Txdata.sw1value = sw1Value;
    Txdata.sw2value = sw2Value;

    Serial.print(F("Message: "));
    Serial.print(Txdata.count);
    Serial.print(F(", xvalue: "));
    Serial.print(Txdata.xvalue);
    Serial.print(F(", yvalue: "));
    Serial.print(Txdata.yvalue);
    Serial.print(F(", bvalue: "));
    Serial.print(Txdata.bvalue);
    Serial.print(F(", sw1value: "));
    Serial.print(Txdata.sw1value);        
    Serial.print(F(", sw2value: "));
    Serial.println(Txdata.sw2value);

    RF24NetworkHeader header0(node00, 'J'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      failcount++;
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    Serial.print(F("Message send ")); 
    if (w_ok){
      bValue = 0; 
      sw1Value = 0;
      sw2Value = 0;
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

    if (failcount > 10){
      fresh = false; // do not send a lot of messages continously
    }

    if(!fresh){ // clear buttons status always after 5 seconds
      bValue = 0; 
      sw1Value = 0;
      sw2Value = 0;
    }

  }

  return fresh;
}



void loop() {


  network.update();

  currentmilli = millis();

  receiveRFnetwork();

  //************************ sensors ****************//


  //************************ sensors ****************//

  newdata = transmitRFnetwork(newdata);

}
