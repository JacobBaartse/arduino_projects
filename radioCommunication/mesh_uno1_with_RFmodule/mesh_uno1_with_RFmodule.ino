/*
 *
 */

#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
 
#define radioChannel 78
/** User Configuration per 'slave' node: nodeID **/
#define slavenodeID 3
#define masterNodeID 0
#define LEDpin1 4
#define LEDpin2 3
#define LEDpin3 2


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(8, 7);
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
 
// Payload from/for SLAVES
struct payload_from_slave {
  unsigned long keyword;
  uint32_t timing;
  bool ledShown;
  uint8_t nodeId;
};
 
uint32_t displayTimer = 0;
uint32_t counter = 0;
bool showLed = false;
bool meshrunning = false;

void restart_arduino(){
  Serial.println("Restart the arduino UNO board...");
  delay(2000);
  NVIC_SystemReset();
}

bool meshstartup(){
  if (meshrunning){
    Serial.println(F("Radio issue, turn up PA level?"));
  }
  return mesh.begin(radioChannel);
}

void setup() {
  pinMode(LEDpin1, OUTPUT);
  pinMode(LEDpin2, OUTPUT);
  pinMode(LEDpin3, OUTPUT);
  digitalWrite(LEDpin2, HIGH);

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
  meshrunning = meshstartup();

  digitalWrite(LEDpin1, HIGH);
  digitalWrite(LEDpin3, HIGH);
}
 
unsigned int mesherror = 0;
uint8_t meshupdaterc = 0;
uint8_t rem_meshupdaterc = 200;

void loop() {
  if (mesherror > 8) {
    meshrunning = meshstartup();
    mesherror = 0;
  }
  // Call mesh.update to keep the network updated
  meshupdaterc = mesh.update();
  if (meshupdaterc != rem_meshupdaterc) {
    Serial.print(F("meshupdaterc: "));
    Serial.println(meshupdaterc);
    rem_meshupdaterc = meshupdaterc;
  }

  // In addition, keep the 'DHCP service' running 
  // on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
 
  // Check for incoming data from the sensors
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      // Display the incoming millis() values from sensor nodes
      case 'M': 
        payload_from_slave payload;
        network.read(header, &payload, sizeof(payload));
        Serial.print(F("Received from Slave nodeId: "));
        Serial.print(payload.nodeId);
        Serial.print(F(", timing: "));
        Serial.print(payload.timing);
        Serial.print(F(", Led shown: "));
        Serial.println(payload.ledShown);
        if (payload.keyword == keywordvalS) {

        }
        else{
          Serial.println("Wrong keyword"); 
        }
        break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.println(header.type);
    }
  }
  
  // Meanwhile, every x seconds...
  if(millis() - displayTimer > 15000) {
    displayTimer = millis();

    //// SHOW DHCP TABLE - BEGIN
    if (mesh.addrListTop > 0) {
      Serial.println(F(" "));
      Serial.println(F("********Assigned Addresses********"));
      for(int i=0; i<mesh.addrListTop; i++){
        Serial.print(F("NodeID: "));
        Serial.print(mesh.addrList[i].nodeID);
        Serial.print(F(" RF24Network Address: 0")); // this is in octal
        Serial.println(mesh.addrList[i].address, OCT);
      }
      Serial.println(F("**********************************"));
    }
    else{
      Serial.print(F(" ."));
      digitalWrite(LEDpin1, LOW);
    }
    //// SHOW DHCP TABLE - END

    //// Send same master message to all slaves - BEGIN
    if (mesh.addrListTop > 0) {
      showLed = !showLed;
      for(int i=0; i<mesh.addrListTop; i++){
        counter += 1;
        payload_from_master payloadS = {keywordvalM, counter, showLed};        
        
        // RF24NetworkHeader header(mesh.addrList[i].address, OCT);
        // // int x = network.write(header, &payload, sizeof(payload));
        // network.write(header, &payloadS, sizeof(payloadS));
        
        if (!mesh.write(&payloadS, 'S', sizeof(payloadS), mesh.addrList[i].nodeID)) {
          Serial.print(F("Send fail, Master to Slave, nodeID: "));
          Serial.print(mesh.addrList[i].nodeID);
          Serial.println(" ");
          mesherror++;
        }
        else {
          Serial.print(F("Send to Slave Node "));
          Serial.print(mesh.addrList[i].nodeID);
          Serial.print(F(" OK: "));
          Serial.println(payloadS.counter);
          mesherror = 0;
        }
      }
    }
    else{
      mesherror++;
      Serial.print(F("No network node to write to ("));
      Serial.print(mesherror);
      Serial.println(F(")"));
    }
    //// Send same master message to all slaves - END
  }
  // end of while network.available



}
