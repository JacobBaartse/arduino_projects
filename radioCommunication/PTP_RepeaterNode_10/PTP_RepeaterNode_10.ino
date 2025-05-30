/*

Form a repeater between the base node and the remote node.

Base <- 100 -> Repeater <----- 110 ------> Remote
 00               01                         00

Target: RF-NANO with additional RF24 module Long Range
nRF24L01-PA (CE,CSN) connected to pin 8, 7
location schuur SO 148

*/

#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "repeater.h"

RF24 radio1(9, 10);              // onboard nRF24L01 (CE, CSN)
RF24 radio2(7, 8);               // external nRF24L01 (CE, CSN)
RF24Network network1(radio1);    // Include the radio in the network
RF24Network network2(radio2);    // Include the radio in the network

const uint16_t base_node = 00;     // Address of the other, base node in Octal format
const uint16_t repeater_node = 01; // Address of the repeater node in Octal format (04, 031, etc.)
const uint16_t remote_node = 00;   // Address of the other, remote node in Octal format

void setup() {
  Serial.begin(230400); // actual baudrate in IDE: 57600 (RF-NANO, micro USB), there is somewhere a mismatch in clock factor of 4
  delay(1000);
  Serial.println();
  Serial.println(__TIMESTAMP__);
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 
  
  SPI.begin();

  radio1.begin();
  radio1.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio1.setDataRate(RF24_1MBPS); // (RF24_2MBPS);
  //radio1.setDataRate(RF24_250KBPS); // (RF24_2MBPS);
  //radio1.setChannel(100);
  // radio1.setAutoAck(true);                                              
  // radio1.enableDynamicPayloads();  
  network1.begin(110, repeater_node); 

  radio2.begin();
  radio2.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio2.setDataRate(RF24_1MBPS); // (RF24_2MBPS);
  //radio2.setDataRate(RF24_250KBPS); // (RF24_2MBPS);
  //radio2.setChannel(102);
  // radio2.setAutoAck(true);                                              
  // radio2.enableDynamicPayloads(); 
  network2.begin(100, repeater_node); 

  Serial.println(F("\n ******"));  
  Serial.println(); 
  Serial.flush();  
}

net_payload nw1Data = EmptyData;
net_payload nw2Data = EmptyData;
bool transmit = false;
bool activitySignal = false;

void loop() {

  network1.update();
  //network2.update();

  if (activitySignal){ // s(t)imulate a radio packet
    nw1Data = DataForNW2;
  }
  else {
    //network1.update();
    nw1Data = receiveRFnetwork(network1, base_node, 1);
  }
  if (nw1Data.data1 != EmptyData.data1){
    Serial.println(F("nw1Data")); 
  }
  if (nw1Data.data1 > 0xab000000) { // data1 should be the keyword
    transmit = transmitRFnetwork(network2, remote_node, nw1Data, 2);
    if (transmit) {
      Serial.println(F("nw1, base -> nw2, remote")); 
    }
  }

  //network1.update();
  network2.update();

  if (activitySignal){ // s(t)imulate a radio packet
    nw2Data = DataForNW1;
  }
  else {
    //network2.update();
    nw2Data = receiveRFnetwork(network2, remote_node, 2);
  }
  if (nw2Data.data1 != EmptyData.data1){
    Serial.println(F("nw2Data")); 
  }
  if (nw2Data.data1 > 0xab000000) { // data1 should be the keyword
    transmit = transmitRFnetwork(network1, base_node, nw2Data, 1);
    if (transmit) {
      Serial.println(F("nw2, remote -> nw1, base")); 
    }
  }

  activitySignal = activitytracker(12);

}
