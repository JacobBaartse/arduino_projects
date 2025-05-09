/*
 * 
 */

RF24 radio(9, 10);               // onboard nRF24L01 (CE, CSN)
RF24Network network(radio);      // Include the radio in the network

#define radio_channel 100;

const uint16_t remote_node = 01;   // Address of node in Octal format (04, 031, etc.)
const uint16_t repeater_node = 00;    // Address of node in Octal format
const uint16_t base_node = 02;    // Address of node in Octal format

void setupRFnetwork(){
  SPI.begin();

  radio.begin();
  //radio.setPALevel(RF24_PA_MIN, false); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
  radio.setDataRate(RF24_1MBPS); // (RF24_2MBPS);

  network.begin(radio_channel, repeater_node); // (channel, node address)
}

//===== Receiving =====//
unsigned int receiveRFnetwork(unsigned long currentmilli){
  unsigned int reaction = 0;

  while (network.available()) { // Is there any incoming data?
    Serial.println(F("Receiving on RF network"));
    RF24NetworkHeader header;
    network_payload incomingData;
    network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    Serial.println(incomingData.keyword, HEX);
    if (header.from_node != node01) {
      Serial.print(F("Received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    receivedmsg++;
    // Serial.print(F("incomingData: "));
    // Serial.println(incomingData.counter);
    // check keyword and sequencenumber
    if (incomingData.keyword == keywordval){
      receiveCounter = incomingData.counter;
      responsefromremote = incomingData.response;
      if (rcvmsgcount > wrappingcounter) { // initialisation
        rcvmsgcount = receiveCounter;
        commanding = command_clear_counters;
      }
      else { // check received message value
        if (rcvmsgcount != receiveCounter) {
          if (receivedmsg > 1) {
             droppedmsg++; // this could be multiple as well
            Serial.print(F("Missed network message(s): "));
            Serial.print(F("received id: "));
            Serial.print(receiveCounter);
            Serial.print(F(", expected id: "));
            Serial.println(rcvmsgcount);             
          }
          rcvmsgcount = receiveCounter; // re-synchronize
        }
      }
      rcvmsgcount = updatecounter(rcvmsgcount); // calculate next expected message 
    }
    else{
      Serial.println(F("Keyword failure"));
    }

    if (responsefromremote > response_none) {
      // Serial.print(F("responsefromremote: "));
      // Serial.println(responsefromremote, HEX);
      unsigned long fails = 1;//incomingData.data2 & 0xffff;
      unsigned long drops = 2;//(incomingData.data2 >> 16) & 0xffff;
      unsigned long rsend = incomingData.data1;
      unsigned long rcoll = incomingData.response;
      Serial.print(F("Remote network messages "));
      Serial.print(F("received: "));
      Serial.print(rcoll);
      Serial.print(F(", send: "));
      Serial.print(rsend);
      Serial.print(F(", missed: "));
      Serial.print(drops);
      Serial.print(F(", failed: "));
      Serial.println(fails);
      Serial.println(F("-"));  
      responsefromremote = response_none;
    }

    network.update();
  }
  return reaction;
}


//===== Sending =====//
unsigned int transmitRFnetwork(unsigned long currentmilli){
  static unsigned long sendingTimer = 0;
  unsigned int traction = 0;

  // Every x seconds...
  unsigned long currentmilli = millis();
  if(currentmilli - sendingTimer > 5000) {
    sendingTimer = currentmilli;
    sendingCounter = updatecounter(sendingCounter); 
    RF24NetworkHeader header1(node01, 'B'); // Address where the data is going
    network_payload outgoing = {keywordval, sendingCounter, currentmilli, commandtx, responding, data1};//, data2, data3};

    //network.update();

    bool ok = network.write(header1, &outgoing, sizeof(outgoing)); // Send the data
    if (!ok) {
      Serial.print(F("Retry sending message: "));
      Serial.println(sendingCounter);      
      ok = network.write(header1, &outgoing, sizeof(outgoing)); // retry once
    }
    if (ok) {
      sendmsg++;
      commanding = command_none;
    }
    else{
      Serial.print(F("Error sending message: "));
      Serial.println(sendingCounter);
      failedmsg++;
    }
    responding = response_none;
  }
  return traction;
}

