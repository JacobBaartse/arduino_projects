/*
 * RF networking, repeater from end-point to/from home base, also receiving commands from home base
 */

#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

#define radio_channel 104

// RF Nano, microUSB, LGT8F328
#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);      // internal nRF24L01 (CE, CSN)
RF24Network network(radio);      // include the radio in the network

const uint16_t shed_node = 01; // Address of node in Octal format
const uint16_t base_node = 00; // Address of node in Octal format

void setupRFnetwork(){
  SPI.begin();

  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  //radio.setPALevel(RF24_PA_MIN, false); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
  radio.setDataRate(RF24_1MBPS); // (RF24_2MBPS);

  network.begin(radio_channel, shed_node); // (channel, node address)
}

// Payload for shed
struct shed_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t light; // 0 - no change, 100 - ON, 200 - OFF
};


//===== Receiving =====//
unsigned int receiveRFnetwork(unsigned long currentmilli){
  unsigned int reaction = 0;

  // Check for incoming data from the sensors
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      case 'S': // Message received from HomeController for RemoteNode
        Serial.print(F("Message received from Base: "));
        shed_payload spayload;
        network.read(header, &spayload, sizeof(spayload));
      break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.print(header.type);
    }
    Serial.println(currentmilli);
  } // end of while network.available

  return reaction;
}

//===== Sending =====//
unsigned int transmitRFnetwork(unsigned long currentmilli, bool fresh){
  static unsigned long sendingTimer = 0;
  unsigned int traction = 0;
  bool ok = false;

  // Every x seconds...
  if((fresh)||(currentmilli - sendingTimer > 5000)){
    sendingTimer = currentmilli;
    shed_payload bpayload;
    bpayload.keyword = 0;
    bpayload.count = 0;
    bpayload.light = 0;
    bpayload.timing = currentmilli;
    RF24NetworkHeader headerL(base_node, 'L'); // Address where the data is going
    ok = network.write(headerL, &bpayload, sizeof(bpayload)); // send the data
    if (!ok) {
      //Serial.print(F("Retry sending message: "));
      //Serial.println(sendingCounter);      
      ok = network.write(headerL, &bpayload, sizeof(bpayload)); // retry once
    }

    Serial.print(currentmilli);
    Serial.print(F(" send message "));
    if (ok) {
      Serial.println(F("OK "));
    }
    else{
      Serial.println(F("Failed "));
    }
    //Serial.println(sendingCounter);
  }
  return traction;
}
