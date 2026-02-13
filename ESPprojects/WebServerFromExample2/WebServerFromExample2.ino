/*
  Based on 
  https://tttapa.github.io/ESP8266/Chap10%20-%20Simple%20Web%20Server.html

 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

const int led = 2;

const String Homepage = "<html>\
  <head>\
    <title>Web Server SO 148</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <form action=\"/LED\" method=\"POST\"><input type=\"submit\" value=\"Toggle LED\"></form><BR>\
    <form action=\"/LEDon\" method=\"POST\"><input type=\"submit\" value=\"LED on\"></form>&nbsp;&nbsp;&nbsp;&nbsp;\
    <form action=\"/LEDoff\" method=\"POST\"><input type=\"submit\" value=\"LED off\"></form><BR>\
    <form action=\"/login\" method=\"POST\">\
        <input type=\"text\" name=\"username\" placeholder=\"Username\">&nbsp;\
        <input type=\"password\" name=\"password\" placeholder=\"Password\">&nbsp;\
        <input type=\"submit\" value=\"Login\">\
    </form><br><hr>\
  </body>\
</html>";

    // <h1>POST plain text</h1><br>\
    // <form method=\"post\" enctype=\"text/plain\" action=\"/postplain/\">\
    //   <input type=\"text\" name=\"plain\": value=\"plain text\"><br>&nbsp;<br>\
    //   <input type=\"submit\" value=\"Submit\">\
    // </form>\
    // <h1>POST form data</h1><br>\
    // <form method=\"post\" enctype=\"application/x-www-form-urlencoded\" action=\"/postform/\">\
    //   <input type=\"text\" name=\"hello\" value=\"world\"><br>&nbsp;<br>\
    //   <input type=\"submit\" value=\"Submit\">\
    // </form>\

void handleRoot();              // function prototypes for HTTP handlers
void handleLED();
void handleLEDon();
void handleLEDoff();
void handleLogin();
void handleNotFound();

void setup(void){
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.println(F("Starting ESP device"));
  Serial.flush();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  pinMode(led, OUTPUT);

  wifiMulti.addAP("T24_optout", "T24T24T24");   // add Wi-Fi networks you want to connect to
  // wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  // wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  server.on("/", HTTP_GET, handleRoot);     // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/LED", HTTP_POST, handleLED);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  server.on("/LEDon", HTTP_POST, handleLEDon);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  server.on("/LEDoff", HTTP_POST, handleLEDoff);  // Call the 'handleLED' function when a POST request is made to URI "/LED"
  server.on("/login", HTTP_POST, handleLogin); // Call the 'handleLogin' function when a POST request is made to URI "/login"
  //server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.onNotFound([](){
    server.send(404, "text/plain", "404: Not found");
  });
  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();                    // Listen for HTTP requests from clients
}

void handleRoot() {                         // When URI / is requested, send a web page with a button to toggle the LED
  server.send(200, "text/html", Homepage);
}

void handleLED() {                          // If a POST request is made to URI /LED
  digitalWrite(led, !digitalRead(led));     // Change the state of the LED
  server.sendHeader("Location", "/");       // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleLEDon() {                          // If a POST request is made to URI /LED
  digitalWrite(led, 0);     // Change the state of the LED
  server.sendHeader("Location", "/");       // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleLEDoff() {                          // If a POST request is made to URI /LED
  digitalWrite(led, 1);     // Change the state of the LED
  server.sendHeader("Location", "/");       // Add a header to respond with a new location for the browser to go to the home page again
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}

void handleLogin() {                         // If a POST request is made to URI /login
  if( ! server.hasArg("username") || ! server.hasArg("password") 
      || server.arg("username") == NULL || server.arg("password") == NULL) { // If the POST request doesn't have username and password data
    server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
    return;
  }
  if(server.arg("username") == "John Doe" && server.arg("password") == "password123") { // If both the username and the password are correct
    server.send(200, "text/html", "<h1>Welcome, " + server.arg("username") + "!</h1><p>Login successful</p>");
  } else {                                                                              // Username and password don't match
    server.send(401, "text/plain", "401: Unauthorized");
  }
}

// void handleNotFound(){
//   server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
// }
