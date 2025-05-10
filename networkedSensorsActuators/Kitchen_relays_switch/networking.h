/*
 * RF networking, repeater from end-point to/from home base, also receiving commands from home base
 */

#define radio_channel 104

RF24 radio(9, 10);               // internal nRF24L01 (CE, CSN)
RF24Network network(radio);      // include the radio in the network

const uint16_t kitchen_node = 01; // Address of node in Octal format
const uint16_t base_node = 00;     // Address of node in Octal format

void setupRFnetwork(){
  SPI.begin();

  radio.begin();
  //radio.setPALevel(RF24_PA_MIN, false); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
  radio.setDataRate(RF24_1MBPS); // (RF24_2MBPS);

  network.begin(radio_channel, kitchen_node); // (channel, node address)
}

// Payload for kitchen
struct kitchen_payload{
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
      case 'K': // Message received from HomeController for RemoteNode
        Serial.print(F("Message received from Base: "));
        kitchen_payload kpayload;
        network.read(header, &kpayload, sizeof(kpayload));
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
    kitchen_payload kpayload;
    kpayload = {0, currentmilli, 0, 0};
    RF24NetworkHeader headerK(base_node, 'K'); // Address where the data is going
    ok = network.write(headerK, &kpayload, sizeof(kpayload)); // send the data
    if (!ok) {
      //Serial.print(F("Retry sending message: "));
      //Serial.println(sendingCounter);      
      ok = network.write(headerK, &kpayload, sizeof(kpayload)); // retry once
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
