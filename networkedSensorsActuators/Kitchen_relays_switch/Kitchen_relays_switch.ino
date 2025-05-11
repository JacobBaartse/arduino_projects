/*
 * Kitchen node for the radio network, as well as sensors and detectors for lighting
 */

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

unsigned long currentMillis = 0; // stores the value of millis() in each iteration of loop()
unsigned int receiveaction = 0;
unsigned int transmitaction = 0;
unsigned int sensoraction = 0;
RelayState relayaction = RelayState::R_None;
RelayState relayactions = RelayState::R_None;

void loop() {
  // put your main code here, to run repeatedly:
  
  currentMillis = millis();   // capture the value of millis() only once in the loop

  network.update();

  //===== Receiving =====//
  receiveaction = receiveRFnetwork(currentMillis);
  bool receivedfresh = receiveaction > 0;

  //===== Sending =====//
  transmitaction = transmitRFnetwork(currentMillis, receivedfresh);

  // sensors
  sensoraction = checkSensors(currentMillis, sensoraction);

  switch(sensoraction){ // from buttons and sensors
    case 100:
      relayaction = RelayState::R_On;
    break;
    case 200:
      relayaction = RelayState::R_Off;
    break;
    default: // if nothing from the local sensors
      switch(receiveaction){ // check remote instructions
        case 100:
          relayaction = RelayState::R_On;
        break;
        case 200:
          relayaction = RelayState::R_Off;
        break;
        default:
          if (relayactions == relayaction){ // if status has become the requested status
            relayaction = RelayState::R_None;
          }
      }
  }

  // actors
  relayactions = handleRelay(currentMillis, relayaction);

}
