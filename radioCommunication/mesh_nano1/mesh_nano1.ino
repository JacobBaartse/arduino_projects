 
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
 
#define radioChannel 78
/** User Configuration per 'slave' node: nodeID **/
#define slaveNodeID 3
#define masterNodeID 0


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9);
RF24Network network(radio);
RF24Mesh mesh(radio, network);
 
unsigned long const keywordvalM = 0xfeebbeef; 
unsigned long const keywordvalS = 0xbeeffeeb; 

// Payload from/for MASTER
struct payload_from_master {
  unsigned long keyword;
  uint32_t counter;
  bool showLed;
};
 
// Payload from/for SLAVE
struct payload_from_slave {
  unsigned long keyword;
  uint32_t timing;
  bool ledShown;
  uint8_t nodeId;
};
 
uint32_t sleepTimer = 0;
bool showLed = false;
bool meshrunning = false;

void restart_arduino(){
  Serial.println("Restart the arduino board...");
  delay(2000);
  //NVIC_SystemReset();
}

bool meshstartup(){
  if (meshrunning){
    Serial.println(F("Radio issue, turn op PA level?"));
  }
  return mesh.begin(radioChannel);
}

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
  mesh.setNodeID(slaveNodeID);
  // Serial.print(F("Setup node: "));
  // Serial.print(slavenodeID);
  Serial.println(F(", connecting to the mesh..."));
  // Connect to the mesh
  meshrunning = meshstartup();
  Serial.print(F("Starting the mesh, nodeID: "));
  Serial.println(mesh.getNodeID());
}
 
unsigned int mesherror = 0;

void loop() {
  if (mesherror > 9) {
    meshrunning = meshstartup();
    mesherror = 0;
  }
  // Call mesh.update to keep the network updated
  mesh.update();
 
  //// Receive a message from master if available - START
  while (network.available()) {
    RF24NetworkHeader header;
    payload_from_master payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print(F("Received packet #"));
    Serial.print(payload.counter);
    Serial.print(F(", show led="));
    Serial.println(payload.showLed);
    if (payload.keyword == keywordvalM) {

    }
    else{
      Serial.println("Wrong keyword"); 
    }
 
    /*
    // this LED is not connected, this example uses the serial output to confirm the mesh is working
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

  //// Send to the master node every two seconds - BEGIN
  if (millis() - sleepTimer > 2000) {
    sleepTimer = millis();
    showLed = !showLed;
    payload_from_slave payloadM = {keywordvalS, sleepTimer, showLed, slaveNodeID};
 
    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&payloadM, 'M', sizeof(payloadM))) {
      // If a write fails, check connectivity to the mesh network
      if (!mesh.checkConnection()) {
        //refresh the network address
        Serial.println(F("Renewing Address"));
        mesh.renewAddress();
      } else {
        Serial.println(F("Send fail, Test OK"));
        mesherror++;
      }
    } else {
      Serial.print(F("Send to Master OK: "));
      Serial.println(payloadM.timing);
      mesherror = 0;
    }
  }
  //// Send to the master node every two seconds - END
}
