/*
 *
 */

#include "matrix.h"
#include "networking.h"
// #include "WiFiS3.h" // already included in networking.h
#include "RTC.h"
#include "clock.h"

#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>
 
#define radioChannel 96
/** User Configuration per 'slave' node: nodeID **/
#define slavenodeID 3
#define masterNodeID 0

#define LEDpin1 4
#define LEDpin2 5
#define LEDpin3 6

WiFiServer server(80);
IPAddress IPhere;

RF24Mesh mesh(radio, network);
 
unsigned long const keywordvalM = 0xfeebbeef; 
unsigned long const keywordvalS = 0xbeeffeeb; 

// Payload from/for MASTER
struct payload_from_master {
  unsigned long keyword;
  uint32_t counter;
  bool relay1;
  bool relay2;
};
 
// Payload from/for SLAVES
struct payload_from_slave {
  unsigned long keyword;
  uint32_t timing;
  bool relayActive;
  uint8_t nodeId;
};
 
uint32_t displayTimer = 0;
uint32_t counter = 0;
bool relay1 = false;
bool relay2 = false;
bool meshrunning = false;

void restart_arduino(){
  Serial.println(F("Restart the arduino UNO board..."));
  delay(2000);
  NVIC_SystemReset();
}

bool meshstartup(){
  if (meshrunning){
    Serial.println(F("Radio issue, turn up PA level?"));
  }
  return mesh.begin(radioChannel);
}

void LEDstatustext(bool LEDon, unsigned long count){
  static unsigned long bcount = 0;
  if (count != bcount){ // update display only once
    String TextHere = "_"; // "_--  ";
    if (LEDon) TextHere = "^"; // "oO0  ";
    TextHere = TextHere + (count % 10);
    Serial.println();
    Serial.print(TextHere);

    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.println(TextHere);
    matrix.endText();
    bcount = count;
  }
}

void setup() {
  pinMode(LEDpin1, OUTPUT);
  pinMode(LEDpin2, OUTPUT);
  pinMode(LEDpin3, OUTPUT);
  digitalWrite(LEDpin2, HIGH);

  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  
  RTC.begin();
  matrix.begin();
  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware not responding."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);

  // Set the nodeID to 0 for the master node
  mesh.setNodeID(masterNodeID);
  Serial.print(F("Starting Master of the mesh, nodeID: "));
  Serial.println(mesh.getNodeID());
  // Connect to the mesh
  meshrunning = meshstartup();

  digitalWrite(LEDpin1, HIGH);
  Serial.print(F("Starting up UNO R4 WiFi"));
  Serial.flush();

  String timestamp = __TIMESTAMP__;
  Serial.print(F("Creation/build time: "));
  Serial.println(timestamp);
  Serial.flush(); 

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION){
    Serial.println(F("Please upgrade the firmware for the WiFi module"));
  }

  // attempt to connect to WiFi network:
  int wifistatus = WifiConnect();
  if (wifistatus == WL_CONNECTED){
    server.begin();
    IPhere = printWifiStatus(connection);
  }
  else{ // stop the wifi connection
    WiFi.disconnect();
  }
  startupscrollingtext(String("-->: ") + IPhere.toString());

  Serial.println(F("\nStarting connection to HS Design"));
  get_time_from_hsdesign();
  // Retrieve the date and time from the RTC and print them
  RTCTime currentTime;
  RTC.getTime(currentTime); 
  digitalWrite(LEDpin3, HIGH);
  Serial.println(F("The RTC was just set to: "));
  Serial.println(currentTime);

  Serial.println();  
  Serial.println(F(" ***************"));  
  Serial.println();  
  Serial.flush(); 
}
 
unsigned int mesherror = 0;
uint8_t meshupdaterc = 0;
uint8_t rem_meshupdaterc = 200;

WiFiClient client;
char c = '\n';
String currentLine = "";
unsigned long acounter = 0;

void loop() {
  if (mesherror > 8) {
    meshrunning = meshstartup();
    mesherror = 0;
  }
  // Call mesh.update to keep the network updated
  meshupdaterc = mesh.update();
  if (meshupdaterc != rem_meshupdaterc) {
    Serial.print(F("meshupdaterc: "));
    Serial.println(meshupdaterc);
    rem_meshupdaterc = meshupdaterc;
  }

  // In addition, keep the 'DHCP service' running 
  // on the master node so addresses will
  // be assigned to the sensor nodes
  mesh.DHCP();
 
  // Check for incoming data from the sensors
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      // Display the incoming millis() values from sensor nodes
      case 'M': 
        payload_from_slave payload;
        network.read(header, &payload, sizeof(payload));
        Serial.print(F("Received from Slave nodeId: "));
        Serial.print(payload.nodeId);
        Serial.print(F(", timing: "));
        Serial.print(payload.timing);
        Serial.print(F(", relayActive: "));
        Serial.println(payload.relayActive);
        if (payload.keyword == keywordvalS) {

        }
        else{
          Serial.println("Wrong keyword"); 
        }
        break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.println(header.type);
    }
  }
  
  // Meanwhile, every x seconds...
  if(millis() - displayTimer > 15000) {
    displayTimer = millis();

    //// SHOW DHCP TABLE - BEGIN
    if (mesh.addrListTop > 0) {
      Serial.println();
      Serial.println(F("******* Assigned Addresses *******"));
      for(int i=0; i<mesh.addrListTop; i++){
        Serial.print(F("NodeID: "));
        Serial.print(mesh.addrList[i].nodeID);
        Serial.print(F(" RF24Network Address: 0")); // this is in octal
        Serial.println(mesh.addrList[i].address, OCT);
      }
      Serial.println(F("**********************************"));
    }
    else{
      Serial.print(F(" ."));
    }
    //// SHOW DHCP TABLE - END

    //// Send same master message to all slaves - BEGIN
    if (mesh.addrListTop > 0) {
      for(int i=0; i<mesh.addrListTop; i++){
        counter += 1;
        payload_from_master payloadS = {keywordvalM, counter, relay1, relay2};        
        
        // RF24NetworkHeader header(mesh.addrList[i].address, OCT);
        // // int x = network.write(header, &payload, sizeof(payload));
        // network.write(header, &payloadS, sizeof(payloadS));
        
        if (!mesh.write(&payloadS, 'S', sizeof(payloadS), mesh.addrList[i].nodeID)) {
          Serial.print(F("Send fail, Master to Slave, nodeID: "));
          Serial.print(mesh.addrList[i].nodeID);
          Serial.println();
          mesherror++;
        }
        else {
          Serial.print(F("Send to Slave Node "));
          Serial.print(mesh.addrList[i].nodeID);
          Serial.print(F(" OK: "));
          Serial.println(payloadS.counter);
          mesherror = 0;
        }
      }
    }
    else{
      mesherror++;
      Serial.print(F("No network node to write to ("));
      Serial.print(mesherror);
      Serial.println(F(")"));
    }
    //// Send same master message to all slaves - END
  }
  // end of while network.available

  client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println(F("new client"));           // print a message out the serial port
    currentLine = "";                       // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        c = client.read();                  // read a byte, then
        Serial.write(c);                    // print it out to the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println(F("HTTP/1.1 200 OK"));
            client.println(F("Content-type:text/html"));
            client.println();

            // the content of the HTTP response follows the header:
            // client.print("<p style=\"font-size:7vw;\">LED<br></p>");
            // client.print("<p style=\"font-size:7vw;\"><a href=\"/H\">ON</a><br></p>");
            // client.print("<p style=\"font-size:7vw;\"><a href=\"/L\">off</a><br></p>");

            client.print(F("<HTML><HEAD><TITLE>Arduino UNO R4 WiFi</TITLE><META content=\"text/html; charset=iso-8859-1\" http-equiv=Content-Type>"));
            client.print(F("<META HTTP-EQUIV=Expires CONTENT=\"Sun, 16-Apr-2028 01:00:00 GMT\"><link rel=\"icon\" href=\"data:,\"></HEAD>")); 
            client.print(F("<BODY TEXT=\"#873e23\" LINK=\"#1f7a1f\" VLINK=\"#1f7a1f\" ALINK=\"#1f7a1f\" BGCOLOR=\"#bb99ff\">"));
            client.print(F("<TABLE style=\"width:100%\"><TR style=\"height:200px; font-size:4em;\"><TH colspan=3 style=\"text-align: center\"><a href=\"/T\">LEDs</a></TH></TR>"));
            client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: center\">1</TD><TD style=\"text-align: center\"><a href=\"/1H\">ON</a></TD><TD style=\"text-align: center\"><a href=\"/1L\">off</a></TD></TR>"));
            client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: center\">2</TD><TD style=\"text-align: center\"><a href=\"/2H\">ON</a></TD><TD style=\"text-align: center\"><a href=\"/2L\">off</a></TD></TR>"));
            client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: center\">3</TD><TD style=\"text-align: center\"><a href=\"/3H\">ON</a></TD><TD style=\"text-align: center\"><a href=\"/3L\">off</a></TD></TR>"));
            client.print(F("</TABLE><TABLE>"));
            client.print(F("<TABLE style=\"width:100%\"><TR style=\"height:200px; font-size:4em;\"><TH colspan=3 style=\"text-align: center\"><a href=\"/Z\">RELAYs</a></TH></TR>"));
            client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: center\">1</TD><TD style=\"text-align: center\"><a href=\"/1R\">ON</a></TD><TD style=\"text-align: center\"><a href=\"/1K\">off</a></TD></TR>"));
            client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: center\">2</TD><TD style=\"text-align: center\"><a href=\"/2R\">ON</a></TD><TD style=\"text-align: center\"><a href=\"/2K\">off</a></TD></TR>"));
            client.print(F("</TABLE></BODY></HTML>"));

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else { // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /1H")) {
          digitalWrite(LEDpin1, HIGH);               // GET /H turns the LED on
          acounter += 1;
        }
        if (currentLine.endsWith("GET /1L")) {
          digitalWrite(LEDpin1, LOW);                // GET /L turns the LED off
          acounter += 1;
        }
        if (currentLine.endsWith("GET /2H")) {
          digitalWrite(LEDpin2, HIGH);               // GET /H turns the LED on
          acounter += 1;
        }
        if (currentLine.endsWith("GET /2L")) {
          digitalWrite(LEDpin2, LOW);                // GET /L turns the LED off
          acounter += 1;
        }
        if (currentLine.endsWith("GET /3H")) {
          digitalWrite(LEDpin3, HIGH);               // GET /H turns the LED on
          acounter += 1;
        }
        if (currentLine.endsWith("GET /3L")) {
          digitalWrite(LEDpin3, LOW);                // GET /L turns the LED off
          acounter += 1;
        }
        if (currentLine.endsWith("GET /T")) {
          digitalWrite(LEDpin1, !digitalRead(LEDpin1));  // GET /T toggles the LED
          digitalWrite(LEDpin2, !digitalRead(LEDpin2));  // GET /T toggles the LED
          digitalWrite(LEDpin3, !digitalRead(LEDpin3));  // GET /T toggles the LED
          acounter += 1;
        }

        if (currentLine.endsWith("GET /1R")) {
          relay1 = true;  
          acounter += 1;
        }
        if (currentLine.endsWith("GET /1K")) {
          relay1 = false;  
          acounter += 1;
        }
        if (currentLine.endsWith("GET /2R")) {
          relay2 = true;  
          acounter += 1;
        }
        if (currentLine.endsWith("GET /2K")) {
          relay2 = false;  
          acounter += 1;
        }
        if (currentLine.endsWith("GET /Z")) {
          relay1 = !relay1;  // GET /Z toggles the relays
          relay2 = !relay2;  // GET /Z toggles the relays
          acounter += 1;
        }

        // if (currentLine.endsWith("GET /")) {  // home page gets triggered as well
        //   acounter += 1;
        // }
        LEDstatustext(digitalRead(LEDpin1), acounter);

        if (currentLine.endsWith("GET /favicon.ico")) {
          // sendFavicon();
          client.println(F("HTTP/1.1 404 Not Found\nConnection: close\n\n"));
        }      
      }
      else {
        //delay(1000); 
        Serial.println(F("breaking from loop"));
        break; // break from loop and disconnect client
      }
    }

    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
    //delay(1000); // make sure the disconnection is detected
  
  }

}
