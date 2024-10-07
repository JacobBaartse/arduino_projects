 
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
https://www.youtube.com/watch?v=_8KZoNWa-nw
9 CSN
10 CE
11 MOSI
12 MISO
13 SCK
*/
RF24 radio(10, 9);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
 
/*
 * User Configuration: nodeID - A unique identifier for each radio. Allows addressing
 * to change dynamically with physical changes to the mesh.
 *
 * In this example, configuration takes place below, prior to uploading the sketch to the device
 * A unique value from 1-255 must be configured for each node.
 */
 /*
#define nodeID 5
 
uint32_t displayTimer = 0;
 
struct payload_t {
  unsigned long ms;
  unsigned long counter;
};
 
/*
void setup() {
  Serial.begin(115200);
 
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
 
  // Set the PA Level to MIN and disable LNA for testing & power supply related issues
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0);
 
  // Connect to the mesh
  Serial.println("Connecting to the mesh...");
  if (!mesh.begin()) {
    if (radio.isChipConnected()) {
      do {
        // mesh.renewAddress() will return MESH_DEFAULT_ADDRESS on failure to connect
        Serial.println("Could not connect to network.\nConnecting to the mesh...");
      } while (mesh.renewAddress() == MESH_DEFAULT_ADDRESS);
    } else {
      Serial.println("Radio hardware not responding.");
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
          mesh.begin();
        }
      } else {
        Serial.println("Send fail, Test OK");
      }
    } else {
      Serial.print("Send OK: ");
      Serial.println(displayTimer);
    }
  }
 
  /*
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
/* */
 
// Payload to MASTER
struct payload_from_master {
  unsigned long counter;
  bool showLed;
};
 
// Payload from SLAVE
struct payload_from_slave {
  uint8_t nodeId;
  uint32_t timer;
  bool ledShown;
};
 
bool showLed;
uint32_t sleepTimer;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);

  // Set the nodeID manually
  mesh.setNodeID(slavenodeID);
  // Connect to the mesh
  Serial.print(F("Setup node: "));
  Serial.print(nodeID);
  Serial.println(F(", connecting to the mesh..."));
  // Connect to the mesh
  mesh.begin(radioChannel);
}
 
void loop() {
  mesh.update();
 
  /* */
  //// Send to the master node every two seconds - BEGIN
  if (millis() - sleepTimer > 2000) {
    sleepTimer = millis();
    payload_from_slave payload = {nodeID, sleepTimer, showLed};
 
    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&payload, 'M', sizeof(payload))) {
      // If a write fails, check connectivity to the mesh network
      if (!mesh.checkConnection()) {
        //refresh the network address
        Serial.println(F("Renewing Address"));
        mesh.renewAddress();
      } else {
        Serial.println(F("Send fail, Test OK"));
      }
    } else {
      Serial.print(F("Send OK: "));
      Serial.println(payload.timer);
    }
  }
  /* */
  //// Send to the master node every two seconds - END
 
  //// Receive a message from master if available - START
  while (network.available()) {
    RF24NetworkHeader header;
    payload_from_master payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print(F("Received packet #"));
    Serial.print(payload.counter);
    Serial.print(F(", show led="));
    Serial.println(payload.showLed);
 
    /*
    showLed = payload.showLed;
 
    if (payload.showLed) {
      digitalWrite(5, HIGH);
    }
    else {
      digitalWrite(5, LOW);
    }
    /* */
  }
  //// Receive a message from master if available - END
}
