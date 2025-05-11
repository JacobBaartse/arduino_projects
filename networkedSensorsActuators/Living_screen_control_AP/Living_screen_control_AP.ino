/*
 *
 */

#include "matrix.h"
#include "networking.h"
#include "screen.h"
#include "webinterface.h"

int status = WL_IDLE_STATUS;

IPAddress IPhere;

char ssid[] = "UNO_R4_AP_Living"; // your network SSID (name)
char pass[] = "TBD PW for UNO_R4_AP_RF"; // your network password

uint8_t WiFichannel = 13; // WiFi channel (1-13), 6 seems default


// void restart_arduino(){
//   Serial.println(F("Restart the arduino UNO board..."));
//   delay(2000);
//   NVIC_SystemReset();
// }

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  delay(1000);
  Serial.println(F("Starting UNO R4 WiFi"));
  Serial.println();
  Serial.print(__FILE__);
  Serial.print(F("\n, creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 
  
  //RTC.begin();
  matrix.begin();

  setupRFnetwork();

  setupScreenControl();

  // print the network name (SSID);
  Serial.print(F("Creating access point named: "));
  Serial.print(ssid);
  Serial.print(F(", password: '"));
  Serial.print(pass);
  Serial.println(F("'"));

  // by default the local IP address will be 192.168.4.1
  // you can override it with the following:
  WiFi.config(IPAddress(192,168,14,4));
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

  Serial.println(F("\n ***************\n"));  
  Serial.flush(); 
}
 
unsigned long currentMillis = 0; // stores the value of millis() in each iteration of loop()
unsigned int receiveaction = 0;
unsigned int transmitaction = 0;
bool screening = false;

void loop() {

  currentMillis = millis();

  network.update();
 
  //===== Receiving =====//
  receiveaction = receiveRFnetwork(currentMillis);
  bool receivedfresh = receiveaction > 0;

  //===== Sending =====//
  transmitaction = transmitRFnetwork(currentMillis, receivedfresh);

  webinterfacing();

  if (screening){
    screening = screenprocessing();
  }

  // client = server.available();   // listen for incoming clients

  // if (client) {                             // if you get a client,
  //   Serial.println(F("reading from client"));        // print a message out the serial port
  //   String currentLine = "";                       // make a String to hold incoming data from the client
  //   while (client.connected()) {            // loop while the client's connected
  //     if (client.available()) {             // if there's bytes to read from the client,
  //       c = client.read();                  // read a byte, then
  //       Serial.write(c);                    // print it out to the serial monitor
  //       if (c == '\n') {                    // if the byte is a newline character

  //         // if the current line is blank, you got two newline characters in a row.
  //         // that's the end of the client HTTP request, so send a response:
  //         if (currentLine.length() == 0) {
  //           // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  //           // and a content-type so the client knows what's coming, then a blank line:
  //           client.println(F("HTTP/1.1 200 OK"));
  //           client.println(F("Content-type:text/html"));
  //           client.println();

  //           client.print(F("<HTML><HEAD><TITLE>Arduino UNO R4 WiFi</TITLE><META content=\"text/html; charset=iso-8859-1\" http-equiv=Content-Type>"));
  //           client.print(F("<META HTTP-EQUIV=Expires CONTENT=\"Sun, 16-Apr-2028 01:00:00 GMT\"><link rel=\"icon\" href=\"data:,\">")); 
  //           client.print(F("<style> table {width:100%;} tr {height:200px; font-size:4em;} th, td {text-align:center;} </style>")); 
  //           client.print(F("</HEAD><BODY TEXT=\"#873e23\" LINK=\"#1f7a1f\" VLINK=\"#1f7a1f\" ALINK=\"#1f7a1f\" BGCOLOR=\"#bb99ff\">"));

  //           client.print(F("<TABLE><TR><TH colspan=3><a href=\"/T\">LEDs</a></TH></TR>"));
  //           client.print(F("<TR><TD>1</TD><TD><a href=\"/1H\">ON</a></TD><TD><a href=\"/1L\">off</a></TD></TR>"));
  //           client.print(F("<TR><TD>2</TD><TD><a href=\"/2H\">ON</a></TD><TD><a href=\"/2L\">off</a></TD></TR>"));
  //           client.print(F("<TR><TD>3</TD><TD><a href=\"/3H\">ON</a></TD><TD><a href=\"/3L\">off</a></TD></TR>"));

  //           // client.print(F("</TABLE><TABLE><TR><TH colspan=3><a href=\"/Z\">RELAYs</a></TH></TR>"));
  //           // client.print(F("<TR><TD>1</TD><TD><a href=\"/1R\">ON</a></TD><TD><a href=\"/1K\">off</a></TD></TR>"));
  //           // client.print(F("<TR><TD>2</TD><TD><a href=\"/2R\">ON</a></TD><TD><a href=\"/2K\">off</a></TD></TR>"));
  //           // client.print(F("</TABLE></BODY></HTML>"));

  //           // The HTTP response ends with another blank line:
  //           client.println();
  //           // break out of the while loop:
  //           break;
  //         }
  //         else { // if you got a newline, then clear currentLine:
  //           currentLine = "";
  //         }
  //       }
  //       else if (c != '\r') {    // if you got anything else but a carriage return character,
  //         currentLine += c;      // add it to the end of the currentLine
  //       }

  //       //remacounter = acounter;
  //       // Check to see if the client request was "GET /H" or "GET /L":
  //       if (currentLine.endsWith("GET /1H")) {
  //         //digitalWrite(LEDpin1, HIGH);               // GET /H turns the LED on
  //         //acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /1L")) {
  //         //digitalWrite(LEDpin1, LOW);                // GET /L turns the LED off
  //         //acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /2H")) {
  //         //digitalWrite(LEDpin2, HIGH);               // GET /H turns the LED on
  //         //acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /2L")) {
  //         //digitalWrite(LEDpin2, LOW);                // GET /L turns the LED off
  //         //acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /3H")) {
  //         //digitalWrite(LEDpin3, HIGH);               // GET /H turns the LED on
  //         //acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /3L")) {
  //         //igitalWrite(LEDpin3, LOW);                // GET /L turns the LED off
  //         //acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /T")) { // GET /T toggles the LEDs
  //         // digitalWrite(LEDpin1, !digitalRead(LEDpin1));  
  //         // digitalWrite(LEDpin2, !digitalRead(LEDpin2));  
  //         // digitalWrite(LEDpin3, !digitalRead(LEDpin3)); 
  //         // acounter += 1;
  //       }

  //       if (currentLine.endsWith("GET /1R")) {
  //         // relay1 = true;  
  //         // acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /1K")) {
  //         // relay1 = false;  
  //         // acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /2R")) {
  //         // relay2 = true;  
  //         // acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /2K")) {
  //         // relay2 = false;  
  //         // acounter += 1;
  //       }
  //       if (currentLine.endsWith("GET /Z")) { // GET /Z toggles the relays
  //         // relay1 = !relay1;  
  //         // relay2 = !relay2;
  //         // acounter += 1;
  //       }

  //       // if (currentLine.endsWith("GET /")) { // home page gets triggered as well
  //       //   acounter += 1;
  //       // }
  //       // if(!sendDirect){ // make sure it is not set to false during this loop
  //       //   sendDirect = (remacounter != acounter);
  //       //   if(sendDirect){
  //       //     Serial.print(F(" sendDirect var set "));
  //       //     Serial.println(millis());
  //       //   }
  //       // }
  //       //LEDstatustext(sendDirect, acounter);

  //       if (currentLine.endsWith("GET /favicon.ico")) {
  //         // sendFavicon();
  //         client.println(F("HTTP/1.1 404 Not Found\nConnection: close\n\n"));
  //       }      
  //     }
  //     else {
  //       //delay(1000); 
  //       Serial.println(F("breaking from loop"));
  //       break; // break from loop and disconnect client
  //     }
  //   }

  //   // close the connection:
  //   client.stop();
  //   Serial.println(F("client disconnected"));
  //   //delay(1000); // make sure the disconnection is detected
  
  // }

}

