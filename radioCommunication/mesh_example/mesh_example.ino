/*
 * example that is what is the goal of this trial:
 * https://forum.arduino.cc/t/nrf24-network-master-crossing-readings/575830
 * ToBeDone 
 */


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
//#include <printf.h>
 
 
/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(7, 8);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
 
/*
 * User Configuration: nodeID - A unique identifier for each radio. Allows addressing
 * to change dynamically with physical changes to the mesh.
 *
 * In this example, configuration takes place below, prior to uploading the sketch to the device
 * A unique value from 1-255 must be configured for each node.
 */
#define nodeID 1
 
 
uint32_t displayTimer = 0;
 
struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

//==============
/*
 void loop() {
    getData();
    showData();
}
https://forum.arduino.cc/t/simple-nrf24l01-2-4ghz-transceiver-demo/405123/2
/* */
bool newData = false;

void getData() {
    if ( radio.available() ) {
        radio.read( &dataReceived, sizeof(dataReceived) );
        newData = true;
    }
}

void showData() {
    if (newData) {
        Serial.print("Data received: ");
        Serial.println(dataReceived);
        newData = false;
    }
}
//==============


void setup() {
 
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
 
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
 
  // Set the PA Level to MIN and disable LNA for testing & power supply related issues
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0);
 
  // Connect to the mesh
  Serial.println(F("Connecting to the mesh..."));
  if (!mesh.begin(radioChannel)) {
    if (radio.isChipConnected()) {
      do {
        // mesh.renewAddress() will return MESH_DEFAULT_ADDRESS on failure to connect
        Serial.println(F("Could not connect to network.\nConnecting to the mesh..."));
      } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
    } else {
      Serial.println(F("Radio hardware not responding."));
      while (1) {
        // hold in an infinite loop
      }
    }
  }
}
 
void loop() {
 
  mesh.update();
 
  // Send to the master node every second
  if (millis() - displayTimer >= 1000) {
    displayTimer = millis();
 
    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&displayTimer, 'M', sizeof(displayTimer))) {
 
      // If a write fails, check connectivity to the mesh network
      if (!mesh.checkConnection()) {
        //refresh the network address
        Serial.println("Renewing Address");
        if (mesh.renewAddress() == MESH_DEFAULT_ADDRESS) {
          //If address renewal fails, reconfigure the radio and restart the mesh
          //This allows recovery from most if not all radio errors
          mesh.begin(radioChannel);
        }
      } else {
        Serial.println("Send fail, Test OK");
      }
    } else {
      Serial.print("Send OK: ");
      Serial.println(displayTimer);
    }
  }
 
  while (network.available()) {
    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print("Received packet #");
    Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);
  }
}
