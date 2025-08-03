/*
 * RF-Nano, headers, USB-C, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#define radioChannel 98 

#define CFG_PIN0 A0
#define CFG_PIN1 A1
#define CFG_PIN2 A2
#define CFG_PIN3 A3
// #define CFG_PIN4 6
// #define CFG_PIN5 7
// #define CFG_PIN6 8
// #define CFG_PIN7 9

#define PIR_PIN 4
#define BUTTON_PIN 5


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

uint16_t detectornode = 05; // configtester node id

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
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.println(F(" ***** >< *****"));  
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
  network.begin(radioChannel, detectornode);
}
 

void loop() {

  delay(5000);

  // read all config PINS and print the result

  Serial.print(F("CFG_PIN0: "));  
  Serial.println(digitalRead(CFG_PIN0));  
  Serial.print(F("CFG_PIN1: "));  
  Serial.println(digitalRead(CFG_PIN1));  
  Serial.print(F("CFG_PIN2: "));  
  Serial.println(digitalRead(CFG_PIN2));  
  Serial.print(F("CFG_PIN3: "));  
  Serial.println(digitalRead(CFG_PIN3));  
 
  Serial.print(F("PIR_PIN: "));  
  Serial.println(digitalRead(PIR_PIN)); 

  Serial.print(F("BUTTON_PIN: "));  
  Serial.println(digitalRead(BUTTON_PIN));  

}
