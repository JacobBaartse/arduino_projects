/*
 * RF networking, repeater from end-point to/from home base, also receiving commands from home base
 */

#define radio_channel 100

RF24 radio(9, 10);               // external nRF24L01 (CE, CSN)
RF24Network network(radio);      // include the radio in the network

const uint16_t remote_node = 01;   // Address of node in Octal format (04, 031, etc.)
const uint16_t repeater_node = 00; // Address of node in Octal format
const uint16_t base_node = 02;     // Address of node in Octal format

void setupRFnetwork(){
  SPI.begin();

  radio.begin();
  //radio.setPALevel(RF24_PA_MIN, false); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
  radio.setDataRate(RF24_1MBPS); // (RF24_2MBPS);

  network.begin(radio_channel, repeater_node); // (channel, node address)
}

// Payload from home and remote
struct repeat_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t value1;
  uint8_t value2;
  uint8_t value3;
};

// Payload for schuur
struct schuur_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t light; // 0 - no change, 100 - ON, 200 - OFF
};

repeat_payload forwards;
schuur_payload acks;
char repeat_type = 'P';

//===== Receiving =====//
unsigned int receiveRFnetwork(unsigned long currentmilli){
  unsigned int reaction = 0;

  // Check for incoming data from the sensors
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      case 'S': // Message received from HomeController for RemoteNode
        Serial.print(F("Message received from Home: "));
        repeat_payload hpayload;
        network.read(header, &hpayload, sizeof(hpayload));
        forwards = hpayload;
        repeat_type = 'T';
        reaction++;
      break;
      case 'H': // Message received from RemoteNode for HomeController 
        Serial.print(F("Message received from Remote: "));
        repeat_payload rpayload;
        network.read(header, &rpayload, sizeof(rpayload));
        forwards = rpayload;
        repeat_type = 'I';
        reaction++;
      break;
      case 'R': // Message received from HomeController for this device
        Serial.print(F("Message received for this device: "));
        schuur_payload lpayload;
        network.read(header, &lpayload, sizeof(lpayload));
        reaction++;
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
  uint16_t destination = base_node;

  // Every x seconds...
  if((fresh)||(currentmilli - sendingTimer > 5000)){
    sendingTimer = currentmilli;

    switch(repeat_type) {
      case 'I':
        destination = base_node;
      break;
      case 'T':
        destination = remote_node;
      break;
      default:
        destination = base_node;
        repeat_type = 'P';
    }

    RF24NetworkHeader headerT(destination, repeat_type); // Address where the data is going
    if (repeat_type != 'P'){ // forward the message
      ok = network.write(headerT, &forwards, sizeof(forwards)); // send the data
      if (!ok) {
        Serial.print(F("Retry sending message: "));
        //Serial.println(sendingCounter);      
        ok = network.write(headerT, &forwards, sizeof(forwards)); // retry once
      }
    }
    else { // acknowledge the message from controller/base node
      ok = network.write(headerT, &acks, sizeof(acks)); // send the data
      if (!ok) {
        Serial.print(F("Retry sending message: "));
        //Serial.println(sendingCounter);      
        ok = network.write(headerT, &acks, sizeof(acks)); // retry once
      }
    }
    Serial.print(currentmilli);
    Serial.print(F(" send message "));
    if (ok) {
      repeat_type = 'P';
    }
    else{
      Serial.print(F("Failed "));
    }
    //Serial.println(sendingCounter);
  }
  return traction;
}
