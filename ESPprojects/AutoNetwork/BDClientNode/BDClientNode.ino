extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h> 
//#include "FreeSerif12pt7b_special.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include "font_16pix_high.h"
#include <Adafruit_SH110X.h> // Adafruit SH110X by Adafruit

// enum DisplayState {
//     Off = 0,
//     Dim = 1,
//     On = 2,
// };

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3C //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3D //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define OLED_RESET -1

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// bool displaystatus = DisplayState::Off;
// void display_oled(bool clear, int x, int y, String text) {
//   if (displaystatus == DisplayState::Off) return;
//   if (clear) display.clearDisplay();
//   display.setCursor(x, y);
//   display.print(text);
//   display.display();
// }

// void clear_display(){
//   display.clearDisplay();
//   display.display();
// }

// char Lines[3][101] = {
//   "Welcome a",
//   "Demo B",
//   "Whats c up?"
// };
// uint8_t LinesYPos[3] = { 0, 21, 42 };
char Lines[4][101] = {
  "Welcome George",
  "Demo {big disp.}", 
  "Whats up?",
  "Hello World"
};  
uint8_t LinesYPos[4] = { 16, 32, 48, 64 };
uint8_t upddisplay = 200;

const char reftext[11] = "client_BD";
const int led = LED_BUILTIN;
const int buttonPin = D3; 
bool devicepaired = false;

enum MessageType { PAIRING, DATA, ACK, TEXT };
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
  Serial.println("PEER added ");
  return devicepaired;
}

void updateDisplay(){
  //bool fresh = true;
  //Serial.println("updateDisplay 1 ");
  //delay(1000);
  display.clearDisplay();
  // for(int lin=0; lin < 4 ; lin++){
  //   Serial.println(Lines[lin]);
  // }
  //Serial.println("updateDisplay 2 ");
  //delay(5000);
  //for(int lin=0; lin < 3 ; lin++){
  for(int lin=0; lin < 4 ; lin++){
    display.setCursor(0, LinesYPos[lin]);
    display.print(Lines[lin]);
  }
  display.display();
  //Serial.println("updateDisplay 3 ");
  upddisplay = 0;
  // display_oled(false, 0, 32, Lines[1]); 
  // display_oled(false, 0, 48, Lines[2]);  
  // display_oled(false, 0, 64, Lines[3]); 
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

// function to check the heartbeat of the server
void heartbeat(unsigned long curtime, bool message){
  static unsigned long htime = 0;
  if (message){
    htime = curtime;
  }
  else {
    if (devicepaired){
      if (htime + 60000 < curtime){ // if not received a message for over 60 seconds, consider pairing dropped
        devicepaired = false;
      }
    }
  }
}

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
  heartbeat(messagetime, true);
  // String datahere(data);
  // Serial.println(datahere);

  uint8_t type = incomingData[0];       // first message byte is the type of message 
  switch (type) {
  case DATA:                           // the message is data type
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
  case TEXT:                           // the message is text type
    Serial.println("TEXT");

    memcpy(&textingData, incomingData, sizeof(textingData));
    Serial.print(textingData.line);
    Serial.print(F(" "));
    Serial.println(textingData.texting);

    // updateDisplay();
    //if (textingData.line < 3){
    if (textingData.line < 4){
      memcpy(&Lines[textingData.line], textingData.texting, 101);
      upddisplay = 90; // update display in the main loop
      runningline = 0; // reset static running line (if received from webserver form input)
    }
    if (textingData.line == 99){
      runningline = runningline % 3;
      memcpy(&Lines[runningline++], textingData.texting, 101);
      upddisplay = 90; // update display in the main loop
    }
    if (textingData.line == 95){
      //for(int lin=0; lin < 3 ; lin++){
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
  Serial.println(reftext);
  Serial.flush(); 

  //Wire.begin();
  display.begin(i2c_Address, true); // Address 0x3C default
  display.oled_command(SH110X_DISPLAYON);
  display.setContrast(0); // dim display
  //displaystatus = DisplayState::Dim;
  display.clearDisplay();
  //display.setFont(&FreeSerif12pt7b);
  display.setFont(&font_16_pix);
  //display.setTextSize(2); // 3 lines of 10-12 chars
  display.setTextSize(1); 
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);
  display.display();

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
  //attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPress, FALLING); // trigger when button pressed

  // display_oled(true, 0, 0, Line1); //16
  // display_oled(false, 2, 21, Line2); //38
  // display_oled(false, 4, 42, Line3); //60
  updateDisplay();

  Serial.print(F("ESP-NOW channel 4, "));
  Serial.println(F("ESP-NOW Transceiver Ready"));
  digitalWrite(led, 1); // turn onboard LED off
}

const char buttonmsg[] = "Button pressed (BC1).";
unsigned long runningtime = 0;
bool action = false;
int actionid = 0;
// bool buttonpressed = false;

// void handle_button(bool pressed, unsigned long timing) {
//   static unsigned long btime = 0;
//   static bool buttonstate = false;
//   bool bpress = pressed;

//   if (buttonstate){
//     int butstate = digitalRead(buttonPin); // check current status of the button
//     if (butstate == LOW) {  // button still pressed within the time period
//       btime = timing;
//       // Serial.println(F("Button press extension"));
//       return;
//     }
//     if (btime + 2000 < timing){
//       buttonstate = false;
//       Serial.print(F("Button can be pressed again "));
//       Serial.println(millis());
//       buttonpressed = false;
//     }
//     else {
//       bpress = false;
//     }
//   }
//   if (bpress) {
//     buttonpressed = true;
//     btime = millis();
//     buttonstate = true;
//     Serial.print(F("Button press: "));
//     Serial.println(btime);
//     sendonesp((uint8_t *)buttonmsg, sizeof(buttonmsg));
//     //esp_now_send(Server_Address, (uint8_t *)buttonmsg, sizeof(buttonmsg));
//   }
// }

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
    }
  }

  //handle_button(false, runningtime);

  if (upddisplay > 0){
    if (++upddisplay > 100){
      updateDisplay();
    }
  }

  heartbeat(runningtime, false);
  
}

// ICACHE_RAM_ATTR void buttonPress(){
//   // Serial.print(F("Button press: "));
//   // Serial.println(millis());
//   handle_button(true, millis());
// }
