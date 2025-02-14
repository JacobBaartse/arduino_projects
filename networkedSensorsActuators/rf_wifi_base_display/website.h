
#include "WiFiS3.h"

WiFiServer server(80);

char c = '\n';
String currentLine = "";
unsigned long acounter = 0;
WiFiClient client;

void websitehandling() {
  
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
