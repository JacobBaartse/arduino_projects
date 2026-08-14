
//bool devicepaired = false;

enum MessageType { PAIRING, DATA, ACK, TEXT, HBEAT };
MessageType messageType;

uint8_t connectedclients[20][6] = { // 20 clients can be connected using ESP-NOW
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
}; 

char referencestring[21][11] = { // 10 charactors + terminator, for 20 clients and the server
  "Ref 0",
  "Ref 1",
  "Ref 2",
  "Ref 3",
  "Ref 4",
  "Ref 5",
  "Ref 6",
  "Ref 7",
  "Ref 8",
  "Ref 9",
  "Ref 10",
  "Ref 11",
  "Ref 12",
  "Ref 13",
  "Ref 14",
  "Ref 15",
  "Ref 16",
  "Ref 17",
  "Ref 18",
  "Ref 19",
  "Ref Server"
};

uint8_t connectedclientcount = 0;

uint8_t Server_Address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // this is at startup the broadcast address
uint8_t Broadcast_Address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // used to find free space in the connected clients array
bool serverknown = false;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  uint8_t msgType;
  uint8_t id;
  float temp;
  float hum;
  unsigned int readingId;
} struct_message;

typedef struct struct_pairing { // structure for pairing
  uint8_t msgType;
  uint8_t id;
  uint8_t ServermacAddr[6];
  uint8_t ClientmacAddr[6];
  uint8_t channel;
  char textref[11];
} struct_pairing;

typedef struct struct_ack { // structure for acknowledge
  uint8_t msgType;
  uint8_t id;
} struct_ack;

struct_pairing pairingData;

typedef struct struct_string { // structure for text
  uint8_t msgType;
  uint8_t id;
  uint8_t line;
  char texting[101]; // 100 characters + terminator char
} struct_string;

struct_string textingData;

typedef struct struct_heartbeat { // structure for text
  uint8_t msgType;
  uint8_t id;
  unsigned long timestamp;
} struct_heartbeat;

struct_heartbeat heartbeatData;

uint8_t textackcount = 0;
char forminput[101] = {'\0'};
uint8_t textfromform = 0;
uint8_t textforclient = 99;

void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

int indexMAC(const uint8_t * mac_addr){
  int index = 99;
  int foundcount = 0;

  for ( int row = 0; row < 20; row++ ){
    foundcount = 0; 
    for ( int id = 0; id < 6; id++ ){
      if (mac_addr[id] == connectedclients[row][id]){
        foundcount += 1;
      }
    }
    if (foundcount == 6){
      index = row;
      break; // found the index
    }
  }
  Serial.print(F("indexMAC "));
  printMAC(mac_addr);
  Serial.print(F(": "));
  Serial.println(index); // index 99 means not found
  return index;
}

int getindexMAC(const uint8_t * mac_addr){
  int macindex = indexMAC(mac_addr);
  if (macindex > 20){ // 99 means MAC not found
    macindex = indexMAC(Broadcast_Address); // this should be 0..19, all 0xFF
    for ( int id = 0; id < 6; id++ ){
      connectedclients[macindex][id] = mac_addr[id];
    }
    connectedclientcount = macindex + 1;
  }
  return macindex;
}

void addPeer(uint8_t *peer_addr){ // add pairing
  esp_now_del_peer(peer_addr); // making sure no duplicates will be introduced
  int res = esp_now_add_peer(peer_addr, ESP_NOW_ROLE_COMBO, 4, NULL, 0);
  if (res == 0){
    Serial.println("ESP-NOW PEER added");
  }
}

// // function to send 1 single ESP-NOW message
// void sendonesp(u8 *data, int len){
//   // char macStr[18];
//   // snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
//   //          Server_Address[0], Server_Address[1], Server_Address[2], Server_Address[3], Server_Address[4], Server_Address[5]);
//   // Serial.print(macStr);
//   printMAC(Server_Address);
//   esp_now_send(Server_Address, data, len);
// }

// function to send 1 single ESP-NOW message
void sendonesp(u8 *da, u8 *data, int len){
  // char macStr[18];
  // snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
  //          da[0], da[1], da[2], da[3], da[4], da[5]);
  // Serial.print(macStr);
  printMAC(da);
  esp_now_send(da, data, len);
}

// // function to check the heartbeat of the server
// void heartbeat(unsigned long curtime, bool message){
//   static unsigned long htime = 0;
//   if (message){
//     htime = curtime;
//   }
//   else {
//     if (devicepaired){
//       if (htime + 60000 < curtime){ // if not received a message for over 60 seconds, consider pairing dropped
//         devicepaired = false;
//       }
//     }
//   }
// }

// next function only for clients I guess 
// void sendpairingsequence(uint8_t pstat){
//   pairingData.msgType = PAIRING;
//   pairingData.id = pstat;
//   for ( int id = 0; id < 6; id++ ){
//     pairingData.ServermacAddr[id] = Server_Address[id];
//     pairingData.ClientmacAddr[id] = Client_Address[id];
//   }
//   pairingData.channel = 4;
//   memcpy(&pairingData.textref, reftext, 11);
//   // for ( int id = 0; id < 11; id++ ){
//   //   pairingData.textref[id] = reftext[id];
//   // }  
//   sendonesp((uint8_t *)&pairingData, sizeof(pairingData));
// }


// // --------------------
// // ESP-NOW Receive Callback
// // --------------------
// // void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
// //   static unsigned long rcount = 0;
// //   rcount += 1;
// //   Serial.print("ESP-NOW Received ");
// //   Serial.print(rcount);
// //   Serial.print(" from ");
// //   char macStr[18];
// //   snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
// //            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
// //   Serial.print(macStr);
// //   Serial.print(" | Data: ");
// //   Serial.write(data, len - 1);
// //   Serial.print(" at: ");
// //   Serial.println(millis());
// // }

// // Callback when data is received
// void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len){
//   static unsigned long rcount = 0;
//   static uint8_t runningline = 0;
//   bool resppairing = true;

//   rcount += 1;  
//   Serial.print("ESP-NOW Received ");
//   Serial.print(rcount);
//   Serial.print(" from ");
//   // char macStr[18];
//   // snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
//   //          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//   // Serial.print(macStr);
//   printMAC(mac);
//   // Serial.print(" | Data: ");
//   // Serial.write(incomingData, len - 1);
//   Serial.print(" at: ");
//   unsigned long messagetime = millis();
//   Serial.println(messagetime);
//   heartbeat(messagetime, true);
//   // String datahere(data);
//   // Serial.println(datahere);

//   uint8_t type = incomingData[0];       // first message byte is the type of message 
//   switch (type) {
//   case DATA:                           // the message is data type
//     Serial.println("DATA");

//     break;
//   case PAIRING:                         // the message is a pairing request 
//     Serial.println("PAIRING");

//     memcpy(&pairingData, incomingData, sizeof(pairingData));
//     // Serial.println(pairingData.msgType);
//     Serial.print(pairingData.id);
//     Serial.print(" Pairing request from MAC Address: ");
//     printMAC(pairingData.ServermacAddr);
//     Serial.print(", ");
//     printMAC(pairingData.ClientmacAddr);
//     Serial.print(" on channel ");
//     Serial.println(pairingData.channel);

//     switch(pairingData.id){
//       case 1: // first reply message on pairing, reply with the Server Mac
//         pairingData.id = 2;
//         for ( int id = 0; id < 6; id++ ){
//           pairingData.ServermacAddr[id] = mac[id];
//           Server_Address[id] = mac[id];
//         }
//         addPeer();
//         memcpy(&pairingData.textref, reftext, 11);
//         // for ( int id = 0; id < 11; id++ ){
//         //   pairingData.textref[id] = reftext[id];
//         // }        
//       break;
//       // case 3: // second reply message on pairing, reply with ?
//       //   pairingData.id = 4;
      
//       // break;
//       default:
//         resppairing = false;
//     }
//     if (resppairing){
//       sendonesp((uint8_t *)&pairingData, sizeof(pairingData));
//     }

//     break;
//   case ACK:                             // the message is an acknowledge message 
//     Serial.println("ACK");

//     break; 
//   case TEXT:                           // the message is text type
//     Serial.println("TEXT");

//     memcpy(&textingData, incomingData, sizeof(textingData));
//     Serial.print(textingData.line);
//     Serial.print(F(" "));
//     Serial.println(textingData.texting);

//     // updateDisplay();
//     if (textingData.line < 4){
//       memcpy(&Lines[textingData.line], textingData.texting, 101);
//       upddisplay = 90; // update display in the main loop
//       runningline = 0; // reset static running line (if received from webserver form input)
//     }
//     if (textingData.line == 99){
//       runningline = runningline % 4;
//       memcpy(&Lines[runningline++], textingData.texting, 101);
//       upddisplay = 90; // update display in the main loop
//     }
//     if (textingData.line == 95){
//       for(int lin=0; lin < 4 ; lin++){
//         Lines[lin][0] = '\0';
//         //memset(Lines[lin], 0, 101);
//       }
//       upddisplay = 90; // update display in the main loop
//       runningline = 0; // reset static running line (if received from webserver form input)
//     }
    
//     // reply with 'ack'
//     textingData.texting[100] = '\0';
//     textingData.texting[0] = '\0';
//     sendonesp((uint8_t *)&textingData, sizeof(textingData));

//     break;
//   default:
//     Serial.print("Unknown message type: ");
//     Serial.println(type);
//   }

//   if (!devicepaired) {
//     for ( int id = 0; id < 6; id++ ){
//       Server_Address[id] = mac[id];
//     }
//     // Add broadcast peer (improves reliability)
//     addPeer();
//   }

//   // add check if mac is Server_Address
//   // send acknowledge message
// //  sendonesp((uint8_t *)ackmsg, sizeof(ackmsg));
//   //esp_now_send(mac, (uint8_t *)ackmsg, sizeof(ackmsg));
// }

// // --------------------
// // ESP-NOW Send Callback
// // --------------------
// // Callback when data is sent
// void onDataSent(uint8_t *mac_addr, uint8_t status) {
//   static unsigned long scount = 0;
//   scount += 1;
//   Serial.print("ESP-NOW Send Status ");
//   Serial.print(scount);
//   Serial.print(": ");
//   Serial.print(status == 0 ? "Success" : "Fail");
//   Serial.print(" at: ");
//   Serial.println(millis());
// }

// --------------------
// ESP-NOW Receive Callback
// --------------------
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  static unsigned long rcount = 0;
  bool resppairing = true;
  int deviceidx = 0;

  rcount += 1;
  Serial.print("ESP-NOW Received ");
  Serial.print(rcount);
  Serial.print(" from ");
  // char macStr[18];
  // snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
  //          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  // Serial.print(macStr);
  printMAC(mac);

  // Serial.print(" | Data: ");
  // Serial.write(incomingData, len - 1);
  Serial.print(" at: ");
  Serial.println(millis());

  uint8_t type = incomingData[0];       // first message byte is the type of message 
  switch (type) {
  case DATA:                           // the message is data type
    Serial.println("DATA");

    Serial.println("processing TBD");

    // memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    // // create a JSON document with received data and send it by event to the web page
    // root["id"] = incomingReadings.id;
    // root["temperature"] = incomingReadings.temp;
    // root["humidity"] = incomingReadings.hum;
    // root["readingId"] = String(incomingReadings.readingId);
    // serializeJson(root, payload);
    // Serial.print("event send :");
    // serializeJson(root, Serial);
    // events.send(payload.c_str(), "new_readings", millis());
    // Serial.println();
    break;

  case PAIRING:                            // the message is a pairing request 
    Serial.println("PAIRING");

    memcpy(&pairingData, incomingData, sizeof(pairingData));
    Serial.print(pairingData.id);
    Serial.print(" Pairing request from MAC Address: ");
    printMAC(pairingData.ServermacAddr);
    Serial.print(", ");
    printMAC(pairingData.ClientmacAddr);
    Serial.print(" on channel ");
    Serial.println(pairingData.channel);

    switch(pairingData.id){
      case 0: // first message on pairing, reply with the Client Mac
        pairingData.id = 1;
        for ( int id = 0; id < 6; id++ ) {
          pairingData.ClientmacAddr[id] = mac[id];
        }
        addPeer(mac);
        if (serverknown) {
          for ( int id = 0; id < 6; id++ ) {
            pairingData.ServermacAddr[id] = Server_Address[id];
          }
        }
        deviceidx = getindexMAC(mac); // add to connected clients list
        for ( int id = 0; id < 11; id++ ) {
          referencestring[deviceidx][id] = pairingData.textref[id];
        }        
        Serial.print(F("Device pairing reference: "));
        Serial.println(referencestring[deviceidx]);
      break;
      case 2: // second message on pairing, capture server MAC (if not already known)
        pairingData.id = 3;
        if (!serverknown) {
          for ( int id = 0; id < 6; id++ ) {
            Server_Address[id] = pairingData.ServermacAddr[id];
          }
          serverknown = true;
        }
        resppairing = false;
      break;
      default:
        resppairing = false;
    }
    if (resppairing) {
      sendonesp(mac, (uint8_t *)&pairingData, sizeof(pairingData));
    }

    // if (pairingData.id > 0) {     // do not replay to server itself
    //   if (pairingData.msgType == PAIRING) { 
    //     pairingData.id = 0;       // 0 is server
    //     // Server is in AP_STA mode: peers need to send data to server soft AP MAC address 
    //     //WiFi.softAPmacAddress(pairingData.macAddr);
    //     Serial.print("Pairing MAC Address: ");
    //     printMAC(clientMacAddress);
    //     pairingData.channel = 4;
    //     Serial.println(" send response");
    //     //esp_err_t result = esp_now_send(clientMacAddress, (uint8_t *) &pairingData, sizeof(pairingData));
    //     addPeer(clientMacAddress);
    //   }  
    // }  

    break; 
  case ACK:                            // the message is an acknowledge message 
    Serial.println("ACK");

    break; 
  case TEXT:                           // the message is text type
    Serial.println("TEXT");
    textackcount += 1;
    break;
  case HBEAT:                      // the message is heartbeat
    Serial.println("HBEAT message unexpected to gateway");
    break;
  default:
    Serial.print("ERROR: Unknown message type: ");
    Serial.println(type);
  }
}

unsigned long datasendtime = 0;
// Callback when data is sent
void onDataSent(uint8_t *mac_addr, uint8_t status) {
  static unsigned long scount = 0;
  scount += 1;
  datasendtime = millis();
  Serial.print(", message: ");
  Serial.print(scount);
  Serial.print(", send status: ");
  Serial.print(status == 0 ? "Success" : "Fail");
  Serial.print(" at: ");
  Serial.println(datasendtime);
}

char rmsg[101];
void randomstringvalue(int numBytes) {
  memset(rmsg, 0, sizeof(rmsg));
  for (int i = 0; i < numBytes; i++) {
    int randomValue = random(0, 36);
    rmsg[i] = randomValue + 'a';
    if (randomValue > 25) {
      rmsg[i] = (randomValue - 26) + '0';
    }
    else { // some change on a capital letter
      int capitalValue = random(0, 101);
      if (capitalValue < 30){
        rmsg[i] = randomValue + 'A';
      }
    }
  }
  // rmsg[numBytes] = '\0'; // bytes are cleared at the start
}

int nextheartbeatclient(int tindex) {
  int foundcount = 0;
  if (tindex > 20) { // 99 means MAC not found
    tindex = 0;
  }
  else tindex++;
  for ( int row = 0; row < 20; row++ ){
    foundcount = 0; 
    for ( int id = 0; id < 6; id++ ){
      if (connectedclients[tindex][id] == 0xFF){
        ++foundcount;
      }
    }
    if (foundcount < 6){
      break; // found the index
    }
    tindex = ++tindex % 20;
  }
  if (foundcount == 6) { // not found a valid client
    tindex = 99; 
  }
  // Serial.print(foundcount);
  // Serial.print(" bytes found at index: ");
  // Serial.println(tindex);
  return tindex;
}

void beating() { // send heartbeat
  static uint8_t hbcounter = 0;
  static int findex = 99;
  uint8_t clientMAC[6];

  findex = nextheartbeatclient(findex);
  if (findex < 20) {
    Serial.print(F("heartbeatclient at index: "));
    Serial.println(findex);
    for ( int id = 0; id < 6; id++ ){
      clientMAC[id] = connectedclients[findex][id];
    }
    heartbeatData.msgType = HBEAT;
    heartbeatData.id = hbcounter++;
    heartbeatData.timestamp = millis();
    sendonesp(clientMAC, (uint8_t *)&heartbeatData, sizeof(heartbeatData));
  }
  else {
    Serial.println(F("No clients found to send heartbeat"));
  }
}

void send_display_line(uint8_t *clientMAC, uint8_t linenum, const char* lineText) {
  static uint8_t tcounter = 0;
  textingData.msgType = TEXT;
  textingData.id = tcounter++;
  textingData.line = linenum;

  Serial.print(F(" "));
  Serial.print(lineText);
  Serial.println(F(" "));
  // memset(&rmsg, 0, sizeof(rmsg));
  // strcpy(lineText, textingData.texting);
  memcpy(&textingData.texting, lineText, sizeof(textingData.texting));
  sendonesp(clientMAC, (uint8_t *)&textingData, sizeof(textingData));
}
