/*

 https://medium.com/engineering-iot/building-a-dual-mode-web-server-with-esp32-ap-and-station-simultaneously-eda403eb1ee4

 https://github.com/ESP32-Work/ESP32_Webserver/tree/main/ESP32_WebServer

 https://gist.github.com/idreamsi/f3caa557fb55e4fd1c69045de8c24719

 combine dualmode AP, with webserver with links and forms

 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// Station Mode Credentials
const char* sta_ssid = "T24_optout";
const char* sta_password = "T24T24T24";

// AP Mode Credentials
const char* ap_ssid = "ESP_AP";
const char* ap_password = "12345678";

ESP8266WebServer server(80);

const int led = LED_BUILTIN;

const String postForms = "<html>\
  <head>\
    <title>Web Server SO148 trial-1</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>POST plain text</h1><br>\
    <form method=\"post\" enctype=\"text/plain\" action=\"/postplain/\">\
      <input type=\"text\" name=\"plain\": value=\"plain text\"><br>&nbsp;<br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
    <h1>POST form data</h1><br>\
    <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
      <input type=\"text\" name=\"hello\" value=\"world\"><br>&nbsp;<br>\
      <input type=\"submit\" value=\"Submit\">\
    </form>\
    &nbsp;<a href=\"/?led=0\">Turn LED on!</a>&nbsp;&nbsp;&nbsp;<a href=\"/?led=1\">Turn LED off!</a>&nbsp;\
  </body>\
</html>";
//<input type=\"text\" name=\'{\"hello\": \"world\", \"trash\": \"\' value=\'\"}\'><br>&nbsp;<br>\

void handleRoot() {
  //digitalWrite(led, 0);
  if (server.hasArg("led")) {
    digitalWrite (led, server.arg("led").toInt());
  }

  server.send(200, "text/html", postForms);
  //digitalWrite(led, 1);
}

void handlePlain() {
  if (server.method() != HTTP_POST) {
    //digitalWrite(led, 0);
    server.send(405, "text/plain", "Method Not Allowed");
    //digitalWrite(led, 1);
  } else {
    //digitalWrite(led, 0);
    server.send(200, "text/plain", "POST body was:\n" + server.arg("plain"));
    // String message = " ";
    // for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
    // server.send(200, "text/plain", "POST body was:\n" + message);
    //digitalWrite(led, 1);
  }
}

void handleForm() {
  if (server.method() != HTTP_POST) {
    //digitalWrite(led, 0);
    server.send(405, "text/plain", "Method Not Allowed");
    //digitalWrite(led, 1);
  } else {
    //digitalWrite(led, 0);
    String message = "POST form was:\n";
    for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
    server.send(200, "text/plain", message);
    //digitalWrite(led, 1);
  }
}

void handleNotFound() {
  //digitalWrite(led, 0);
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
  //digitalWrite(led, 1);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0); // turn LED on
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
  WiFi.softAP(ap_ssid, ap_password);  // Start the access point
  IPAddress local_ip(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.begin(sta_ssid, sta_password); // Connect to existing network
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

  if (MDNS.begin("esp8266")) { Serial.println("MDNS responder started"); }

  server.on("/", handleRoot);
  server.on("/led=1", handleRoot);
  server.on("/led=0", handleRoot);
  server.on("/postplain/", handlePlain);
  server.on("/postform/", handleForm);
  // server.on ( "/inline", []() {
  //   server.send ( 200, "text/plain", "this works as well" );
  // } );
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(led, 1); // turn LED off
}

void loop(void) {
  server.handleClient();
}
