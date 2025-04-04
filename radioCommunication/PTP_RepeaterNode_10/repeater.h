/*
 *
 *
 */

// max 32 bytes can be in the FIFO of the nRF24L01
// that means maximum 8 data items of unsigned long
struct net_payload {
  unsigned long data1;
  unsigned long data2;
  unsigned long data3;
  unsigned long data4;
  unsigned long data5;
  unsigned long data6;  
  //unsigned long data7;
  //unsigned long data8;
};

net_payload EmptyData = {0, 0, 0, 0, 0, 0};//, 0, 0};
net_payload DataForNW1 = {0xab000001, 1, 1, 1, 1, 1};//, 1, 1};
net_payload DataForNW2 = {0xab000002, 2, 2, 2, 2, 2};//, 2, 2};

//===== Receiving Radio =====//
net_payload receiveRFnetwork(RF24Network netw, uint16_t from_node, uint8_t id){
  net_payload returnData = EmptyData;

  netw.update();
  
  while (netw.available()) { // Is there any incoming data?
    Serial.print(F("Receiving on network"));
    Serial.println(id);
    RF24NetworkHeader header;
    net_payload incomingData;
    netw.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    Serial.println(incomingData.data1, HEX);
    if (header.from_node != from_node) {
      Serial.print(F("Received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    else{
      Serial.print(F("Data received from network"));
      Serial.println(id);
      returnData = incomingData;
    }
  }
  return returnData;
}

char mtypeval[3] = {"012"};

//===== Sending radio =====//
bool transmitRFnetwork(RF24Network netw, uint16_t to_node, net_payload senddata, uint8_t id){
  bool ok = false;

  //netw.update();
  
  RF24NetworkHeader header(to_node, mtypeval[id]); // Address where the data is going
  ok = netw.write(header, &senddata, sizeof(senddata)); // Send the data
  if (ok) {
    Serial.print(F("Send nw: "));
    Serial.println(id);  
  }
  else {
    // Serial.print(F("Retry send nw: "));
    // Serial.println(id);
    ok = netw.write(header, &senddata, sizeof(senddata)); // retry once  
  }
  if (!ok) {
    Serial.print(F("Error send nw: "));
    Serial.println(id);
  }
  return ok;
}

//===== check activity, heartbeat =====//
bool activitytracker(uint8_t timingseconds=20){
  static unsigned long beattime = 0;
  bool timing = false;
  
  if (((millis() - beattime)/1000) > timingseconds){
    beattime = millis();
    timing = true;
    Serial.print(F("Running check: "));
    Serial.println((beattime/1000));
  }
  return timing;
}