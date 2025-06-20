/*
 * RF networking, kitchen node, repeater for keypad, also receiving commands from home base
 */

#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

#define radio_channel 104

// RF Nano, USB-C
#define CE_PIN 10
#define CSN_PIN 9

RF24 radio(CE_PIN, CSN_PIN);     // internal nRF24L01 (CE, CSN)
RF24Network network(radio);      // include the radio in the network

const uint16_t kitchen_node = 02; // Address of node in Octal format
const uint16_t base_node = 00;    // Address of node in Octal format

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
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
  radio.setDataRate(RF24_1MBPS); // (RF24_2MBPS);

  network.begin(radio_channel, kitchen_node); // (channel, node address)
}

// Payload for kitchen (from base)
struct kitchen_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t light; // 0 - no change, 100 - ON, 200 - OFF
};

// Payload for base (from kitchen)
struct base_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t light;
  uint8_t pirs;
  uint8_t distance;
};

const uint32_t kitchenkeyword = 0x10112003;

//===== Receiving =====//
unsigned int receiveRFnetwork(unsigned long currentmilli){
  unsigned int reaction = 0;

  // Check for incoming data from the sensors
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      case 'K': // Message received from HomeController for RemoteNode
        Serial.print(F("Message received from Base: "));
        kitchen_payload kpayload;
        network.read(header, &kpayload, sizeof(kpayload));
        reaction = 0; // nothing
        if (kitchenkeyword == kpayload.keyword){ // valid message received               
          if (kpayload.light > 0){
            reaction = 111; // on
            if (kpayload.light > 199){
              reaction = 222; // off
            }
            Serial.print(F("Light received over RF: "));
            Serial.println(kpayload.light);
          }
        }
        else{
          Serial.print(F("Found unexpected keyword, expected: "));
          Serial.print(kitchenkeyword);
          Serial.print(F(", received keyword: "));
          Serial.println(kpayload.keyword);          
        }
      break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.print(header.type);
        reaction = 255; // unexpected message received
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
    base_payload bpayload;
    bpayload.keyword = kitchenkeyword;
    bpayload.count = 0;
    bpayload.light = 0;
    bpayload.timing = currentmilli;
    RF24NetworkHeader headerK(base_node, 'L'); // Address where the data is going
    ok = network.write(headerK, &bpayload, sizeof(bpayload)); // send the data
    if (!ok) {
      //Serial.print(F("Retry sending message: "));
      //Serial.println(sendingCounter);      
      ok = network.write(headerK, &bpayload, sizeof(bpayload)); // retry once
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
