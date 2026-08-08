#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "serials.h"

#ifndef STASSID
#define STASSID "T24_optout"
#define STAPSK "T24T24T24"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const int led = LED_BUILTIN;

const String startsection = "<!DOCTYPE HTML><html><head><title>Controller internet gateway</title> \
      <style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style> \
      </head><h1>Running and can connect to internet</h1><br><br>";
const String endsection = "</body></html>";
const String GWhtml = "<a href=\"/GW\">Link item 1</a>";
const String BChtml = "<a href=\"/BC\">Second 2 link</a>";

String makewebpagehtml(){ // to be enhanced, array processing
  String htmlpage = startsection;
  htmlpage += F("internet gateway trial<BR><BR>");
  htmlpage += F("For now 2 items/links which can be clicked");
  htmlpage += F("<BR><BR>");
  htmlpage += GWhtml;
  htmlpage += F("<BR><BR>");
  htmlpage += BChtml;
  htmlpage += F("<BR><HR>");
  htmlpage += endsection;
  // Serial.print(htmlpage);
  return htmlpage;
}

ESP8266WebServer server(80);
unsigned long runningtime = 0;

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
  Serial.println(F("handle link 1"));

  // toggle LED or so
  sendserialtext("link 1");
  sendserialtext("from internet connection");
  sendserialhex(0xdeadbeef, 14);
  sendserialhex(0xdeadbeef, 1);
  sendserialhex(runningtime, 15);

  String webpage = makewebpagehtml(); // include the current status information
  server.send(200, "text/html", webpage);
  Serial.println(F(" "));
}

const char webmsg[] = "webcontrol message";

void handleBC() {
  Serial.println(F("link 2"));

  sendserialtext("link 2");
  sendserialtext("from internet connection");
  sendserialhex(0xdeadbeef, 2);
  sendserialhex(0xdeadbeef, 13);
  sendserialhex(runningtime, 15);

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

  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/BC", handleBC);
  server.on("/GW", handleGW);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  digitalWrite(led, 1); // turn onboard LED off
}

const char msg[] = "Hello from Gateway !";
const char buttonmsg[] = "Button pressed (GW1).";
bool action = false;
bool newdata = false;

// --------------------
// Main Loop
// --------------------
void loop() {
  
  runningtime = millis();

  action = timepassing(runningtime, 30000);
  if (action){
      // something on the serial port to the GatewayLocal
  }

  server.handleClient();

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

    }
    newdata = false;
  }

}
