/*
 * repeater node for the PTP network, as well as sensors and detectors for shed lighting
 */

#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>
#include "networking.h"
#include "relays.h"
#include "sensors.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush();  

  setuprelays();

  setupsensors();

  setupRFnetwork();

  Serial.println(F("\n ******"));  
  Serial.println(); 
  Serial.flush(); 
}

unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
unsigned int receiveaction = 0;
unsigned int transmitaction = 0;
unsigned int sensoraction = 0;
unsigned int relayaction = 0;

void loop() {
  // put your main code here, to run repeatedly:
  currentMillis = millis();   // capture the value of millis() only once in the loop

  network.update();

  //===== Receiving =====//
  receiveaction = receiveRFnetwork(currentMillis);

  //===== Sending =====//
  transmitaction = transmitRFnetwork(currentMillis);

  // sensors
  sensoraction = 0; // checkSensors(currentMillis);

  // actors
  relaysaction = handleRelays(currentMillis);

}
