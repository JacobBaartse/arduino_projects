/*
 * Nano with external RF module
 */

#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>

//#include <Wire.h>

// #########################################################

//int SR04_I2CADDR = 0x57;

#define pinPIR 7 // PIR pin connection

#define radioChannel 96
/** User Configuration per 'slave' node: nodeID **/
#define slaveNodeID 1
#define masterNodeID 0

/**** Configure the nrf24l01 CE and CSN pins ****/
// for the NANO with onboard RF24 module:
// RF24 radio(10, 9); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external RF24 module:
//RF24 radio(8, 7); // nRF24L01 (CE, CSN)

#define CE_PIN 10
#define CSN_PIN 9
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)

RF24Network network(radio);
RF24Mesh mesh(radio, network);
 
unsigned long const keywordvalM = 0xfeebbeef; 
unsigned long const keywordvalS = 0xbeeffeeb; 

// Payload from/for MASTER
struct payload_from_master {
  unsigned long keyword;
  uint32_t counter;
  bool relay1;
  bool relay2;
};
 
// Payload from/for SLAVE
struct payload_from_slave {
  unsigned long keyword;
  uint32_t timing;
  uint8_t nodeId;
  uint8_t detection;
  uint8_t distance;
};
 
uint32_t sleepTimer = 0;
bool meshrunning = false;

void restart_arduino(){
  Serial.println(F("Restart the Arduino board..."));
  delay(2000);
  //NVIC_SystemReset(); // TBD
}

bool meshstartup(){
  if (meshrunning){
    Serial.println(F("Radio issue, turn op PA level?"));
  }
  return mesh.begin(radioChannel, RF24_250KBPS);
}

bool PIRdetection(){
  bool activedetection = digitalRead(pinPIR) == HIGH;
  //Serial.println(activedetection);
  return activedetection;
}

int distanceMeasurement(){
  static unsigned long remdistance = 0;
  byte ds[3];
  ds[0] = 0;
  ds[1] = 0;
  ds[2] = 0;

  Wire.beginTransmission(SR04_I2CADDR);
  Wire.write(1);          // 1 = cmd to start meansurement
  Wire.endTransmission();
  delay(120);             // 1 cycle approx. 100 ms. 
  int i = 0;
  Wire.requestFrom(SR04_I2CADDR,3);  // read distance       
  while (Wire.available())
  {
    ds[i++] = Wire.read();
  }        
    
  unsigned long distance = (unsigned long)(ds[0]) * 65536;
  distance = distance + (unsigned long)(ds[1]) * 256;
  distance = (distance + (unsigned long)(ds[2])) / 10000;
  if (distance > 0) {
    if (remdistance != distance) {
      remdistance = distance;
      Serial.print(distance);
      Serial.println(" cm");
    }
  }

  return (int)distance;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F(" *************>>"));  

  pinMode(pinPIR, INPUT);

  // Start I2C
  Wire.begin();

  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);

  // Set the nodeID manually
  mesh.setNodeID(slaveNodeID);
  Serial.print(F("Setup node: "));
  Serial.print(slaveNodeID);
  Serial.println(F(", connecting to the mesh..."));
  // Connect to the mesh
  meshrunning = meshstartup();

  Serial.println(F(" <<*************"));  
  Serial.flush();  
}
 
unsigned int mesherror = 0;
uint8_t meshupdaterc = 0;
uint8_t rem_meshupdaterc = 200;
bool detectionval = false;
bool remdetectionval = false;
uint8_t detval = 0;
uint8_t distval = 0;
bool sendDirect = false;

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

  //// Receive a message from master if available - START
  while (network.available()) {
    RF24NetworkHeader header;
    payload_from_master payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print(F("Received packet #"));
    Serial.println(payload.counter);
    if (payload.keyword == keywordvalM) {

    }
    else{
      Serial.println(F("Wrong keyword")); 
    }
  }
  //// Receive a message from master if available - END

  if(sendDirect){
    Serial.print(F(" send direct about to happen ")); 
    Serial.println(millis());
  }
  //// Send to the master node every x seconds - BEGIN
  if ((sendDirect) || (millis() - sleepTimer > 10000)) {
    sleepTimer = millis();
    sendDirect = false;
    payload_from_slave payloadM = {keywordvalS, sleepTimer, slaveNodeID, detval, distval};
    Serial.print(detval);
    Serial.print(F(", "));
    Serial.println(distval);

    // Send an 'M' type message containing the current millis()
    if (!mesh.write(&payloadM, 'M', sizeof(payloadM))) {
      // If a write fails, check connectivity to the mesh network
      if (!mesh.checkConnection()) {
        //refresh the network address
        Serial.println(F("Renewing Address"));
        if (mesh.renewAddress() == MESH_DEFAULT_ADDRESS) {
          // If address renewal fails, reconfigure the radio and restart the mesh
          // This allows recovery from most, if not all radio errors
          meshstartup();
        }
      }
      else {
        //Serial.println(F("Send fail, Test OK"));
        mesherror++;
      }
    } else {
      Serial.print(F("Send to Master OK: "));
      Serial.println(payloadM.timing);
      mesherror = 0;
    }
  }
  //// Send to the master node every x seconds - END

  detectionval = PIRdetection();
  if (remdetectionval != detectionval){
    sendDirect = true;
    remdetectionval = detectionval;
    Serial.print(F("PIR detection: "));
    Serial.println(detectionval);
    detval = 0;
    if (detectionval) detval = 0xff;
  }

  distval = distanceMeasurement();

}
