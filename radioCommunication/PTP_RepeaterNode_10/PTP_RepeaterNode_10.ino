/*

Form a repeater between the base node and the remote node.

Base <---> Repeater <-----------> Remote
 00          010                    01

Target: RF-NANO with additional RF24 module Long Range
nRF24L01-PA (CE,CSN) connected to pin 8, 7
location schuur SO 148

*/

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include "repeater.h"

RF24 radio1(10, 9);              // onboard nRF24L01 (CE, CSN)
RF24 radio2(8, 7);               // external nRF24L01 (CE, CSN)
RF24Network network1(radio1);    // Include the radio in the network
RF24Network network2(radio2);    // Include the radio in the network

const uint16_t base_node = 00;     // Address of the other, base node in Octal format
const uint16_t repeater_node = 01; // Address of the repeater node in Octal format (04, 031, etc.)
const uint16_t remote_node = 00;   // Address of the other, remote node in Octal format

void setup() {
  Serial.begin(230400); // actual baudrate in IDE 57600 (RF-NANO), there is somewhere a mismatch in clock factor of 4
  String timestamp = __TIMESTAMP__;
  Serial.print(F("PTP_RepeaterNode_10, creation/build time: "));
  Serial.println(timestamp);
  Serial.flush(); 
  
  SPI.begin();
  radio1.begin();
  radio2.begin();
  radio1.setPALevel(RF24_PA_MIN, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio2.setPALevel(RF24_PA_MIN, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network1.begin(70, repeater_node); // (channel, node address)
  network2.begin(60, repeater_node); // (channel, node address)
  radio1.setDataRate(RF24_250KBPS); // (RF24_2MBPS);
  radio2.setDataRate(RF24_250KBPS); // (RF24_2MBPS);

  Serial.println(" ");  
  Serial.println(" *************** ");  
  Serial.println(" "); 
  Serial.flush();  
}

net_payload nw1Data = EmptyData;
net_payload nw2Data = EmptyData;
bool transmit = false;
//bool blink = false;

void loop() {

  network1.update();
  network2.update();

  nw1Data = receiveRFnetwork(network1, base_node);
  if (nw1Data.data1 > 0xab000000) { // data1 should be the keyword
    transmit = transmitRFnetwork(network2, remote_node, nw1Data);
    if (transmit) {
      Serial.println("nw1, base -> nw2, remote"); 
    }
  }

  nw2Data = receiveRFnetwork(network2, remote_node);
  if (nw2Data.data1 > 0xab000000) { // data1 should be the keyword
    transmit = transmitRFnetwork(network1, base_node, nw2Data);
    if (transmit) {
      Serial.println("nw2, remote -> nw1, base"); 
    }
  }

}
