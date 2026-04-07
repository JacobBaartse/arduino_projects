extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const int led = LED_BUILTIN;
const int buttonPin = D3; 
char reftext[11] = "web_server";

enum MessageType {PAIRING, DATA, ACK, TEXT};
MessageType messageType;

uint8_t connectedclients[20][6] = {
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
uint8_t Broadcast_Address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//uint8_t Client_Address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
bool serverknown = false;

/*
board Server:
Station MAC: 48:3F:DA:69:CB:61
SoftAP MAC: 4A:3F:DA:69:CB:61

board Client1:
Station MAC: 68:C6:3A:FC:23:76
SoftAP MAC: 6A:C6:3A:FC:23:76

*/

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
uint8_t textackcount = 0;
char forminput[101] = {'\0'};
uint8_t textfromform = 0;

IPAddress local_ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

const char* ssidname = "CH4_192_168_4_1";
const char* ssidpassword = "ch4ch4ch4";

ESP8266WebServer server(80);


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
      //referencestring[index] = refstring; 
      break; // return index;
    }
  }
  Serial.print(F("indexMAC "));
  printMAC(mac_addr);
  Serial.print(F(": "));
  Serial.println(index);
  return index;
}

int getindexMAC(const uint8_t * mac_addr){
  int macindex = indexMAC(mac_addr);
  if (macindex > 20){
    macindex = indexMAC(Broadcast_Address); // this should be 0..19
    for ( int id = 0; id < 6; id++ ){
      connectedclients[macindex][id] = mac_addr[id];
    }
    connectedclientcount = macindex + 1;
  }
  return macindex;
}

void addPeer(uint8_t *peer_addr) {      // add pairing
  esp_now_del_peer(peer_addr);
  int res = esp_now_add_peer(peer_addr, ESP_NOW_ROLE_COMBO, 4, NULL, 0);
  if (res == 0){
    Serial.println("PEER added ");
  }
}

// function to send 1 single ESP-NOW message
void sendonesp(u8 *da, u8 *data, int len){
  // char macStr[18];
  // snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
  //          da[0], da[1], da[2], da[3], da[4], da[5]);
  // Serial.print(macStr);
  printMAC(da);
  esp_now_send(da, data, len);
}

// --------------------
// ESP-NOW Receive Callback
// --------------------
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

  uint8_t type = incomingData[0];       // first message byte is the type of message 
  switch (type) {
  case DATA:                           // the message is data type
    Serial.println("DATA");
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
    // Serial.println(pairingData.msgType);
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
        for ( int id = 0; id < 6; id++ ){
          pairingData.ClientmacAddr[id] = mac[id];
          //Client_Address[id] = mac[id];
        }
        addPeer(mac);
        if (serverknown){
          for ( int id = 0; id < 6; id++ ){
            pairingData.ServermacAddr[id] = Server_Address[id];
          }
        }
        getindexMAC(mac); // add to connected clients list
      break;
      case 2: // second message on pairing, capture server MAC (if not already known)
        pairingData.id = 3;
        if (!serverknown){
          for ( int id = 0; id < 6; id++ ){
            Server_Address[id] = pairingData.ServermacAddr[id];
          }
          serverknown = true;
        }
        resppairing = false;
      break;
      default:
        resppairing = false;
    }
    if (resppairing){
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
  case ACK:                            // the message is an aknowledge message 
    Serial.println("ACK");

    break; 
  case TEXT:                           // the message is text type
    Serial.println("TEXT");
    textackcount += 1;
    break;
  default:
    Serial.print("Unknown message type: ");
    Serial.println(type);
  }
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

const String startsection = "<!DOCTYPE HTML><html><head><title>ESP-NOW controller and webpage</title> \
      <style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style> \
      </head><h1>Local esp-now network with AP</h1><br><br>";
const String endsection = "<BR><HR></body></html>";
const String GWhtml = "<a href=\"/GW\">GateWay</a>";
const String BChtml = "<a href=\"/BC\">Remote Node</a>";
const String FORMhtml = "<BR>Text input:<FORM action=\"/post\"><input type=\"text\" name=\"textinput\" required minlength=\"1\" maxlength=\"20\" size=\"10\"/>&nbsp;&nbsp;&nbsp;<input type=\"submit\" value=\"send\" name=\"send\"/></FORM><a href=\"/cleardisplay\">Clear</a><BR>";

String deviceslisting = "";
void activedeviceslisting(){
  deviceslisting = "<select name=\"device\" id=\"dev\"><option value=\"99\" selected=\"selected\">All</option>";
  for ( int idn = 0; idn < connectedclientcount; idn++ ){
    deviceslisting += "<option value=\"";
    deviceslisting += idn;
    deviceslisting += "\">";
    deviceslisting += referencestring[idn];
    deviceslisting += "</option>";
  }
  deviceslisting += "<option value=\"20\">";  
  deviceslisting += referencestring[20]; // 21th item is the web server
  deviceslisting += "</option>";
  deviceslisting += "</select>";
 
}

String makewebpagehtml(){ // to be enhanced, array processing
  activedeviceslisting();

  String htmlpage = startsection;
  htmlpage += F("Demo/trial/PoC<BR><BR>\n");
  htmlpage += F("For now 2 links which can be clicked\n");
  htmlpage += F("<BR><BR>\n");
  htmlpage += GWhtml;
  htmlpage += F("<BR><BR>\n");
  htmlpage += BChtml;
  htmlpage += F("<BR><HR>\n");
  htmlpage += FORMhtml;
  htmlpage += deviceslisting;
  htmlpage += endsection;
  //Serial.print(htmlpage);
  return htmlpage;
}

// login credentials
// const char* http_username = "so148";
// const char* http_password = "pietcarla";

void handleRoot() {

  // // Check if client credentials match
  // if (!server.authenticate(http_username, http_password)) {
  //   // If not authenticated, request authentication
  //   Serial.println(F("Requesting credentials."));
  //   // Send 401 Unauthorized response with WWW-Authenticate header
  //   return server.requestAuthentication(BASIC_AUTH, "Sickengaoord 148 thuis netwerk");
  // }

  Serial.print(millis());
  Serial.print(F(", handleRoot: "));
  // Serial.print("URI: "); 
  // Serial.print(server.uri()); 
  // Serial.print(", method: "); 
  // Serial.print(server.method() == HTTP_GET ? "GET" : "POST"); 
  // Serial.print(", arguments:"); 
  // for (uint8_t i = 0; i < server.args(); i++){ 
  //   Serial.printf(" %s = %s", server.argName(i).c_str(), server.arg(i).c_str()); 
  // }
  // Serial.println(F(" "));

  Serial.println(F("Server html page"));
  String webpage = makewebpagehtml(); // include the current status information
  server.send(200, "text/html", webpage);
  Serial.println(F(" "));
}

void handleGW() {
  Serial.println(F("handleGW"));

  // toggle LED or so

  String webpage = makewebpagehtml(); // include the current status information
  server.send(200, "text/html", webpage);
  Serial.println(F(" "));
}

const char webmsg[] = "webcontrol message";

void handleBC() {
  Serial.println(F("handleBC"));

  sendonesp(Broadcast_Address, (uint8_t *)webmsg, sizeof(webmsg));
  //esp_now_send(BC1_Address, (uint8_t *)webmsg, sizeof(webmsg));

  String webpage = makewebpagehtml(); // include the current status information
  server.send(200, "text/html", webpage);
  Serial.println(F(" "));
}

void handleFORM() {
  Serial.println(F("handleFORM"));

  if (server.hasArg("textinput") && server.arg("textinput") != NULL){
    //Serial.print("text input: ");
    memset(forminput, 0, sizeof(forminput));
    String serverstring = server.arg("textinput");
    //Serial.println(serverstring);
    serverstring.toCharArray(forminput, serverstring.length() + 1);
    Serial.print("forminput: ");
    Serial.println(forminput);
    textfromform = 2;
  }

  String webpage = makewebpagehtml(); // include the current status information
  server.send(200, "text/html", webpage);
  Serial.println(F(" "));
}
void handleCLEAR() {
  Serial.println(F("handleCLEAR"));
  textfromform = 95;
  String webpage = makewebpagehtml(); // include the current status information
  server.send(200, "text/html", webpage);
  Serial.println(F(" "));
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) { 
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; 
  }
  Serial.print(F("Response from server (unexpected request): '*'"));
  Serial.print(message);
  Serial.println(F("'*'"));
  server.send(404, "text/plain", message);
}

// function to indicate the passing of certain duration
bool timepassing(unsigned long curtime, unsigned long duration){
  static unsigned long rtime = 0;
  if(rtime + duration > curtime) return false;
  rtime = millis(); // get fresh time to base the new interval on
  return true;
}

// function to indicate the passing of certain duration
bool timepassing2(unsigned long curtime, uint8_t inputval, unsigned long duration){
  static unsigned long rtime = 0;
  if (inputval < 2){ // skip time check if inputval > 1
    if(rtime + duration > curtime) return false;
  }
  rtime = millis(); // get fresh time to base the new interval on
  return true;
}

char rmsg[101];
void randomstringvalue(int numBytes){
  memset(rmsg, 0, sizeof(rmsg));
  for(int i = 0; i < numBytes; i++) {
    int randomValue = random(0, 36);
    rmsg[i] = randomValue + 'a';
    if(randomValue > 25) {
      rmsg[i] = (randomValue - 26) + '0';
    }
    else { // some change on a capital letter
      int capitalValue = random(0, 101);
      if (capitalValue < 30){
        rmsg[i] = randomValue + 'A';
      }
    }
  }
  //rmsg[numBytes] = '\0';
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

  memcpy(&reftext, referencestring[20], 11);

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  // WiFi.softAP(ssidname, ssidpassword, 4); // Start the local access point
  WiFi.softAP(ssidname, "", 4); // Start the local access point

  Serial.print(F("AP: "));
  Serial.println(WiFi.softAPIP());
  Serial.println("");  

  // ESP-NOW init
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  // Add broadcast peer (improves reliability)
  //esp_now_add_peer(BC1_Address, ESP_NOW_ROLE_COMBO, 4, NULL, 0);

  server.on("/", handleRoot);
  server.on("/BC", handleBC);
  server.on("/GW", handleGW);
  server.on("/post", handleFORM);
  server.on("/cleardisplay", handleCLEAR);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPress, FALLING); // trigger when button pressed

  Serial.print(F("ESP-NOW channel 4, "));
  Serial.println(F("ESP-NOW Server Ready"));
  digitalWrite(led, 1); // turn onboard LED off
}

const char msg[] = "Hello from Server !";
const char buttonmsg[] = "Button pressed (Server).";
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
//     sendonesp(Broadcast_Address, (uint8_t *)buttonmsg, sizeof(buttonmsg));
//     //esp_now_send(Broadcast_Address, (uint8_t *)buttonmsg, sizeof(buttonmsg));
//   }
// }

uint8_t textcount = 0;
uint8_t runningclient = 0;

// --------------------
// Main Loop
// --------------------
void loop() {
  
  runningtime = millis();

  action = timepassing(runningtime, 30000);
  if (action){
    sendonesp(Broadcast_Address, (uint8_t *)msg, sizeof(msg)); // heartbeat message

    // if (connectedclientcount < 1){
    //   pairingData.id = 33;
    //   pairingData.msgType = PAIRING;
    //   pairingData.channel = 4;
    //   sendonesp(Broadcast_Address, (uint8_t *)&pairingData, sizeof(pairingData));
    // }
    // else {
    //   sendonesp(Broadcast_Address, (uint8_t *)msg, sizeof(msg));
    //   //esp_now_send(BC1_Address, (uint8_t *)msg, sizeof(msg));
    // }
  }
  
  action = timepassing2(runningtime, textfromform, 35000);
  if (action){
    Serial.print(F("Text action: "));
    Serial.println(textackcount);
    textackcount = 0;
    textingData.msgType = TEXT;
    textingData.id = textcount++;
    if (textfromform > 1){ // only send if textfromform is 2
      if (textfromform > 90){ // clear displays
        textingData.line = textfromform; // send specific command to client 91..98, 99 is looping, 95 is clear displays
      }
      else {
        textingData.line = 99;
        memcpy(&textingData.texting, forminput, 101); 
        Serial.print(F("Text textingData.texting: "));
        Serial.println(textingData.texting);
        memset(forminput, 0, sizeof(forminput));
      }
      textfromform = 1; // only send once (to all clients)
      runningclient = connectedclientcount;
    }
    else{
      if (textfromform == 0){
        textingData.line = random(0, 4); // 4 lines of displays
        // textingData.texting[100] = '\0';
        // strcpy(textingData.texting, "tube "); 
        randomstringvalue(random(1, 15));
        //strcpy(textingData.texting, rmsg); 
        memcpy(&textingData.texting, rmsg, 101); 
        runningclient = connectedclientcount;
      }
    }

    // print connected clients (+ 1 extra, showing clearly the end of the list)
    Serial.print(F("Message: "));
    Serial.println(textingData.texting);
    Serial.println(F("Listing clients:"));
    for ( int idn = 0; idn <= connectedclientcount; idn++ ){
      printMAC(connectedclients[idn]);
      Serial.println(F(" "));
    }
    Serial.println(F("------"));
  }

  if (runningclient > 0){
    runningclient -= 1;
    Serial.print(F(" texting "));
    Serial.println(runningclient);
    sendonesp(connectedclients[runningclient], (uint8_t *)&textingData, sizeof(textingData));
  }

  server.handleClient();

  //handle_button(false, runningtime);

}

// ICACHE_RAM_ATTR void buttonPress(){
//   // Serial.print(F("Button press: "));
//   // Serial.println(millis());
//   handle_button(true, millis());
// }
