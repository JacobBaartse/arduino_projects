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
#include <SPI.h>

#include <Wire.h>
#include <Adafruit_GFX.h> // already included from font file
#include "font_16pix_high.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include <Adafruit_SH110X.h> // Adafruit SH110X by Adafruit
#include "printf.h"
#include <Servo.h>

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

Servo ServoRotate; // 360 degrees servo
Servo ServoDirection; // 180 degrees servo

#define radioChannel 106

#define LEDpin1 4
#define LEDpin2 5
#define LEDpin3 6

int status = WL_IDLE_STATUS;

WiFiServer server(80);
IPAddress IPhere;

char ssid[] = "UNO_R4_AP_RF"; // your network SSID (name)
char pass[] = "TBD PW for UNO_R4_AP_RF"; // your network password

uint8_t WiFichannel = 13; // WiFi channel (1-13), 6 seems default

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
  uint8_t nodeId;
  uint8_t detection;
  uint8_t distance;
};
 
// // Payload from/for joystick
// typedef struct {
//   unsigned long keyword;
//   unsigned long timing;
//   unsigned int xvalue;
//   unsigned int yvalue;
//   unsigned int bvalue;
// } joystick_payload; // payload_from_joystick;

// Payload from/for joystick
struct joystick_payload{
  uint32_t keyword;
  uint32_t timing;
  uint16_t xvalue;
  uint16_t yvalue;
  uint8_t bvalue;
  uint8_t sw1value;
  uint8_t sw2value;
};

// Payload from/for keypad
struct keypad_payload{
  uint32_t keyword;
  uint32_t timing;
  char keys[11];
};


unsigned long displayTimer = 0;
uint32_t counter = 0;
bool relay1 = false;
bool relay2 = false;
bool meshrunning = false;

// void restart_arduino(){
//   Serial.println(F("Restart the arduino UNO board..."));
//   delay(2000);
//   NVIC_SystemReset();
// }

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
  if (clear) display.clearDisplay();
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

String Line1 = "Welcome @ demo"; 
String Line2 = "George & Jacob"; 
String Line3 = "radio control";  
String Line4 = "Joystick/keypad";  

int prevx, x, minX;
int dy1, dy2, dy3, dy4, minY;
bool oncecompleted = false;

void setup() {
  pinMode(LEDpin1, OUTPUT);
  pinMode(LEDpin2, OUTPUT);
  pinMode(LEDpin3, OUTPUT);
  digitalWrite(LEDpin2, HIGH);
  ServoRotate.attach(9);  // attaches the servo on pin 9 to the servo object
  ServoDirection.attach(10);  // attaches the servo on pin 10 to the servo object

  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  delay(1000);
  Serial.println(F("Starting UNO R4 WiFi"));
  Serial.flush();

  Serial.print(__FILE__);
  Serial.print(F("\n, creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 
  
  display.begin(i2c_Address, true); // Address 0x3C default
  displaystatus = setDisplay(DisplayState::Dim);
  display.clearDisplay();
  display.setFont(&font_16_pix);
  display.setTextSize(1); // 4 lines of 13-16 chars
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);
  display.display();

  x = display.width();
  dy1 = 16;
  dy2 = 32;
  dy3 = 48;
  dy4 = 64;
  // minX = -128;
  minX = -200; // depends on length of the text
  minY = -22;

  display_oled(true, 0, dy1, Line1); 
  display_oled(false, 0, dy2, Line2); 
  display_oled(false, 0, dy3, Line3);  
  display_oled(false, 0, dy4, Line4);  
  //display.display();
  prevx = x;

  //RTC.begin();
  matrix.begin();
  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware not responding."));
    while (true) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_1MBPS); 
  network.begin(radioChannel, this_node);

  digitalWrite(LEDpin1, HIGH);

  // String fv = WiFi.firmwareVersion();
  // if (fv < WIFI_FIRMWARE_LATEST_VERSION){
  //   Serial.println(F("Please upgrade the firmware for the WiFi module"));
  // }

  // print the network name (SSID);
  Serial.print(F("Creating access point named: "));
  Serial.print(ssid);
  Serial.print(F(", password: '"));
  Serial.print(pass);
  Serial.println(F("'"));

  // by default the local IP address will be 192.168.4.1
  // you can override it with the following:
  WiFi.config(IPAddress(192,168,12,3));
  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid, WiFichannel); // no password needed
  //status = WiFi.beginAP(ssid, pass, WiFichannel); // password required
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true) delay(1000);
  }

  // wait 10 seconds for connection:
  delay(10000);
  // start the web server on port 80
  server.begin();
  // you're connected now, so print out the status
  IPhere = printWiFiStatus();

  Serial.println(F("Created access point available"));

  startupscrollingtext(String("-->: ") + IPhere.toString());

  //Serial.println(F("\nStart connection to time reference"));
  //get_time_from_hsdesign();
  // Retrieve the date and time from the RTC and print them
  //RTCTime currentTime;
  //RTC.getTime(currentTime); 
  digitalWrite(LEDpin3, HIGH);
  //Serial.println(F("The time was just set to: "));
  //Serial.println(currentTime);

  // display.setTextWrap(true);
  // display_oled(true, 0, dy1, currentTime); 
  // delay(1500);

  Serial.println(F("\n ***************\n"));  
  Serial.flush(); 

  display_oled(true, 0, dy2, IPhere.toString());
  display.setTextWrap(false);
  //clear_display();

  printf_begin();
}
 
WiFiClient client;
char c = '\n';
String displaychar = "o";
String currentLine = "";
unsigned long acounter = 0;
unsigned long remacounter = 0;
bool sendDirect = false;
unsigned long looptiming = 0;
uint16_t xpos = 0;
uint16_t ypos = 0;
uint16_t sxpos = 0;
uint16_t sypos = 0;
bool continuousclear = false;
unsigned long chartimer = 0;

void loop() {

  looptiming = millis();

  network.update();
 
  // Check for incoming data from the sensors
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      case 'J': // Message received from Joystick 
        Serial.print(F("Message received from Joystick: "));
        joystick_payload jpayload;
        network.read(header, &jpayload, sizeof(jpayload));
        Serial.println(looptiming);

        // Serial.print(F("Keyword: 0x"));
        // Serial.print(jpayload.keyword, HEX);
        // Serial.print(F(", timing: "));
        // Serial.print(jpayload.timing);
        Serial.print(F("xvalue: "));
        Serial.print(jpayload.xvalue);
        Serial.print(F(", yvalue: "));
        Serial.print(jpayload.yvalue);
        Serial.print(F(", bvalue: "));
        Serial.print(jpayload.bvalue);
        Serial.print(F(", sw1value: "));
        Serial.print(jpayload.sw1value);        
        Serial.print(F(", sw2value: "));
        Serial.println(jpayload.sw2value);

        sxpos = map(jpayload.xvalue, 0, 1023, 0, 180);
        sypos = map(jpayload.yvalue, 0, 1023, 0, 180);
        ServoRotate.write(sypos);
        ServoDirection.write(sxpos);

        xpos = map(jpayload.xvalue, 0, 1023, 0, 110);
        ypos = map(jpayload.yvalue, 0, 1023, 10, 64);
        if (jpayload.bvalue > 10){
          Serial.print(F("clear_display"));
          clear_display();
          continuousclear = !continuousclear;
        }
        if (jpayload.sw1value > 0){
          if (looptiming > chartimer){
            chartimer = looptiming + 3000;
            displaychar = "_";
          }
        }
        if (jpayload.sw2value > 0){
          if (looptiming > chartimer){
            chartimer = looptiming + 3000;
            displaychar = "|";
          }
        }

        display_oled(continuousclear, xpos, ypos, displaychar); 

        //Serial.println((char*)&jpayload);
        // Serial.println(F("--:"));
        // for(int i = 0; i < sizeof(jpayload); i++)
        // {
        //   //Serial.print(((char*)&jpayload)[i]);
        //   Serial.printf("%02x ",((char*)&jpayload)[i]);
        // }
        // Serial.println(F("<--"));
        break;
      // Display the incoming millis() values from sensor nodes

      case 'K': // Message received from Keypad       
        Serial.print(F("Message received from Keypad: "));
        keypad_payload kpayload;
        network.read(header, &kpayload, sizeof(kpayload));
        Serial.println(looptiming);

        if (kpayload.keys[0] > 0){
          // Serial.print(F("Keyword: 0x"));
          // Serial.print(kpayload.keyword, HEX);
          // Serial.print(F(", timing: "));
          // Serial.print(kpayload.timing);
          Serial.print(F("Keys: "));
          Serial.println(kpayload.keys);
        }
      break;

      // case 'M': 
      //   payload_from_slave payload;
      //   network.read(header, &payload, sizeof(payload));
      //   Serial.print(F("Received from Slave nodeId: "));
      //   Serial.print(payload.nodeId);
      //   Serial.print(F(", timing: "));
      //   Serial.println(payload.timing);
      //   if (payload.keyword == keywordvalS) {
      //     if(payload.nodeId == 1){
      //       Serial.print(F("presence: "));
      //       Serial.print(payload.detection);
      //       String distanceString = "Dist.: ";
      //       distanceString += payload.distance;
      //       distanceString += " cm";
      //       Serial.print(F(", "));
      //       Serial.println(distanceString);
              
      //       display_oled(true, 0, dy1, distanceString); 
      //       if (payload.detection > 0){
      //         display_oled(false, 50, dy3, "O"); 
      //       }
      //       else{
      //         display_oled(false, 20, dy3, "--"); 
      //       }  
      //     }        
      //   }
      //   else{
      //     Serial.println("Wrong keyword"); 
      //   }
      //   break;

      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.println(header.type);
    }

  }
  // end of while network.available

  client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println(F("reading from client"));        // print a message out the serial port
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

            // client.print(F("</TABLE><TABLE><TR><TH colspan=3><a href=\"/Z\">RELAYs</a></TH></TR>"));
            // client.print(F("<TR><TD>1</TD><TD><a href=\"/1R\">ON</a></TD><TD><a href=\"/1K\">off</a></TD></TR>"));
            // client.print(F("<TR><TD>2</TD><TD><a href=\"/2R\">ON</a></TD><TD><a href=\"/2K\">off</a></TD></TR>"));
            // client.print(F("</TABLE></BODY></HTML>"));

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


// void PrintString(const char *str) {
//     const char *p;
//     p = str;
//     while (*p) {
//         Serial.print(*p);
//         p++;
//     }
// }

// void PrintString(const char *str) {
// const char *p;
// p = str;
// while (*p) {
// Serial.print(*p);
// p++;
// }
// }
