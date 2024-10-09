 
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <EEPROM.h>
//#include <printf.h>
 
#define radioChannel 78
/** User Configuration per 'slave' node: nodeID **/
#define slavenodeID 5
#define masterNodeID 0


/**** Configure the nrf24l01 CE and CSN pins ****/
/*
https://www.youtube.com/watch?v=8p_hN53TxY8

https://www.electronicwings.com/arduino/nrf24l01-interfacing-with-arduino-uno


this one:
https://forum.arduino.cc/t/simple-nrf24l01-2-4ghz-transceiver-demo/405123/3

*/
RF24 radio(10, 9);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
 
// Payload from MASTER
struct payload_from_master {
  unsigned long counter;
  bool showLed;
};
 
// Payload to SLAVES
struct payload_from_slave {
  uint8_t nodeId;
  unsigned long timer;
  bool ledShown;
};
 
uint32_t displayTimer = 0;
uint32_t counter = 0;
bool showLed;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  if (!radio.begin()){
    Serial.println(F("Radio hardware not responding."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);

  // Set the nodeID to 0 for the master node
  mesh.setNodeID(masterNodeID);
  Serial.print(F("Starting Master of the mesh, nodeID: "));
  Serial.println(mesh.getNodeID());
  // Connect to the mesh
  mesh.begin(radioChannel);
}
 
void loop() {
  // Call mesh.update to keep the network updated
  mesh.update();
 
  // In addition, keep the 'DHCP service' running 
  // on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
 
  // Check for incoming data from the sensors
  if(network.available()){
    RF24NetworkHeader header;
    network.peek(header);
    payload_from_slave payload;
  
    switch(header.type){
      // Display the incoming millis() values from sensor nodes
      case 'M': network.read(header, &payload, sizeof(payload));
        Serial.print(F(" On slave: "));
        Serial.print(payload.nodeId);
        Serial.print(F(", millis: "));
        Serial.print(payload.timer);
        Serial.print(F(", Led shown: "));
        Serial.println(payload.ledShown);
        break;
      default: network.read(header,0,0);
        Serial.println(header.type);
    }
  }
  
  // Meanwhile, every 5 seconds...
  if(millis() - displayTimer > 5000){
    displayTimer = millis();

    //// SHOW DHCP TABLE - BEGIN
    if (mesh.addrListTop > 0){
      Serial.println(F(" "));
      Serial.println(F("********Assigned Addresses********"));
      for(int i=0; i<mesh.addrListTop; i++){
        Serial.print(F("NodeID: "));
        Serial.print(mesh.addrList[i].nodeID);
        Serial.print(F(" RF24Network Address: 0")); // this is in octal
        Serial.println(mesh.addrList[i].address,OCT);
      }
      Serial.println(F("**********************************"));
    }
    else{
      Serial.print(F(" . "));
    }
    //// SHOW DHCP TABLE - END

    //// Send same master message to all slaves - BEGIN
    if (mesh.addrListTop > 0){
      showLed = !showLed;

      for(int i=0; i<mesh.addrListTop; i++){
        counter++;
        payload_from_master payload = {counter, showLed};
        RF24NetworkHeader header(mesh.addrList[i].address, OCT);
        // int x = network.write(header, &payload, sizeof(payload));
        network.write(header, &payload, sizeof(payload));
      }
    }
    else{
      Serial.println(F("No network node to write to"));
    }
    //// Send same master message to all slaves - END
  }
}
