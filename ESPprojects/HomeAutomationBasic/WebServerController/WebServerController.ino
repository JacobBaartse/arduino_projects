/*

 Dualmode wifi, webserver using links

 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// Station Mode Credentials
const char* sta_ssid = "T24_optout";
const char* sta_password = "T24T24T24";

// AP Mode Credentials
const char* ap_ssid = "Local_AP";
const char* ap_password = "local_148_AP";

// login credentials
const char* http_username = "so148";
const char* http_password = "pietcarla";

ESP8266WebServer server(80);

const int led = LED_BUILTIN;
int led0_val = 9;
int led1_val = 9;
int led2_val = 9;
int led3_val = 9;
int led4_val = 9;
int rootresponse = 0;
bool ledflash = false;

unsigned long runningtime = 0;
unsigned long requesttime = 0;
unsigned long pollingtime = 5000;
unsigned long norequesttime = pollingtime * 4.5;

const String homeLinks = "<!DOCTYPE HTML><html> \
  <head> \
    <title>Home controller for Sickengaoord 148</title> \
    <style> \
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; } \
    </style> \
  </head> \
  <h1>Home network, LED control (onboard LEDs)</h1> \
  <br><br> \
  <table> \
  <tr><th>on</th><th>off</th><th>flash</th></tr> \
  <tr><td><a href=\"/?id=0&stat=0\">server</a></td><td><a href=\"/?id=0&stat=1\">server</a></td><td><a href=\"/?id=0&stat=2\">server</a></td></tr> \
  <tr><td><a href=\"/?id=1&stat=0\">client 1</a></td><td><a href=\"/?id=1&stat=1\">client 1</a></td><td><a href=\"/?id=1&stat=2\">client 1</a></td></tr> \
  <tr><td><a href=\"/?id=2&stat=0\">client 2</a></td><td><a href=\"/?id=2&stat=1\">client 2</a></td><td><a href=\"/?id=2&stat=2\">client 2</a></td></tr> \
  <tr><td><a href=\"/?id=3&stat=0\">client 3</a></td><td><a href=\"/?id=3&stat=1\">client 3</a></td><td><a href=\"/?id=3&stat=2\">client 3</a></td></tr> \
  <tr><td><a href=\"/?id=4&stat=0\">client 4</a></td><td><a href=\"/?id=4&stat=1\">client 4</a></td><td><a href=\"/?id=4&stat=2\">client 4</a></td></tr> \
  <tr><td><a href=\"/?id=999&stat=0\">all</a></td><td><a href=\"/?id=999&stat=1\">all</a></td><td><a href=\"/?id=999&stat=2\">all</a></td></tr> \
  </table> \
  <table> \
  <tr><th>polling</th></tr> \
  <tr><td><a href=\"/?poll=2\">2 sec</a></td></tr> \
  <tr><td><a href=\"/?poll=5\">5 sec</a></td></tr> \
  <tr><td><a href=\"/?poll=10\">10 sec</a></td></tr> \
  <tr><td><a href=\"/?poll=60\">1 min</a></td></tr> \
  </table> \
  </body> \
</html>";

    // &nbsp;<a href=\"/?led0=0\">Turn local LED on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led0=1\">Turn locaL LED off</a>&nbsp;<a href=\"/?led0=2\">Flash local LED</a>&nbsp;<br><br> \
    // &nbsp;<a href=\"/?led1=0\">Turn LED 1 on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led1=1\">Turn LED 1 off</a>&nbsp;<a href=\"/?led1=2\">Flash LED 1</a>&nbsp;<br><br> \
    // &nbsp;<a href=\"/?led2=0\">Turn LED 2 on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led2=1\">Turn LED 2 off</a>&nbsp;<a href=\"/?led2=2\">Flash LED 2</a>&nbsp;<br><br> \
    // &nbsp;<a href=\"/?led3=0\">Turn LED 3 on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led3=1\">Turn LED 3 off</a>&nbsp;<a href=\"/?led3=2\">Flash LED 3</a>&nbsp;<br><br> \
    // &nbsp;<a href=\"/?led4=0\">Turn LED 4 on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led4=1\">Turn LED 4 off</a>&nbsp;<a href=\"/?led4=2\">Flash LED 4</a>&nbsp;<br><br> \
    // &nbsp;<a href=\"/?ledall=0\">Turn all LEDs on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?ledall=1\">Turn all LEDs off</a>&nbsp;<a href=\"/?ledall=2\">Flash all LEDs</a>&nbsp;<br><br> \
    // &nbsp;<a href=\"/?poll=1\">1 second</a>&nbsp;&nbsp;&nbsp;<a href=\"/?poll=5\">5 seconds</a>&nbsp;<br><br> \
    // &nbsp;<a href=\"/?poll=10\">10 seconds</a>&nbsp;&nbsp;&nbsp;<a href=\"/?poll=60\">1 minute</a>&nbsp;<br><br> \

void handleRoot() {
  int valnow = 9;
  int idval = 9;

  // Check if client credentials match
  if (!server.authenticate(http_username, http_password)) {
    // If not authenticated, request authentication
    Serial.println("Authentication failed, requesting credentials.");
    // Send 401 Unauthorized response with WWW-Authenticate header
    return server.requestAuthentication(BASIC_AUTH, "Sickengaoord 148 thuis netwerk");
  }

  Serial.print(F("handleRoot: "));
  // Serial.print("URI: "); 
  // Serial.print(server.uri()); 
  // Serial.print(", method: "); 
  // Serial.print(server.method() == HTTP_GET ? "GET" : "POST"); 
  // Serial.print(", arguments:"); 
  // for (uint8_t i = 0; i < server.args(); i++){ 
  //   Serial.printf(" %s = %s", server.argName(i).c_str(), server.arg(i).c_str()); 
  // }
  // Serial.println(F(" "));

  if (server.hasArg("poll")) {
    rootresponse = server.arg("poll").toInt();
  }
  if (server.hasArg("stat")) {
    valnow = server.arg("stat").toInt();
  }
  if (server.hasArg("id")) {
    idval = server.arg("id").toInt();
    if (idval > 10){ // all (clients and the server)
      led0_val = valnow;
      led1_val = valnow;
      led2_val = valnow;
      led3_val = valnow;
      led4_val = valnow;
    }
    if (idval == 0){ // web server
      led0_val = valnow;
    }
    if (idval == 1){ // client 1
      led1_val = valnow;
    }
    if (idval == 2){ // client 2
      led2_val = valnow;
    }
    if (idval == 3){ // client 3
      led3_val = valnow;
    }
    if (idval == 4){ // client 4
      led4_val = valnow;
    }
  }
  // if (server.hasArg("led0")) {
  //   led0_val = server.arg("led0").toInt();
  // }
  //Serial.print("led 0: ");
  Serial.print("leds: ");
  Serial.print(led0_val);
  // if (server.hasArg("led1")) {
  //   led1_val = server.arg("led1").toInt();
  // }
  Serial.print(", ");
  Serial.print(led1_val);
  // if (server.hasArg("led2")) {
  //   led2_val = server.arg("led2").toInt();
  // }
  Serial.print(", ");
  Serial.print(led2_val);  
  // if (server.hasArg("led3")) {
  //   led3_val = server.arg("led3").toInt();
  // }
  Serial.print(", ");
  Serial.print(led3_val);  
  // if (server.hasArg("led4")) {
  //   led4_val = server.arg("led4").toInt();
  // }
  Serial.print(", ");
  Serial.print(led4_val);
  Serial.println(F(" !"));

  if (led0_val < 3){ // local LED on this server board
    if (led0_val < 2){ // local LED on this server board
      ledflash = false;
      digitalWrite(led, led0_val);
    }
    else{ // value 2 means flashing
      ledflash = true;
    }
  }
  Serial.println(F("Server homepage"));
  server.send(200, "text/html", homeLinks);
  Serial.println(F(" "));
}

void handleLEDjson() {
  int clientval = 9;
  int clientstat = 9;
  int clientresp = 9;
  //int qval = 9;
  runningtime = millis();
  Serial.print(F("handleLED, "));
  // Serial.print("URI: "); 
  // Serial.print(server.uri()); 
  // Serial.print(", method: "); 
  // Serial.print(server.method() == HTTP_GET ? "GET" : "POST"); 
  // Serial.print(", arguments:"); 
  // for (uint8_t i = 0; i < server.args(); i++){ 
  //   Serial.printf(" %s = %s", server.argName(i).c_str(), server.arg(i).c_str()); 
  // }
  // Serial.println(F(" "));

  String response = ""; // "{\"led\" : 9, \"value\" : 9"; // }"; // unknown client id

  if (server.hasArg("cid")) {
    clientval = server.arg("cid").toInt();
  } 
  if (server.hasArg("cstat")) {
    clientstat = server.arg("cstat").toInt();
  } 
  // Serial.print("clientstat: "); 
  // Serial.println(clientstat); 

  if (clientval == 1){ // client 1
    if (led1_val > 2){
      led1_val = clientstat;
    }
    clientresp = led1_val; 
  }
  if (clientval == 2){ // client 2
    if (led2_val > 2){
      led2_val = clientstat;
    }
    clientresp = led2_val; 
  }
  if (clientval == 3){ // client 3
    if (led3_val > 2){
      led3_val = clientstat;
    }
    clientresp = led3_val; 
  }
  if (clientval == 4){ // client 4
    if (led4_val > 2){
      led4_val = clientstat;
    }
    clientresp = led4_val; 
  }
  
  response = "{\"led\" : " + String(clientval) + ", \"value\" : " + String(clientresp); // + "}";

  response += ", \"servertime\" : " + String(runningtime) + ", \"pollingtime\" : " + String(pollingtime) + " }";

  Serial.print(F("response: '"));
  Serial.print(response);
  Serial.println(F("'"));
  server.send(200, "application/json", response);
  requesttime = runningtime;
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

bool requesttimelapsed(unsigned long duration){
  static int lapsed = 0;
  if(runningtime < requesttime + duration){ 
    lapsed = 0;
    return lapsed;
  }
  if (lapsed > 0){
    Serial.print(F("No timely request received: "));
    Serial.print(lapsed);
    Serial.print(F(", "));
    Serial.println(runningtime);
  }
  lapsed += 1;
  requesttime = runningtime; // set the printing as request time, to get 1 message per duration
  return lapsed; 
}

void ledflashing(unsigned long timestamp, unsigned long duration){
  static unsigned long ledtime = 0;
  if(timestamp < ledtime) return;
  ledtime = millis() + duration; // make sure to get 'fresh' timestamp to avoid processing time influences
  digitalWrite(led, !digitalRead(led)); // toggle onboard LED
}

void setup(void) {

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

  WiFi.mode(WIFI_AP_STA);  // Enable both AP and Station modes
  WiFi.softAP(ap_ssid, ap_password);  // Start the local access point
  IPAddress local_ip(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.begin(sta_ssid, sta_password); // Connect to existing network (router, internet)
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("");
  Serial.print("Connected to: ");
  Serial.print(sta_ssid);
  Serial.print(", IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(" ");

  Serial.print("Local Access Point: ");
  Serial.print(ap_ssid);
  Serial.print(", IP address: ");
  Serial.println(local_ip);
  Serial.println(" ");

  if (MDNS.begin("esp8266")) { 
    Serial.println("MDNS responder started"); 
  }

  server.on("/", handleRoot);
  server.on("/led", handleLEDjson);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(led, 1); // turn onboard LED off
}

int norequest = 0;

void loop(void) {

  runningtime = millis();

  if (ledflash){
    ledflashing(runningtime, 1000);
  }

  server.handleClient();
  if(rootresponse > 0){
    Serial.print("Root responsing: ");
    Serial.println(rootresponse);
    pollingtime = rootresponse * 1000;
    norequesttime = pollingtime * 4.5;
    rootresponse = 0; // handle once
  }

  norequest = requesttimelapsed(norequesttime);
  // if (norequest > 0){ // no request received within time period
  //   pollingtime = 2000;
  // }

}
