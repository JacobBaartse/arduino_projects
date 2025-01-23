/*
  WiFi Web Server Control Color Display

  A simple web server that lets you control a Color Display via a local network.
  This sketch will create a new access point (with no password).
  It will then launch a new server and print out the IP address
  to the Serial Monitor. From there, you can open that address in a web browser
  to use the controls on the web page.

  Find the full UNO R4 WiFi Network documentation here:
  https://docs.arduino.cc/tutorials/uno-r4-wifi/wifi-examples#access-point

 */
/*
 *
 * default ip address: 192.168.12.2
 *
 */

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#include "WiFiS3.h"
// #include <Ethernet.h>
// #include <StreamLib.h>  
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library

// ST7789 TFT module connections
#define TFT_CS    10  // define chip select pin
#define TFT_DC     9  // define data/command pin
#define TFT_RST    8  // define reset pin, or set to -1 and connect to Arduino RESET pin

// Initialize Adafruit ST7789 TFT library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

ArduinoLEDMatrix matrix;

char ssid[] = "UNO_R4_AP_wCD"; // your network SSID (name)
uint8_t WiFichannel = 13; // WiFi channel (1-13), 6 seems default

WiFiServer server(80);
IPAddress IPhere;

void startupscrollingtext(String starttext){
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(100);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(starttext);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
}

//const uint32_t clear[] = {0x19819, 0x80000001, 0x81f8000};

void Colorstatustext(int color){
  static unsigned long bcolor = 99;
  if (color != bcolor){ // update display only once
    //matrix.loadFrame(clear);
    matrix.textFont(Font_4x6);

    String TextHere = String(color) + "  "; 

    Serial.println(F(""));
    Serial.print(TextHere);
    Serial.println(F(" -"));

    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.println(TextHere);
    matrix.endText();

    bcolor = color;
  }
}

IPAddress printWiFiStatus() {
  // print the SSID of the network you're hosting (Access Point mode)
  Serial.print("SSID: ");
  Serial.print(WiFi.SSID());

  // print your AP IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(", IP address: ");
  Serial.print(ip);

  // print where to go in a browser:
  Serial.print(", browse to http://");
  Serial.println(ip);
  
  return ip;
}

int status = WL_IDLE_STATUS;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("Access Point Web Server with Color Display"));
  matrix.begin();

  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
  // pinMode(led, OUTPUT); // set the LED pin mode

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the WiFi firmware");
  }

  String timestamp = __TIMESTAMP__;
  Serial.print(F("Creation/build time: "));
  Serial.println(timestamp);
  Serial.flush(); 

  // print the network name (SSID);
  Serial.print(F("Creating access point named: "));
  Serial.println(ssid);

  // by default the local IP address will be 192.168.4.1
  // you can override it with the following:
  WiFi.config(IPAddress(192,168,12,2));

  // Create open network. Change this line if you want to create an WEP network:
  //status = WiFi.beginAP(ssid, pass);
  //status = WiFi.beginAP(ssid); // no password needed
  status = WiFi.beginAP(ssid, WiFichannel);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }

  tft.init(240, 240, SPI_MODE2);    // Init ST7789 display 240x240 pixel
  // if the screen is flipped, remove this command
  tft.setRotation(2);
  Serial.println(F("Initialized Color display"));
  tft.fillScreen(ST77XX_BLACK);
  // wait some seconds for connection:
  delay(5000);
  digitalWrite(7, HIGH);

  // start the web server on port 80
  server.begin();
  // you're connected now, so print out the status
  IPhere = printWiFiStatus();

  Serial.println(F("Created access point available"));

  tft.setTextWrap(false);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);
  tft.println(IPhere.toString());
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);

  startupscrollingtext(String("-->: ") + IPhere.toString());

  Serial.println(F(" "));  
  Serial.println(F(" *************** "));  
  Serial.println(F(" "));  
  Serial.flush(); 
}

WiFiClient client;

char c = '\n';
String currentLine = "";
unsigned long acounter = 0;
unsigned long currenttime = 0;
unsigned long screentiming = 0;
bool actiontodo = false;
String actiontext = "";
int currentmeta = 0;


void loop() {
  
  // compare the previous status to the current status
  // newstatus = WiFi.status();
  
  currenttime = millis();

  // // screensaver
  // if (currenttime - screentiming > 10000) {
  //   tft.fillScreen(ST77XX_BLACK);
  //   screentiming = currenttime;
  // }

  client = server.available();              // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println(F("client:"));           // print a message out the serial port
    if (currenttime - screentiming > 1000) {
      tft.fillScreen(ST77XX_BLACK);
      screentiming = currenttime;
    }
    currentmeta = 0;
    currentLine = "";                       // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        c = client.read();                  // read a byte, then
        //Serial.write(c);                    // print it out to the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() < 1) {
            HTMLreply();
            //break;
          }
          //else { // if there is a newline, then print, process and clear currentLine
            tft.println(currentLine);
            HTMLresponseline(currentLine, currentmeta);
            currentmeta = 1;
            currentLine = "";
          //}
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // // Check to see if the client request was "GET /H" or "GET /L":
        // if (currentLine.endsWith("GET /H")) {
        //   //digitalWrite(led, HIGH);               // GET /H turns the LED on
        //   acounter += 1;
        // }
        // if (currentLine.endsWith("GET /L")) {
        //   //digitalWrite(led, LOW);                // GET /L turns the LED off
        //   acounter += 1;
        // }
        // if (currentLine.endsWith("GET /T")) {
        //   //digitalWrite(led, !digitalRead(led));  // GET /T toggles the LED
        //   acounter += 1;
        // }
        // // if (currentLine.startsWith("GET /")) {  // text input follows
        // //   acounter += 1;
        // // }

        // if (currentLine.endsWith("GET /favicon.ico")) {
        //   client.println(F("HTTP/1.1 404 Not Found\nConnection: close\n\n"));
        // }      
      }
      else {
        Serial.println(F("\nbreaking from loop"));
        break; // break from loop and disconnect client
      }
    } // while client.connected

    HTMLresponseline(currentLine, 9);
    if (currentLine.length() > 0){
      tft.print(F("Last data: "));
      tft.print(currentLine);
      tft.println(F("<---"));
      currentLine = "";
    }
    tft.setCursor(0, 0);

    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
    //delay(1000); // make sure the disconnection is detected
  }

  if (actiontodo){


    actiontodo = false;
  }
}

void HTMLreply(){
  Serial.print(F("HTMLreply"));

  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-type:text/html"));
  client.println();

  // the content of the HTTP response follows the header:

  client.print(F("<HTML><HEAD><TITLE>Arduino UNO R4 WiFi</TITLE><META content=\"text/html; charset=iso-8859-1\" http-equiv=Content-Type>"));
  client.print(F("<META HTTP-EQUIV=Expires CONTENT=\"Sun, 16-Apr-2028 01:00:00 GMT\"><link rel=\"icon\" href=\"data:,\">")); 
  client.print(F("<style> table {width:100%;} tr {height:200px; font-size:4em;} th, td {text-align:center;} </style>")); 
  client.print(F("</HEAD><BODY TEXT=\"#873e23\" LINK=\"#1f7a1f\" VLINK=\"#1f7a1f\" ALINK=\"#1f7a1f\" BGCOLOR=\"#bb99ff\">"));

  client.print(F("<TABLE><TR><TH colspan=3>Coloring</TH></TR>"));
  client.print(F("<TR><TD><a href=\"/C1\">Red</a></TD><TD><a href=\"/C2\">Green</a></TD><TD><a href=\"/C3\">Blue</a></TD></TR>"));
  client.print(F("<TR><TD><a href=\"/C4\">Cyan</a></TD><TD><a href=\"/C5\">Magenta</a></TD><TD><a href=\"/C6\">Yellow</a></TD></TR>"));
  client.print(F("<TR><TD><a href=\"/C7\">Orange</a></TD><TD><a href=\"/C8\">White</a></TD><TD><a href=\"/C9\">Black</a></TD></TR>"));

  client.print(F("</TABLE><HR><label for=\"textin\">Text input:</label><FORM action=\"textin\" method=\"post\">"));
  client.print(F("<input type=\"text\" id=\"textin\" name=\"textin\" required minlength=\"4\" maxlength=\"80\" size=\"30\"/>"));
  client.print(F("&nbsp;&nbsp;&nbsp;<input type=\"submit\"/>")); // <input type=\"submit\" hidden />
  client.print(F("</FORM><HR></BODY></HTML>"));

  // The HTTP response ends with another blank line:
  client.println();
  Serial.println(F(" done"));
}

int HTMLresponseline(String requestline, int metadata){
  static bool postedtext = false;
  static int postedcolors = 0;

  if (metadata < 1){
    postedtext = false;
    postedcolors = 0;
  }

  if (requestline.length() > 0) {
    Serial.print(F("HTMLresp |"));
    Serial.print(requestline);
    Serial.println(F("|"));

    // check the line for actions

    if (currentLine.startsWith("GET /favicon.ico")) {
      client.println(F("HTTP/1.1 404 Not Found\nConnection: close\n\n"));
    } 
    if (requestline.startsWith("POST /")) {  // text input follows
      postedtext = true;
    }    
    if (requestline.startsWith("GET /C0")) {  // color item
      postedcolors = postedcolors | 0x0200;
    } 
    if (requestline.startsWith("GET /C1")) {  // color item
      postedcolors = postedcolors | 0b00000001;
    }     
    if (requestline.startsWith("GET /C2")) {  // color item
      postedcolors = postedcolors | 0b00000010;
    } 
    if (requestline.startsWith("GET /C3")) {  // color item
      postedcolors = postedcolors | 0b00000100;
    }     
    if (requestline.startsWith("GET /C4")) {  // color item
      postedcolors = postedcolors | 0b00001000;
    }     
    if (requestline.startsWith("GET /C5")) {  // color item
      postedcolors = postedcolors | 0b00010000;
    }     
    if (requestline.startsWith("GET /C6")) {  // color item
      postedcolors = postedcolors | 0b00100000;
    } 
    if (requestline.startsWith("GET /C7")) {  // color item
      postedcolors = postedcolors | 0b01000000;
    } 
    if (requestline.startsWith("GET /C8")) {  // color item
      postedcolors = postedcolors | 0b10000000;
    } 
    if (requestline.startsWith("GET /C9")) {  // color item
      postedcolors = postedcolors | 0x0100;
    } 
 
    /*

    Serial.println(F(" "));
    Serial.println("1. Red");
    Serial.println("2. Green");
    Serial.println("3. Blue");
    Serial.println("4. Cyan");
    Serial.println("5. Magenta");
    Serial.println("6. Yellow");
    Serial.println("7. Orange");
    Serial.println(F(" "));

    switch (menuChoice) {
      case 1:
        TextColor = ST77XX_RED;
        break;
      case 2:
        TextColor = ST77XX_GREEN;
        break;
      case 3:
        TextColor = ST77XX_BLUE;
        break;
      case 4:
        TextColor = ST77XX_CYAN;
        break;
      case 5:
        TextColor = ST77XX_MAGENTA;
        break;
      case 6:
        TextColor = ST77XX_YELLOW;
        break;
      case 7:
        TextColor = ST77XX_ORANGE;
        break;
      default:
        TextColor = ST77XX_WHITE;
        menuChoice = 0;
        /* */
  }
  if (metadata == 9){ // last line of response
    if (postedtext){
      // @todo split this line in param and text data, replace + char with a space char
      actiontext = requestline;
      postedtext = false;
    }
    Serial.print(F("posted colors: "));
    Serial.print(postedcolors);
    Serial.print(F(", 0x"));
    Serial.println(postedcolors, HEX);
    Serial.print(F("text input: |"));
    Serial.print(actiontext);
    Serial.println(F("|"));
    if (postedcolors > 0){
      Colorstatustext(postedcolors);
    }
  }
}
