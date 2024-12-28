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

#include <Wire.h>
//#include <Adafruit_GFX.h> // already included from font file
#include "FreeSerif12pt7b_special.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include <Adafruit_SH110X.h> // Adafruit SH110X by Adafruit


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3C //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3D //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define OLED_RESET -1

enum DisplayState {
    Off = 0,
    Dim = 1,
    On = 2,
};

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
 
unsigned long displayTimer = 0;
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
  return mesh.begin(radioChannel, RF24_250KBPS);
}

void LEDstatustext(bool LEDon, unsigned long count){
  static unsigned long bcount = 0;
  if (count != bcount){ // update display only once
    String TextHere = "_"; // "_--  ";
    if (LEDon) TextHere = "^"; // "oO0  ";
    TextHere = TextHere + (count % 10);
    Serial.println();
    Serial.print(TextHere);
    Serial.println();

    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.println(TextHere);
    matrix.endText();
    bcount = count;
  }
}


bool displaystatus = DisplayState::Off;
void display_oled(bool clear, int x, int y, String text) {
  if (displaystatus == DisplayState::Off) return;
  if (clear) clear_display();
  display.setCursor(x, y);
  display.print(text);
  display.display();
}

// // move text, write old location in background color
// void display_move(int x, int y, int nx, int ny, String text) {
//   if (displaystatus == DisplayState::Off) return;
//   display.setCursor(x, y);
//   display.setTextColor(SH110X_BLACK);
//   display.print(text);
//   display.setCursor(nx, ny);
//   display.setTextColor(SH110X_WHITE);
//   display.print(text);
//   display.display();
// }

DisplayState setDisplay(DisplayState statustoset){
  static DisplayState displaystatus = DisplayState::Dim;
  switch(statustoset){
    case DisplayState::Dim:
      display.oled_command(SH110X_DISPLAYON);
      display.setContrast(0); // dim display
      displaystatus = DisplayState::Dim;
      break;
    case DisplayState::On:
      display.oled_command(SH110X_DISPLAYON);
      displaystatus = DisplayState::On;
      break;
    //case DisplayState::Off:
    default:
      display.oled_command(SH110X_DISPLAYOFF);
      displaystatus = DisplayState::Off;
  }
  return displaystatus;
}

void clear_display(){
  display.clearDisplay();
  display.display();
}

// String leftrotate(String str, int d){
//    String ans = str.substring(d, str.length() - d) + str.substring(0, d);
//    return ans;
// }

String Line1 = "Welcome \x81"; 
String Line2 = "Demo \x81{characters} \x81"; 
String Line3 = "Whats \x81 up?";  

int prevx, x, minX;
int dy1, dy2, dy3, minY;
bool oncecompleted = false;

void setup() {
  pinMode(LEDpin1, OUTPUT);
  pinMode(LEDpin2, OUTPUT);
  pinMode(LEDpin3, OUTPUT);
  digitalWrite(LEDpin2, HIGH);

  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  display.begin(i2c_Address, true); // Address 0x3C default
  displaystatus = setDisplay(DisplayState::Dim);
  display.clearDisplay();
  display.setFont(&FreeSerif12pt7b);
  display.setTextSize(1); // 3 lines of 10-12 chars
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);
  display.display();

  x = display.width();
  dy1 = 16;
  dy2 = 38;
  dy3 = 60;
  // minX = -128;
  minX = -200; // depends on length of the text
  minY = -22;

  display_oled(true, 0, dy1, Line1); 
  display_oled(false, 2, dy2, Line2); 
  display_oled(false, 4, dy3, Line3);  
  //display.display();
  prevx = x;

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
  // radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);

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

  Serial.println(F("\nStart connection to time reference"));
  get_time_from_hsdesign();
  // Retrieve the date and time from the RTC and print them
  RTCTime currentTime;
  RTC.getTime(currentTime); 
  digitalWrite(LEDpin3, HIGH);
  Serial.println(F("The RTC was just set to: "));
  Serial.println(currentTime);

  display.setTextWrap(true);
  display_oled(true, 0, dy1, currentTime); 
  //display.display();
  delay(2000);
  display.setTextWrap(false);
  clear_display();

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
unsigned long remacounter = 0;
bool sendDirect = false;

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
  
  
  if(sendDirect){
    Serial.print(F(" send direct about to happen ")); 
    Serial.println(millis());
  }
  // Meanwhile, every x seconds...
  if ((sendDirect) || (millis() - displayTimer > 15000)) {
    displayTimer = millis();
    sendDirect = false;

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
    Serial.println(F("new client"));        // print a message out the serial port
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

            client.print(F("<HTML><HEAD><TITLE>Arduino UNO R4 WiFi</TITLE><META content=\"text/html; charset=iso-8859-1\" http-equiv=Content-Type>"));
            client.print(F("<META HTTP-EQUIV=Expires CONTENT=\"Sun, 16-Apr-2028 01:00:00 GMT\"><link rel=\"icon\" href=\"data:,\">")); 
            client.print(F("<style> table {width:100%;} tr {height:200px; font-size:4em;} th, td {text-align:center;} </style>")); 
            client.print(F("</HEAD><BODY TEXT=\"#873e23\" LINK=\"#1f7a1f\" VLINK=\"#1f7a1f\" ALINK=\"#1f7a1f\" BGCOLOR=\"#bb99ff\">"));

            client.print(F("<TABLE><TR><TH colspan=3><a href=\"/T\">LEDs</a></TH></TR>"));
            client.print(F("<TR><TD>1</TD><TD><a href=\"/1H\">ON</a></TD><TD><a href=\"/1L\">off</a></TD></TR>"));
            client.print(F("<TR><TD>2</TD><TD><a href=\"/2H\">ON</a></TD><TD><a href=\"/2L\">off</a></TD></TR>"));
            client.print(F("<TR><TD>3</TD><TD><a href=\"/3H\">ON</a></TD><TD><a href=\"/3L\">off</a></TD></TR>"));

            client.print(F("</TABLE><TABLE><TR><TH colspan=3><a href=\"/Z\">RELAYs</a></TH></TR>"));
            client.print(F("<TR><TD>1</TD><TD><a href=\"/1R\">ON</a></TD><TD><a href=\"/1K\">off</a></TD></TR>"));
            client.print(F("<TR><TD>2</TD><TD><a href=\"/2R\">ON</a></TD><TD><a href=\"/2K\">off</a></TD></TR>"));
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

        remacounter = acounter;
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
        if (currentLine.endsWith("GET /T")) { // GET /T toggles the LEDs
          digitalWrite(LEDpin1, !digitalRead(LEDpin1));  
          digitalWrite(LEDpin2, !digitalRead(LEDpin2));  
          digitalWrite(LEDpin3, !digitalRead(LEDpin3)); 
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
        if (currentLine.endsWith("GET /Z")) { // GET /Z toggles the relays
          relay1 = !relay1;  
          relay2 = !relay2;
          acounter += 1;
        }

        // if (currentLine.endsWith("GET /")) { // home page gets triggered as well
        //   acounter += 1;
        // }
        if(!sendDirect){ // make sure it is not set to false during this loop
          sendDirect = (remacounter != acounter);
          if(sendDirect){
            Serial.print(F(" sendDirect var set "));
            Serial.println(millis());
          }
        }
        LEDstatustext(sendDirect, acounter);

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
