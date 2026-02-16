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
    &nbsp;<a href=\"/?ledallon\">Turn all LEDs on</a>&nbsp;&nbsp;&nbsp;<a href=\"/?ledalloff\">Turn all LEDs off</a>&nbsp;<br><br>\
  </body>\
</html>";

void handleRoot() {
  Serial.print(F("handleRoot: "));
  if (server.hasArg("ledallon")) {
    led0_val = 0;
    led1_val = 0;
    led2_val = 0;
    led3_val = 0;
  }
  if (server.hasArg("ledalloff")) {
    led0_val = 1;
    led1_val = 1;
    led2_val = 1;
    led3_val = 1;
  }
  if (server.hasArg("led0")) {
    led0_val = server.arg("led0").toInt();
  }
  //Serial.print("led 0: ");
  Serial.print("leds: ");
  Serial.print(led0_val);
  if (server.hasArg("led1")) {
    led1_val = server.arg("led1").toInt();
  }
  //Serial.print(", led 1: ");
  Serial.print(", ");
  Serial.print(led1_val);
  if (server.hasArg("led2")) {
    led2_val = server.arg("led2").toInt();
  }
  //Serial.print(", led 2: ");
  Serial.print(", ");
  Serial.print(led2_val);  
  if (server.hasArg("led3")) {
    led3_val = server.arg("led3").toInt();
  }
  //Serial.print(", led 3: ");
  Serial.print(", ");
  Serial.print(led3_val);
  Serial.println(F(" !"));
  Serial.println(F(" "));

  if (led0_val < 2){
    digitalWrite(led, led0_val);
  }
  server.send(200, "text/html", homeLinks);
}

void handleLED() {
  Serial.println(F("handleLED"));

  String response = "";
  if (server.hasArg("led1")) {
    if (led1_val < 2){

    }
  }  
  if (server.hasArg("led2")) {
    if (led2_val < 2){

    }
  }
  if (server.hasArg("led3")) {
    if (led3_val < 2){

    }
  }
  //server.print(response);
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
  for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
  server.send(404, "text/plain", message);
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
  server.on("/led", handleLED);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(led, 1); // turn onboard LED off
}

void loop(void) {
  server.handleClient();
}
