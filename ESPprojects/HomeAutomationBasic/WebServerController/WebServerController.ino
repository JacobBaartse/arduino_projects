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

ESP8266WebServer server(80);

const int led = LED_BUILTIN;
int led0_val = 2;
int led1_val = 2;
int led2_val = 2;
int led3_val = 2;
int led4_val = 2;
unsigned long runningtime = 0;
unsigned long requesttime = 0;

const String homeLinks = "<html>\
  <head>\
    <title>Home controller SO-148</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <h1>Home network, LED control (onboard LEDs)</h1>\
  <br><br>\
    &nbsp;<a href=\"/?led0=0\">Turn local LED on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led0=1\">Turn locaL LED off</a>&nbsp;<br><br>\
    &nbsp;<a href=\"/?led1=0\">Turn LED 1 on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led1=1\">Turn LED 1 off</a>&nbsp;<br><br>\
    &nbsp;<a href=\"/?led2=0\">Turn LED 2 on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led2=1\">Turn LED 2 off</a>&nbsp;<br><br>\
    &nbsp;<a href=\"/?led3=0\">Turn LED 3 on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led3=1\">Turn LED 3 off</a>&nbsp;<br><br>\
    &nbsp;<a href=\"/?led4=0\">Turn LED 4 on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led4=1\">Turn LED 4 off</a>&nbsp;<br><br>\
    &nbsp;<a href=\"/?ledall=0\">Turn all LEDs on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?ledall=1\">Turn all LEDs off</a>&nbsp;<br><br>\
  </body>\
</html>";

void handleRoot() {
  Serial.print(F("handleRoot: "));
  if (server.hasArg("ledall")) {
    int valnow = server.arg("ledall").toInt();
    led0_val = valnow;
    led1_val = valnow;
    led2_val = valnow;
    led3_val = valnow;
    led4_val = valnow;
  }
  // if (server.hasArg("ledalloff")) {
  //   led0_val = 1;
  //   led1_val = 1;
  //   led2_val = 1;
  //   led3_val = 1;
  //   led4_val = 1;
  // }
  if (server.hasArg("led0")) {
    led0_val = server.arg("led0").toInt();
  }
  //Serial.print("led 0: ");
  Serial.print("leds: ");
  Serial.print(led0_val);
  if (server.hasArg("led1")) {
    led1_val = server.arg("led1").toInt();
  }
  Serial.print(", ");
  Serial.print(led1_val);
  if (server.hasArg("led2")) {
    led2_val = server.arg("led2").toInt();
  }
  Serial.print(", ");
  Serial.print(led2_val);  
  if (server.hasArg("led3")) {
    led3_val = server.arg("led3").toInt();
  }
  Serial.print(", ");
  Serial.print(led3_val);  
  if (server.hasArg("led4")) {
    led4_val = server.arg("led4").toInt();
  }
  Serial.print(", ");
  Serial.print(led4_val);
  Serial.println(F(" !"));

  if (led0_val < 2){ // local LED on this server board
    digitalWrite(led, led0_val);
  }
  Serial.println(F("Server homepage"));
  server.send(200, "text/html", homeLinks);
  Serial.println(F(" "));
}

void handleLEDplain() {
  Serial.print(F("handleLED, "));

  String response = "-*-";
  if (server.hasArg("led1")) {
    if (led1_val < 2){
      response = "led1=" + String(led1_val);
    }
  }  
  if (server.hasArg("led2")) {
    if (led2_val < 2){
      response = "led2=" + String(led2_val);
    }
  }
  if (server.hasArg("led3")) {
    if (led3_val < 2){
      response = "led3=" + String(led3_val);
    }
  }
  Serial.print(F("response: '"));
  Serial.print(response);
  Serial.print(F("', runningtime: "));
  Serial.println(runningtime);
  server.send(200, "text/plain", response);
  requesttime = runningtime;
}

void handleLEDjson() {
  Serial.print(F("handleLED, "));

  String response = "{\"led\" : 9, \"value\" : 2 }"; // invalid client id
  if (server.hasArg("led1")) {
    // int Mode = server.arg("led1").toInt();
    // Serial.println(Mode);

    if (led1_val < 3){
      response = "{\"led\" : 1, \"value\" : " + String(led1_val) + "}";
    }
  }  
  if (server.hasArg("led2")) {
    if (led2_val < 3){
      response = "{\"led\" : 2, \"value\" : " + String(led2_val) + "}";
    }
  }
  if (server.hasArg("led3")) {
    if (led3_val < 3){
      response = "{\"led\" : 3, \"value\" : " + String(led3_val) + "}";
    }
  }
  if (server.hasArg("led4")) {
    if (led4_val < 3){
      response = "{\"led\" : 4, \"value\" : " + String(led4_val) + "}";
    }
  }
  Serial.print(F("response: '"));
  Serial.print(response);
  Serial.print(F("', runningtime: "));
  Serial.println(runningtime);
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
  if(runningtime < requesttime + duration) return false;
  Serial.print(F("No timely request received: "));
  Serial.println(runningtime);
  requesttime = runningtime; // set the printing as request time, to get 1 message per duration
  return true;
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

bool norequest = false;

void loop(void) {

  runningtime = millis();

  server.handleClient();

  norequest = requesttimelapsed(60000);

}
