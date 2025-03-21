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
  unsigned long data7;
  unsigned long data8;
};

net_payload EmptyData = {0, 0, 0, 0, 0, 0, 0, 0};

//===== Receiving Radio =====//
net_payload receiveRFnetwork(RF24Network netw, uint16_t from_node, uint8_t id){
  net_payload incomingData = EmptyData;

  while (netw.available()) { // Is there any incoming data?
    RF24NetworkHeader header;
    netw.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    if (header.from_node != from_node) {
      incomingData = EmptyData;
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    else{
      Serial.print(F("Data received from network"));
      Serial.println(id);
    }
  }
  return incomingData;
}

//===== Sending radio =====//
bool transmitRFnetwork(RF24Network netw, uint16_t to_node, net_payload senddata){
  bool ok = false;

  RF24NetworkHeader header(to_node); // Address where the data is going
  ok = netw.write(header, &senddata, sizeof(senddata)); // Send the data
  if (!ok) {
    Serial.println(F("Retry sending message"));
    ok = netw.write(header, &senddata, sizeof(senddata)); // retry once
  }
  if (!ok) {
    Serial.print(F("Error sending message to: "));
    Serial.println(to_node);
  }
  return ok;
}

//===== check activity, heartbeat =====//
void activitytracker(uint8_t timingseconds=20){
  static unsigned long beattime = 0;
  if (((millis() - beattime)/1000) > timingseconds){
    beattime = millis();
    Serial.print(F("Running check: "));
    Serial.println((beattime/1000));
  }
}