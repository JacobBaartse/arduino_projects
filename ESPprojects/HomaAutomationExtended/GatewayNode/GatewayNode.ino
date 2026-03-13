extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>

const int led = LED_BUILTIN;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t GW1_Address[] = { 0x48, 0x3F, 0xDA, 0x69, 0xCB, 0x61};
uint8_t BC1_Address[] = { 0x68, 0xC6, 0x3A, 0xFC, 0x23, 0x76};

/*
board GW1:
Station MAC: 48:3F:DA:69:CB:61
SoftAP MAC: 4A:3F:DA:69:CB:61

board BC1:
Station MAC: 68:C6:3A:FC:23:76
SoftAP MAC: 6A:C6:3A:FC:23:76


*/

// --------------------
// WiFi Settings
// --------------------
// const char* ssid = "T24_optout";
// const char* password = "T24T24T24";

// const char* ssid = "PietLebara";
// const char* password = "piet1234";

ESP8266WebServer server(80);

// --------------------
// ESP-NOW Receive Callback
// --------------------
void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  static unsigned long rcount = 0;
  rcount += 1;
  Serial.print("ESP-NOW Received ");
  Serial.print(rcount);
  Serial.print(" from ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(macStr);
  Serial.print(" | Data: ");
  Serial.write(data, len - 1);
  Serial.print(" at: ");
  Serial.println(millis());
}

// Callback when data is sent
void onDataSent(uint8_t *mac_addr, uint8_t status) {
  static unsigned long scount = 0;
  scount += 1;
  Serial.print("ESP-NOW Send Status ");
  Serial.print(scount);
  Serial.print(": ");
  Serial.print(status == 0 ? "Success" : "Fail");
  Serial.print(" at: ");
  Serial.println(millis());
}

const String startsection = "<!DOCTYPE HTML><html><head><title>Home controller for Sickengaoord 148</title> \
      <style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style> \
      </head><h1>Home network SO 148</h1><br><br>";
const String endsection = "</body></html>";

String makewebpagehtml(){ // to be enhanced, array processing
  String htmlpage = startsection;

  htmlpage += endsection;
  
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

bool timepassing(unsigned long curtime, unsigned long duration){
  static unsigned long rtime = 0;
  if(rtime + duration > curtime) return false;
  rtime = millis();
  return true;
}

// --------------------
// Setup
// --------------------
void setup() {
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

  // WiFi STA mode (required for ESP-NOW)
  // //WiFi.mode(WIFI_STA);
  // WiFi.mode(WIFI_AP_STA);  // Enable both AP and Station modes
  // WiFi.softAP("ESP_NOW_CH_4", "", 4, false); 

  // // Connect to WiFi for webserver
  // Serial.print("Connecting to WiFi ");
  // Serial.print(ssid);
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(300);
  //   Serial.print(".");
  // }
  // Serial.println("\nWiFi connected");
  // Serial.print(", IP address: ");
  // Serial.println(WiFi.localIP());
  // uint8_t ch = WiFi.channel();
  // Serial.printf(", router channel: %d\n", ch);
  // //Serial.println(" ");

  //WiFi.mode(WIFI_STA);
  WiFi.mode(WIFI_AP_STA);
  wifi_promiscuous_enable(1);   // required to allow channel change
  wifi_set_channel(4);         // choose your channel (1–13)
  wifi_promiscuous_enable(0);
  //WiFi.disconnect();
  WiFi.softAP("ESP_NOW_CH_4", "", 4, false, 8); // Start the local access point
  WiFi.softAP("ESP_NOW_CH_4"); // Start the local access point
  IPAddress local_ip(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
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
  //esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 4, NULL, 0);
  esp_now_add_peer(BC1_Address, ESP_NOW_ROLE_COMBO, 4, NULL, 0);


  // if (MDNS.begin("esp8266")) { 
  //   Serial.println("MDNS responder started"); 
  // }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  Serial.print(F("ESP-NOW channel 4, "));
  Serial.println(F("ESP-NOW Gateway Ready"));
  digitalWrite(led, 1); // turn onboard LED off
}

const char msg[] = "Hello from Gateway !";
unsigned long runningtime = 0;
bool action = false;

// --------------------
// Main Loop
// --------------------
void loop() {
  
  runningtime = millis();

  action = timepassing(runningtime, 30000);
  if (action){
    esp_now_send(BC1_Address, (uint8_t *)msg, sizeof(msg));
  }
}
