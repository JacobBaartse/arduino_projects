extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "serials.h"
#include "esp_now.h"

char reftext[11] = "client_SD";

const int led = LED_BUILTIN;
//const int buttonPin = D3; 

// char Lines[4][101] = {
//   "Welcome Leo",
//   "Demo {small disp.}", 
//   "Whats up?",
//   "Hello World"
// };  
// uint8_t LinesYPos[4] = { 16, 32, 48, 64 };
// uint8_t upddisplay = 200;

// uint8_t GW1_Address[] = { 0x48, 0x3F, 0xDA, 0x69, 0xCB, 0x61};
// uint8_t BC1_Address[] = { 0x68, 0xC6, 0x3A, 0xFC, 0x23, 0x76};
// uint8_t GW1_Address[] = { 0x4A, 0x3F, 0xDA, 0x69, 0xCB, 0x61};
// uint8_t BC1_Address[] = { 0x6A, 0xC6, 0x3A, 0xFC, 0x23, 0x76};

uint8_t GW1_Address[] = { 0x86, 0xcc, 0xa8, 0xa1, 0xe0, 0x00 }; // This local Gateway node
uint8_t BC1_Address[] = { 0x84, 0xF3, 0xEB, 0x6C, 0xF1, 0xAB };  // Client, Small Display #1

/*
board GW1:
Station MAC: 48:3F:DA:69:CB:61
SoftAP MAC: 4A:3F:DA:69:CB:61

board BC1:
Station MAC: 68:C6:3A:FC:23:76
SoftAP MAC: 6A:C6:3A:FC:23:76

*/

IPAddress local_ip(192,168,4,1);
IPAddress gateway(192,168,4,1);
IPAddress subnet(255,255,255,0);

const char* ssidname = "ESP_NOW_CH_4";
const char* ssidpassword = "ch4ch4ch4";

ESP8266WebServer server(80);

const String startsection = "<!DOCTYPE HTML><html><head><title>ESP-NOW controller and webpage</title> \
      <style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style> \
      </head><h1>Local AP browser with esp-now network</h1><br><br>";
const String endsection = "</body></html>";
const String GWhtml = "<a href=\"/GW\">L1</a>";
const String BChtml = "<a href=\"/BC\">L2</a>";

String makewebpagehtml() { // to be enhanced, array processing
  String htmlpage = startsection;
  htmlpage += F("Local AP trial<BR><BR>");
  htmlpage += F("For now 2 links which can be clicked");
  htmlpage += F("<BR><BR>");
  htmlpage += GWhtml;
  htmlpage += F("<BR><BR>");
  htmlpage += BChtml;
  htmlpage += F("<BR><HR>");
  htmlpage += endsection;
  // Serial.print(htmlpage);
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

const char webmsg[] = "webcontrol message";

uint8_t linkcount = 0;
void handleLinkClear() {
  if (linkcount > 0) {
    send_display_line(BC1_Address, 0, "-0-");
    send_display_line(BC1_Address, 1, "-1-");
    send_display_line(BC1_Address, 2, "-2-");
    send_display_line(BC1_Address, 3, "-3-");
  }
}

void handleLink1(bool local=false) {
  bool remote = (storeData[0] & 1) > 0;
  // Serial.print(F("1 "));
  // Serial.println(storeData[0], HEX);
  if (remote) {
    Serial.println(F("remote 1 "));
  }
  if (local) {
    Serial.println(F("local 1 "));
  }
  if (local || remote) {
    //Serial.println(F("handle link 1"));

    storeData[0] = storeData[0] & 0xfffffffe;
    // Serial.print(F("L1 "));
    // Serial.println(storeData[0], HEX);

    send_display_line(BC1_Address, 1, "eerste klik");
    //randomstringvalue(22);
    //send_display_line(BC1_Address, 1, rmsg);
    linkcount++;
  }
}

void handleLink2(bool local=false) {
  bool remote = (storeData[0] & 2) > 0;
  // Serial.print(F("2 "));
  // Serial.println(storeData[0], HEX);
  if (remote) {
    Serial.println(F("remote 2 "));
  }
  if (local) {
    Serial.println(F("local 2 "));
  }
  if (local || remote) {
    //Serial.println(F("handle link 2"));

    // this should not be a roadcast address, also the device should be checked for connectivity first
    //esp_now_send(BC1_Address, (uint8_t *)webmsg, sizeof(webmsg));

    storeData[0] = storeData[0] & 0xfffffffd;
    // Serial.print(F("L2 "));
    // Serial.println(storeData[0], HEX);

    send_display_line(BC1_Address, 2, "klikt num twee");
    // randomstringvalue(11);
    // send_display_line(BC1_Address, 2, rmsg);
    linkcount++;
  }
}

void handleGW() {
  Serial.println(F("local link 1"));

  // toggle LED or so
  handleLink1(true);

  String webpage = makewebpagehtml(); // include the current status information
  server.send(200, "text/html", webpage);
  Serial.println(F(" "));
}

void handleBC() {
  Serial.println(F("local link 2"));

  handleLink2(true);

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

// --------------------
// Setup
// --------------------
void setup() {
  //pinMode(buttonPin, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0); // turn onboard LED on
  Serial.begin(115200);
  serial_setup(); // serial connection to the other board

  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.println(F("Starting ESP device"));
  Serial.flush();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

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
  esp_now_add_peer(BC1_Address, ESP_NOW_ROLE_COMBO, 4, NULL, 0);

  server.on("/", handleRoot);
  server.on("/BC", handleBC);
  server.on("/GW", handleGW);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  Serial.print(F("ESP-NOW channel 4, "));
  Serial.println(F("ESP-NOW Gateway Ready"));

  //attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPress, FALLING); // trigger when button pressed

  digitalWrite(led, 1); // turn onboard LED off
}

const char msg[] = "Hello from Gateway !";
const char buttonmsg[] = "Button pressed (GW1).";
unsigned long runningtime = 0;
bool action = false;
bool newdata = false;
uint32_t remStoredData = 0xffffffff;

// --------------------
// Main Loop
// --------------------
void loop() {
  
  runningtime = millis();

  action = timepassing(runningtime, 30000);
  if (action) { 

    handleLinkClear();

    beating(); // send heart beats to clients

    //esp_now_send(BC1_Address, (uint8_t *)msg, sizeof(msg));

    // and something on the serial port to the GatewayInternet
  }

  server.handleClient();

  //handle_button(false, runningtime);

  newdata = readserialdata();
  if (newdata){
    if (resetclear){ // other board is restarted
      Serial.println(F("reset detected other node"));
      for (uint8_t ai=0;ai<16;ai++){
        Serial.print("Stored at index: ");
        Serial.print(ai);
        Serial.print(", 0x");
        Serial.println(storeData[ai], HEX);
        storeData[ai] = 0;
      }      
      //ESP.restart(); // do not restart this board, endless loop will happen for serial1 and serial2

      resetclear = false;
    }
    else {
      if (storeData[1] > 0){
        storeData[0] = storeData[0] | 1;
        storeData[1] = 0;
      }
      if (storeData[2] > 0){
        storeData[0] = storeData[0] | 2;
        storeData[2] = 0;
      }

      // only print in case the value is changed
      if (storeData[0] != remStoredData){
        Serial.println(storeData[0], HEX);
        remStoredData = storeData[0]; 

        Serial.println(F("check handlers"));

        handleLink1();
        handleLink2();

        Serial.println(storeData[0], HEX);
        remStoredData = storeData[0]; 
      }
    }
    newdata = false;
  }

}

// ICACHE_RAM_ATTR void buttonPress(){
//   // Serial.print(F("Button press: "));
//   // Serial.println(millis());
//   handle_button(true, millis());
// }
