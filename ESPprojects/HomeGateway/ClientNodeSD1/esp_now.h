
bool devicepaired = false;
uint8_t pairingheartbeat = 0;
char reftext[11] = "client_SD1";

enum MessageType { PAIRING, DATA, ACK, TEXT, HBEAT };
MessageType messageType;

uint8_t Server_Address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // this is at startup the broadcast address
uint8_t Client_Address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // this is at startup the broadcast address

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

void printMAC(const uint8_t * mac_addr) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

bool addPeer() { // add pairing
  esp_now_del_peer(Server_Address);
  int res = esp_now_add_peer(Server_Address, ESP_NOW_ROLE_COMBO, 4, NULL, 0);
  devicepaired = res == 0;
  Serial.println("PEER added ");
  return devicepaired;
}

// function to send 1 single ESP-NOW message
void sendonesp(u8 *data, int len) {
  // char macStr[18];
  // snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
  //          Server_Address[0], Server_Address[1], Server_Address[2], Server_Address[3], Server_Address[4], Server_Address[5]);
  // Serial.print(macStr);
  printMAC(Server_Address);
  esp_now_send(Server_Address, data, len);
}

// // function to check the heartbeat of the server
// void heartbeat(unsigned long curtime, bool message) {
//   static unsigned long htime = 0;
//   if (message) {
//     htime = curtime;
//   }
//   else {
//     if (devicepaired) {
//       if (htime + 60000 < curtime){ // if not received a message for over 60 seconds, consider pairing dropped
//         devicepaired = false;
//       }
//     }
//   }
// }

// Callback when data is received
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  static unsigned long rcount = 0;
  static uint8_t runningline = 0;
  bool resppairing = true;

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
  unsigned long messagetime = millis();
  Serial.println(messagetime);
  // heartbeat(messagetime, true);
  // String datahere(data);
  // Serial.println(datahere);

  uint8_t type = incomingData[0];      // first message byte is the type of message 
  switch (type) {
  case DATA:                           // the message is data type
    Serial.println(F("DATA"));

    Serial.println("processing TBD");

    break;
  case PAIRING:                         // the message is a pairing request 
    Serial.println(F("PAIRING"));

    memcpy(&pairingData, incomingData, sizeof(pairingData));
    // Serial.println(pairingData.msgType);
    Serial.print(pairingData.id);
    Serial.print(" Pairing request from MAC Address: ");
    printMAC(pairingData.ServermacAddr);
    Serial.print(", ");
    printMAC(pairingData.ClientmacAddr);
    Serial.print(" on channel ");
    Serial.println(pairingData.channel);

    switch(pairingData.id){
      case 1: // first reply message on pairing, reply with the Server Mac
        pairingData.id = 2;
        for ( int id = 0; id < 6; id++ ) {
          pairingData.ServermacAddr[id] = mac[id];
          Server_Address[id] = mac[id];
        }
        addPeer();
        memcpy(&pairingData.textref, reftext, 11);
        // for ( int id = 0; id < 11; id++ ){
        //   pairingData.textref[id] = reftext[id];
        // }        
      break;
      // case 3: // second reply message on pairing, reply with ?
      //   pairingData.id = 4;
      
      // break;
      default:
        resppairing = false;
    }
    if (resppairing) {
      sendonesp((uint8_t *)&pairingData, sizeof(pairingData));
    }

    break;
  case ACK:                             // the message is an acknowledge message 
    Serial.println(F("ACK"));

    break; 
  case TEXT:                           // the message is text type
    Serial.println(F("TEXT"));

    memcpy(&textingData, incomingData, sizeof(textingData));
    Serial.print(textingData.line);
    Serial.print(F(" "));
    Serial.println(textingData.texting);

    // updateDisplay();
    if (textingData.line < 4) {
      memcpy(&Lines[textingData.line], textingData.texting, 101);
      upddisplay = 90; // update display in the main loop
      runningline = 0; // reset static running line (if received from webserver form input)
    }
    if (textingData.line == 99) {
      runningline = runningline % 4;
      memcpy(&Lines[runningline++], textingData.texting, 101);
      upddisplay = 90; // update display in the main loop
    }
    if (textingData.line == 95) {
      for(int lin=0; lin < 4 ; lin++){
        Lines[lin][0] = '\0';
        //memset(Lines[lin], 0, 101);
      }
      upddisplay = 90; // update display in the main loop
      runningline = 0; // reset static running line (if received from webserver form input)
    }
    
    // reply with 'ack'
    textingData.texting[100] = '\0';
    textingData.texting[0] = '\0';
    sendonesp((uint8_t *)&textingData, sizeof(textingData));

    break;
  case HBEAT:                      // the message is heartbeat
    Serial.println(F("HBEAT"));
    pairingheartbeat = 30; // minutes, countdown
    break;
  default:
    Serial.print("Unknown message type: ");
    Serial.println(type);
  }

  if (!devicepaired) {
    for ( int id = 0; id < 6; id++ ) {
      Server_Address[id] = mac[id];
    }
    // Add broadcast peer (improves reliability)
    addPeer();
  }

  // add check if mac is Server_Address
  // send acknowledge message
//  sendonesp((uint8_t *)ackmsg, sizeof(ackmsg));
  //esp_now_send(mac, (uint8_t *)ackmsg, sizeof(ackmsg));
}

// Callback when data is sent
void onDataSent(uint8_t *mac_addr, uint8_t status) {
  static unsigned long scount = 0;
  scount += 1;
  Serial.print(", message: ");
  Serial.print(scount);
  Serial.print(", send status: ");
  Serial.print(status == 0 ? "Success" : "Fail");
  Serial.print(" at: ");
  Serial.println(millis());
}

void sendpairingsequence(uint8_t pstat) {
  pairingData.msgType = PAIRING;
  pairingData.id = pstat;
  for ( int id = 0; id < 6; id++ ) {
    pairingData.ServermacAddr[id] = Server_Address[id];
    pairingData.ClientmacAddr[id] = Client_Address[id];
  }
  pairingData.channel = 4;
  memcpy(&pairingData.textref, reftext, 11);
  // for ( int id = 0; id < 11; id++ ){
  //   pairingData.textref[id] = reftext[id];
  // }  
  sendonesp((uint8_t *)&pairingData, sizeof(pairingData));
}
