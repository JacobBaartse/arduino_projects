/*
 * Schuur node for the radio network, as well as sensors and detectors for lighting control
 *
 * https://github.com/nulllaborg/lgt-rf-nano
 *
 */

#include "networking.h"
#include "relays.h"
#include "sensors.h"

void setup() {
  Serial.begin(115200);
  
  setuprelays();
  setupsensors();
  // delay(1000);

  Serial.println();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush();  

  setupRFnetwork();

  Serial.println(F("\n ******"));  
  Serial.println(); 
  Serial.flush(); 
}

unsigned long currentMillis = 0; // stores the value of millis() in each iteration of loop()
unsigned int receiveaction = 0;
unsigned int transmitaction = 0;
unsigned int sensoraction = 0;
RelayState relaysstatus = RelayState::R_Off;
RelayState relayaction = RelayState::R_Off;

void loop() {
  
  currentMillis = millis();   // capture the value of millis() only once in the loop

  network.update();

  //===== Receiving =====//
  receiveaction = receiveRFnetwork(currentMillis);
  bool receivedfresh = receiveaction > 9;

  //===== Sending =====//
  transmitaction = transmitRFnetwork(currentMillis, receivedfresh);

  // sensors
  sensoraction = checkSensors(currentMillis, sensoraction);

//  #define mask_button 1
// #define mask_no_movement 2
// #define mask_remain_on 4
// #define mask_auto_on 8
// #define mask_auto_off 16

  //bool buttonpressed = sensoraction & mask_button








  // switch(sensoraction){ // from buttons and sensors
  //   case 100:
  //   case 1000:
  //     relayaction = RelayState::R_On;
  //   break;
  //   case 10000:
  //     relayaction = RelayState::R_Off;
  //   break;
  //   default: // if nothing from the local sensors
  //     switch(receiveaction){ // check remote instructions
  //       case 100:
  //         relaysstatus = RelayState::R_On;
  //       break;
  //       case 200:
  //         relaysstatus = RelayState::R_Off;
  //       break;
  //       default:
  //         if (relaysstatus == relayaction){ // if status has become the requested status
  //           relaysstatus = RelayState::R_Off;
  //         }
  //     }
  // }

  // actors
  relaysstatus = handleRelay(currentMillis, relayaction);

}
