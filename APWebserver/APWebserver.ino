/*
  WiFi Web Server LED Control

  A simple web server that lets you control a LED via a local network.
  This sketch will create a new access point (with no password).
  It will then launch a new server and print out the IP address
  to the Serial Monitor. From there, you can open that address in a web browser
  to turn on and off the LED on pin 13.

  If the IP address of your board is yourAddress:
    http://yourAddress/H turns the LED on
    http://yourAddress/L turns the LED off

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
#include <Ethernet.h>
#include <StreamLib.h>  

ArduinoLEDMatrix matrix;

char ssid[] = "UNO_R4_demo"; // your network SSID (name)

char c;
int led =  LED_BUILTIN;
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

void LEDstatustext(bool LEDon){
  String TextHere = "F"; // "_--  ";
  if (LEDon) TextHere = "N"; // "oO0  ";
  Serial.println("");
  Serial.println(TextHere);

  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(TextHere);
  matrix.endText();
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
  Serial.println(F("Access Point Web Server"));
  matrix.begin();

  pinMode(led, OUTPUT); // set the LED pin mode

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  String timestamp = __TIMESTAMP__;
  Serial.print(F("Creation/build time: "));
  Serial.println(timestamp);
  Serial.flush(); 

  // by default the local IP address will be 192.168.4.1
  // you can override it with the following:
  //WiFi.config(IPAddress(192,48,56,2));

  // print the network name (SSID);
  Serial.print(F("Creating access point named: "));
  Serial.println(ssid);

  WiFi.config(IPAddress(192,168,12,2));

  // Create open network. Change this line if you want to create an WEP network:
  //status = WiFi.beginAP(ssid, pass);
  status = WiFi.beginAP(ssid); // no password needed
  if (status != WL_AP_LISTENING) {
    Serial.println(F("Creating access point failed"));
    // don't continue
    while (true);
  }

  // wait 10 seconds for connection:
  delay(10000);
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

//String ICOimageString = "data:image/jpeg;base64,/....";
WiFiClient client;

//void sendFavicon(EthernetClient &client)
void sendFavicon()
{
  // create a favion: https://www.favicon.cc/
  // convert to hex: http://tomeko.net/online_tools/file_to_hex.php?lang=en or https://www.onlinehexeditor.com/
  // Please note that if PROGMEM variables are not globally defined, 
  // you have to define them locally with static keyword, in order to work with PROGMEM.
  const static byte tblFavicon[] PROGMEM = {0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x10, 0x00, 0x01, 0x00, 0x04, 0x00, 0x28, 0x01, 
                                            0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 
                                            0x00, 0x00, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x00, 0x82, 0x7E, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x01, 0x00, 0x01, 0x00, 0x10, 0x00, 0x10, 0x10, 
                                            0x01, 0x00, 0x01, 0x00, 0x10, 0x00, 0x11, 0x10, 0x01, 0x00, 0x01, 0x00, 0x11, 0x10, 0x10, 0x10, 
                                            0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x10, 0x10, 0x11, 0x10, 0x11, 0x10, 0x11, 0x10, 0xFF, 0xFF, 
                                            0x00, 0x00, 0xF0, 0x1F, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 
                                            0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFA, 0xBF, 0x00, 0x00, 0xFC, 0x7F, 
                                            0x00, 0x00, 0xFE, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x5B, 0xB7, 0x00, 0x00, 0x5B, 0xB7, 
                                            0x00, 0x00, 0x1B, 0xB1, 0x00, 0x00, 0x5B, 0xB5, 0x00, 0x00, 0x51, 0x11, 0x00, 0x00 
                                           };

  const size_t MESSAGE_BUFFER_SIZE = 64;
  char buffer[MESSAGE_BUFFER_SIZE];  // a buffer needed for the StreamLib
  BufferedPrint message(client, buffer, sizeof(buffer));
  message.print(F("HTTP/1.0 200 OK\r\n"
                 "Content-Type: image/x-icon\r\n"
                 "\r\n"));

  for (uint16_t i = 0; i < sizeof(tblFavicon); i++)
  {
    byte p = pgm_read_byte_near(tblFavicon + i);
    message.write(p);
  }
  message.flush();
  client.stop();
}

void loop() {
  
  // compare the previous status to the current status
  newstatus = WiFi.status();
  if (status != newstatus) {
    // it has changed update the variable
    status = newstatus;
  
    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }
  
  client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
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
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("<p style=\"font-size:7vw;\">LED<br></p>");
            client.print("<p style=\"font-size:7vw;\"><a href=\"/H\">ON</a><br></p>");
            client.print("<p style=\"font-size:7vw;\"><a href=\"/L\">off</a><br></p>");

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
          digitalWrite(led, HIGH);               // GET /H turns the LED on
          LEDstatustext(true);
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(led, LOW);                // GET /L turns the LED off
          LEDstatustext(false);
        }
        if (currentLine.endsWith("GET /favicon.ico")) {
          sendFavicon();
          //client.println(F("HTTP/1.1 404 Not Found\nConnection: close\n\n"));

          // client.println("HTTP/1.1 200 OK");
          // client.println("Content-type:text/html");
          // client.println("Connection: close");
          // client.println();
          // client.println(ICOimageString);
          //"data:image/x-icon;");
          //data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGQAAAB0CAYAAABzNJfPAAAABGdBTUEAALGP.
          //client.print("<p style=\"font-size:7vw;\">LED<br></p>");
        }      
      }
    }

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
