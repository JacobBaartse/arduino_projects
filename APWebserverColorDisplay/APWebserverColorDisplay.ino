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
 * default ip address: 192.168.4.1
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

// int led =  LED_BUILTIN;
int status = WL_IDLE_STATUS;
int newstatus = WL_IDLE_STATUS;

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

void LEDstatustext(bool LEDon, unsigned long count){
  static unsigned long bcount = 0;
  if (count != bcount){ // update display only once
    String TextHere = "_"; // "_--  ";
    if (LEDon) TextHere = "^"; // "oO0  ";
    TextHere = TextHere + (count % 10);
    Serial.println(F(""));
    Serial.print(TextHere);

    matrix.beginText(0, 1, 0xFFFFFF);
    matrix.println(TextHere);
    matrix.endText();
    bcount = count;
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
  // wait 10 seconds for connection:
  delay(10000);
  digitalWrite(7, HIGH);

  // start the web server on port 80
  server.begin();
  // you're connected now, so print out the status
  IPhere = printWiFiStatus();

  Serial.println(F("Created access point available"));

  startupscrollingtext(String("-->: ") + IPhere.toString());

  Serial.println(F(" "));  
  Serial.println(F(" *************** "));  
  Serial.println(F(" "));  
  Serial.flush(); 
}

WiFiClient client;

// void sendFavicon()
// {
//   // create a favion: https://www.favicon.cc/
//   // convert to hex: http://tomeko.net/online_tools/file_to_hex.php?lang=en or https://www.onlinehexeditor.com/
//   // Please note that if PROGMEM variables are not globally defined, 
//   // you have to define them locally with static keyword, in order to work with PROGMEM.
//   const static byte tblFavicon[] PROGMEM = {0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x10, 0x00, 0x01, 0x00, 0x04, 0x00, 0x28, 0x01, 
//                                             0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 
//                                             0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x00, 0x82, 0x7E, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
//                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x01, 0x00, 0x01, 0x00, 0x10, 0x00, 0x10, 0x10, 
//                                             0x01, 0x00, 0x01, 0x00, 0x10, 0x00, 0x11, 0x10, 0x01, 0x00, 0x01, 0x00, 0x11, 0x10, 0x10, 0x10, 
//                                             0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x10, 0x10, 0x11, 0x10, 0x11, 0x10, 0x11, 0x10, 0xFF, 0xFF, 
//                                             0x00, 0x00, 0xF0, 0x1F, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 
//                                             0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFA, 0xBF, 0x00, 0x00, 0xFC, 0x7F, 
//                                             0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x5B, 0xB7, 0x00, 0x00, 0x5B, 0xB7, 
//                                             0x00, 0x00, 0x1B, 0xB1, 0x00, 0x00, 0x5B, 0xB5, 0x00, 0x00, 0x51, 0x11, 0x00, 0x00 
//                                            };

//   const size_t MESSAGE_BUFFER_SIZE = 64;
//   char buffer[MESSAGE_BUFFER_SIZE];  // a buffer needed for the StreamLib
//   BufferedPrint message(client, buffer, sizeof(buffer));
//   message.print(F("HTTP/1.0 200 OK\r\n"
//                  "Content-Type: image/x-icon\r\n"
//                  "\r\n"));

//   for (uint16_t i = 0; i < sizeof(tblFavicon); i++)
//   {
//     byte p = pgm_read_byte_near(tblFavicon + i);
//     message.write(p);
//   }
//   message.flush();
//   client.stop();
// }

char c = '\n';
String currentLine = "";
unsigned long acounter = 0;

void loop() {
  
  // compare the previous status to the current status
  newstatus = WiFi.status();
  if (status != newstatus) {
    // it has changed update the variable
    status = newstatus;
  
    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println(F("Device connected to AP"));
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println(F("Device disconnected from AP"));
    }
  }
  
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
            client.print(F("<BODY TEXT=\"#33cc33\" LINK=\"#1f7a1f\" VLINK=\"#1f7a1f\" ALINK=\"#1f7a1f\" BGCOLOR=\"#bb99ff\">"));
            client.print(F("<TABLE style=\"width:100%\"><TR style=\"height:200px; font-size:4em;\"><TH colspan=2 style=\"text-align: center\"><a href=\"/T\">LED</a></TH></TR>"));
            client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: center\"><a href=\"/H\">ON</a></TD><TD style=\"text-align: center\"><a href=\"/L\">off</a></TD></TR>"));
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
        if (currentLine.endsWith("GET /H")) {
          //digitalWrite(led, HIGH);               // GET /H turns the LED on
          acounter += 1;
        }
        if (currentLine.endsWith("GET /L")) {
          //digitalWrite(led, LOW);                // GET /L turns the LED off
          acounter += 1;
        }
        if (currentLine.endsWith("GET /T")) {
          //digitalWrite(led, !digitalRead(led));  // GET /T toggles the LED
          acounter += 1;
        }
        // if (currentLine.endsWith("GET /")) {  // home page gets triggered as well
        //   acounter += 1;
        // }
        //LEDstatustext(digitalRead(led), acounter);

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
