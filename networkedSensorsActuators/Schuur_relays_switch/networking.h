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

RF24 radio(CE_PIN, CSN_PIN);     // internal nRF24L01 (CE, CSN)
RF24Network network(radio);      // include the radio in the network

const uint16_t shed_node = 01; // Address of node in Octal format
const uint16_t base_node = 00; // Address of node in Octal format

void setupRFnetwork(){
  SPI.begin();

  Serial.print(F("CE_PIN: "));
  Serial.print(CE_PIN);
  Serial.print(F(", CSN_PIN: "));
  Serial.println(CSN_PIN);

  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  //radio.setPALevel(RF24_PA_MIN, false); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_LOW); // testing, RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
  //radio.setPALevel(RF24_PA_MAX); // when in use
  radio.setDataRate(RF24_1MBPS); // (RF24_2MBPS);

  network.begin(radio_channel, shed_node); // (channel, node address)
}

// Payload for shed (from base)
struct shed_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t light; // 0 - no change, 100 - ON, 200 - OFF
};

// Payload for base (from shed)
struct base_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t light;
  uint8_t pirs;
  uint8_t distance;
};

const uint32_t shedkeyword = 0xffddeecc;

//===== Receiving =====//
unsigned int receiveRFnetwork(unsigned long currentmilli){
  static unsigned long mesreceived = 0;
  unsigned int reaction = 9; // no message received

  // Check for incoming data from the sensors
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      case 'S': // Message received from HomeController for RemoteNode
        Serial.print(F("Message received from Base: "));
        Serial.println(++mesreceived);
        shed_payload spayload;
        network.read(header, &spayload, sizeof(spayload));
        reaction = 0; // nothing
        if (shedkeyword == spayload.keyword){ // valid message received               
          if (spayload.light > 0){
            reaction = 111; // on
            if (spayload.light > 199){
              reaction = 222; // off
            }
          }
        }
        else{
          Serial.print(F("Found unexpected keyword, expected: "));
          Serial.print(shedkeyword);
          Serial.print(F(", received keyword: "));
          Serial.println(spayload.keyword);          
        }
      break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("timing: "));
        Serial.print(currentmilli);
        Serial.print(F(", TBD header.type: "));
        Serial.print(header.type);
        reaction = 255; // unexpected message received
        mesreceived = 0;
    }
    //Serial.println(currentmilli);
  } // end of while network.available

  return reaction;
}

//===== Sending =====//
unsigned int transmitRFnetwork(unsigned long currentmilli, bool fresh){
  static unsigned long messend = 0;
  static unsigned long failCounter = 0;
  //static unsigned long retryCounter = 0;
  static unsigned long sendingTimer = 0;
  unsigned int traction = 9; // no message send
  bool ok = false;

  // Every x seconds...
  if((fresh)||(currentmilli > sendingTimer)){
    sendingTimer = currentmilli + 5000;
    traction = 255; // sending failed
    base_payload bpayload;
    bpayload.keyword = shedkeyword;
    bpayload.count = 0;
    bpayload.light = 0;
    bpayload.pirs = 0;
    bpayload.distance = 0;
    bpayload.timing = currentmilli;
    RF24NetworkHeader headerL(base_node, 'B'); // Address where the data is going
    ok = network.write(headerL, &bpayload, sizeof(bpayload)); // send the data
    if (!ok) {
      //Serial.print(F("Retry sending message: "));
      //Serial.println(sendingCounter);      
      ok = network.write(headerL, &bpayload, sizeof(bpayload)); // retry once
    }

    Serial.print(currentmilli);
    Serial.print(F(" send message "));
    if (ok) {
      Serial.print(F("OK "));
      Serial.println(++messend);
      traction = 0; // send message OK
      failCounter = 0;
    }
    else{
      Serial.print(F("Failed "));
      Serial.println(++failCounter);
      messend = 0;
    }
    //Serial.println(sendingCounter);
  }
  return traction;
}
