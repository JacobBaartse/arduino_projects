extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>


const int led = LED_BUILTIN;
const int buttonPin = D3; 
bool devicepaired = false;

enum MessageType {PAIRING, DATA, ACK};
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
} struct_pairing;

typedef struct struct_ack { // structure for acknowledge
    uint8_t msgType;
    uint8_t id;
} struct_ack;

struct_pairing pairingData;


void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

bool addPeer() {      // add pairing
  esp_now_del_peer(Server_Address);
  int res = esp_now_add_peer(Server_Address, ESP_NOW_ROLE_COMBO, 4, NULL, 0);
  devicepaired = res == 0;
  Serial.println("PEER added");
  return devicepaired;
}

// function to send 1 single ESP-NOW message
void sendonesp(u8 *data, int len){
  // char macStr[18];
  // snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
  //          Server_Address[0], Server_Address[1], Server_Address[2], Server_Address[3], Server_Address[4], Server_Address[5]);
  // Serial.print(macStr);
  printMAC(Server_Address);
  esp_now_send(Server_Address, data, len);
}

// Callback when data is received
void onDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  static unsigned long rcount = 0;
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
  Serial.println(millis());
  // String datahere(data);
  // Serial.println(datahere);

  uint8_t type = incomingData[0];       // first message byte is the type of message 
  switch (type) {
  case DATA :                           // the message is data type
    Serial.println("DATA");

    break;
  case PAIRING:                         // the message is a pairing request 
    Serial.println("PAIRING");

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
        for ( int id = 0; id < 6; id++ ){
          pairingData.ServermacAddr[id] = mac[id];
          Server_Address[id] = mac[id];
        }
        addPeer();
      break;
      // case 3: // second reply message on pairing, reply with ?
      //   pairingData.id = 4;
      
      // break;
      default:
        resppairing = false;
    }
    if (resppairing){
      sendonesp((uint8_t *)&pairingData, sizeof(pairingData));
    }

    break;
  case ACK:                             // the message is an acknowledge message 
    Serial.println("ACK");

    break; 
  default:
    Serial.print("Unknown message type: ");
    Serial.println(type);
  }

  if (!devicepaired) {
    if (false) {//if (strcmp(data,"HIERO") == 0) { // consider pairing found
      for ( int id = 0; id < 6; id++ ){
        Server_Address[id] = mac[id];
      }
      // Add broadcast peer (improves reliability)
      esp_now_add_peer(Server_Address, ESP_NOW_ROLE_COMBO, 4, NULL, 0);
      devicepaired = true;
    }
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

void sendpairingsequence(int pstat){
  static int seq = 0;

  pairingData.msgType = PAIRING;
  pairingData.id = seq;
  for ( int id = 0; id < 6; id++ ){
    pairingData.ServermacAddr[id] = Server_Address[id];
    pairingData.ClientmacAddr[id] = Client_Address[id];
  }
  pairingData.channel = 4;
  sendonesp((uint8_t *)&pairingData, sizeof(pairingData));
}

// function to indicate the passing of certain duration
bool timepassing(unsigned long curtime, unsigned long duration){
  static unsigned long rtime = 0;
  if(rtime + duration > curtime) return false;
  rtime = millis(); // get fresh time to base the new interval on
  return true;
}

// --------------------
// Setup
// --------------------
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0); // turn onboard LED on
  Serial.begin(115200);

  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.println(F("Starting ESP device"));
  Serial.flush();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  // ESP-NOW requires WiFi in STA mode
  WiFi.mode(WIFI_STA);
  wifi_promiscuous_enable(1);   // required to allow channel change
  wifi_set_channel(4);          // choose your channel (1–13)
  wifi_promiscuous_enable(0);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  // Set role and register callbacks
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  // use the button on an interrupt hadling
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPress, FALLING); // trigger when button pressed

  Serial.print(F("ESP-NOW channel 4, "));
  Serial.println(F("ESP-NOW Transceiver Ready"));
  digitalWrite(led, 1); // turn onboard LED off
}

const char buttonmsg[] = "Button pressed (BC1).";
unsigned long runningtime = 0;
bool action = false;
int actionid = 0;
bool buttonpressed = false;

void handle_button(bool pressed, unsigned long timing) {
  static unsigned long btime = 0;
  static bool buttonstate = false;
  bool bpress = pressed;

  if (buttonstate){
    int butstate = digitalRead(buttonPin); // check current status of the button
    if (butstate == LOW) {  // button still pressed within the time period
      btime = timing;
      // Serial.println(F("Button press extension"));
      return;
    }
    if (btime + 2000 < timing){
      buttonstate = false;
      Serial.print(F("Button can be pressed again "));
      Serial.println(millis());
      buttonpressed = false;
    }
    else {
      bpress = false;
    }
  }
  if (bpress) {
    buttonpressed = true;
    btime = millis();
    buttonstate = true;
    Serial.print(F("Button press: "));
    Serial.println(btime);
    sendonesp((uint8_t *)buttonmsg, sizeof(buttonmsg));
    //esp_now_send(Server_Address, (uint8_t *)buttonmsg, sizeof(buttonmsg));
  }
}

// --------------------
// Main Loop
// --------------------
void loop() {

  runningtime = millis();

  action = timepassing(runningtime, 9000);
  if (action){
    if (devicepaired){
      //sendonesp((uint8_t *)msg, sizeof(msg));
      //esp_now_send(Server_Address, (uint8_t *)msg, sizeof(msg));
    }
    else{
      sendpairingsequence(0);
      //sendonesp((uint8_t *)pairmsg, sizeof(pairmsg));
      //esp_now_send(Server_Address, (uint8_t *)pairmsg, sizeof(pairmsg));
    }
  }

  handle_button(false, runningtime);
  
}

ICACHE_RAM_ATTR void buttonPress(){
  // Serial.print(F("Button press: "));
  // Serial.println(millis());
  handle_button(true, millis());
}
