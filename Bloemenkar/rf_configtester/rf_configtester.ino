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

uint16_t detectornode; // Address of this node in Octal format (04, 031, etc.)
const uint16_t basenode = 00; // Address of the home/host/controller node in Octal format

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
 

void loop() {

  delay(1000);

  // read all config PINS and print the result

  // sw1Value = digitalRead(SW_PIN1);



}
