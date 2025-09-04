/*
 * RF-Nano, PIR sensor(s) and distance sensor as well as off button, 2 active relays
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#include "sensors.h"
//#include "drivers.h"
#include "relays.h"

#define radioChannel 104
#define CE_PIN 10
#define CSN_PIN 9

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t kitchen_node = 02; // Address of this node in the kitchen
const uint16_t base_node = 00;    // Address of the home/host/controller node in Octal format

unsigned long const keywordvalM = 0xfeedbeed; 

void setup() {
  Serial.begin(115200);

  //setupsensors();
  //setupdrivers();
  setuprelays();

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
  radio.setPALevel(RF24_PA_LOW, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, kitchen_node);

  attachInterrupt(digitalPinToInterrupt(pinPressButton), buttonPress, FALLING); // trigger when button is pressed
}

//===== Receiving =====//
bool receiveRFnetwork(unsigned long currentRFmilli){
  bool mreceived = false;

  // Check for incoming data details
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      // Display the incoming millis() values from sensor nodes
      case 'Y': 

        mreceived = true;
        break;
      // Display the incoming millis() values from sensor nodes
      case 'Z': 
    
        mreceived = true;
        break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.println(header.type);
    }
  }
  // commandaction = RelayState::R_On;
  // commandaction = RelayState::R_Off;
  return mreceived;
}

//===== Sending =====//
void transmitRFnetwork(bool fresh, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  //if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
  // if (fresh)
  //   if ((unsigned long)(currentRFmilli - sendingTimer) > 5000){

  //   sendingTimer = currentRFmilli;

  //   network_payload Txdata;
  //   Txdata.keyword = keywordvalM;
  //   Txdata.timing = currentRFmilli;
  //   Txdata.counter = counter++;

  //   RF24NetworkHeader header0(joynode, 'M'); // address where the data is going
  //   w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
  //   if (!w_ok){ // retry
  //     failcount++;
  //     delay(50);
  //     w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
  //   }
  //   Serial.print(F("Message send ")); 
  //   if (w_ok){
  //     failcount = 0;
  //   }    
  //   else{
  //     Serial.print(F("failed "));
  //     failcount++;
  //   }
  //   Serial.print(Txdata.counter);
  //   Serial.print(F(", "));
  //   Serial.println(currentRFmilli);

  // }
}

unsigned long currentmilli = 0;
//uint8_t sensorstatus = 0;
bool newdata = false;
bool relayserror = false;

void loop() {

  network.update();

  currentmilli = millis();

  newdata = receiveRFnetwork(currentmilli);

  //************************ sensors/actuators ****************//

  // sensorstatus = checkSensors(currentmilli);

  relayserror = relaytracking(newdata, currentmilli);
  //driveRelays(sensorstatus, currentmilli);

  //************************ sensors/actuators ****************//

  if (relayserror){
    Serial.print(F("Relays error detected "));
    Serial.println(currentmilli);
  }

  //transmitRFnetwork(ack, currentmilli);

}
