
#include "WiFiS3.h"

WiFiServer server(80);

char c = '\n';
String currentLine = "";
unsigned long acounter = 0;
WiFiClient client;

float tempval1 = 0;
float tempval2 = 0;
int humidval = 0;
int presval = 0;

void HTMLreply() {
  Serial.print(F("HTMLreply"));

  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-type:text/html"));
  client.println();
  // the content of the HTTP response follows the header:
  
  client.print(F("<HTML><HEAD><TITLE>Arduino UNO R4 WiFi with Display</TITLE><META content=\"text/html; charset=iso-8859-1\" http-equiv=Content-Type>"));
  client.print(F("<META HTTP-EQUIV=Expires CONTENT=\"Sun, 16-Apr-2028 01:00:00 GMT\"><link rel=\"icon\" href=\"data:,\"></HEAD>")); 
  client.print(F("<BODY TEXT=\"#33cc33\" LINK=\"#1f7a1f\" VLINK=\"#1f7a1f\" ALINK=\"#1f7a1f\" BGCOLOR=\"#bb99ff\">"));

  client.print(F("<TABLE style=\"width:100%\"><TR style=\"height:200px; font-size:4em;\"><TH colspan=2 style=\"text-align: center\">Measured values</TH></TR>"));
  client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: right\">Temp 1</TD><TD style=\"text-align: right\">"));
  client.print(tempval1);
  client.print(F(" &deg;C</TD></TR>"));
  client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: right\">Temp 2</TD><TD style=\"text-align: right\">"));
  client.print(tempval2);
  client.print(F(" &deg;C</TD></TR>"));
  client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: right\">rel. Humidity</TD><TD style=\"text-align: right\">"));
  client.print(humidval);
  client.print(F("%</TD></TR>"));
  client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: right\">Pressure</TD><TD style=\"text-align: right\">"));
  client.print(presval);
  client.print(F(" hPa</TD></TR>"));
  client.print(F("</TABLE>"));

  client.print(F("<TABLE style=\"width:100%\"><TR style=\"height:200px; font-size:4em;\"><TH colspan=2 style=\"text-align: center\"><a href=\"/T\">LED</a></TH></TR>"));
  client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: center\"><a href=\"/H\">ON</a></TD><TD style=\"text-align: center\"><a href=\"/L\">off</a></TD></TR>"));
  client.print(F("</TABLE>"));

  client.print(F("</BODY></HTML>"));

  // The HTTP response ends with another blank line:
  client.println();
  Serial.println(F(" done"));  
}

int HTMLresponseline(String requestline, int metadata) {
  // static bool postedtext = false;
  // static int postedcolors = 0;

  if (requestline.length() > 0) {
    Serial.print(F("HTMLresp |"));
    Serial.print(requestline);
    Serial.println(F("|"));

    // check the line for actions

    if (currentLine.startsWith("GET /favicon.ico")) {
      client.println(F("HTTP/1.1 404 Not Found\nConnection: close\n\n"));
    } 
    if (requestline.startsWith("POST /")) {  // text input follows
      //postedtext = true;
    }    
    if (requestline.startsWith("GET /C0")) {  // color item
      //postedcolors = postedcolors | 0x0200;
    } 
    // if (requestline.startsWith("GET /C1")) {  // color item
    //   postedcolors = postedcolors | 0b00000001;
    // }     
    // if (requestline.startsWith("GET /C2")) {  // color item
    //   postedcolors = postedcolors | 0b00000010;
    // } 
    // if (requestline.startsWith("GET /C3")) {  // color item
    //   postedcolors = postedcolors | 0b00000100;
    // }     
    // if (requestline.startsWith("GET /C4")) {  // color item
    //   postedcolors = postedcolors | 0b00001000;
    // }     
    // if (requestline.startsWith("GET /C5")) {  // color item
    //   postedcolors = postedcolors | 0b00010000;
    // }     
    // if (requestline.startsWith("GET /C6")) {  // color item
    //   postedcolors = postedcolors | 0b00100000;
    // } 
    // if (requestline.startsWith("GET /C7")) {  // color item
    //   postedcolors = postedcolors | 0b01000000;
    // } 
    // if (requestline.startsWith("GET /C8")) {  // color item
    //   postedcolors = postedcolors | 0b10000000;
    // } 
    // if (requestline.startsWith("GET /C9")) {  // color item
    //   postedcolors = postedcolors | 0x0100;
    //   //actiontext = ""; // clear text
    // }  
    // if (requestline.startsWith("GET /SL")) {  // size item
    //   //if (text_size > 1) text_size--;
    // }
    // if (requestline.startsWith("GET /SU")) {  // size item
    //   //if (text_size < 6) text_size++;
    // }

  }
  if (metadata == 9){ // last line of response
    Serial.print(F("posted info: "));
    // Serial.print(postedcolors);
    // Serial.print(F(", 0x"));
    // Serial.println(postedcolors, HEX);
    // Serial.print(F("text input: |"));
    // //Serial.print(actiontext);
    Serial.println(F("|"));
  }
}

void websitehandling(float temp1, float temp2, int humid, int press) {
  
  client = server.available();              // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println(F("new client"));        // print a message out the serial port
    currentLine = "";                       // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        c = client.read();                  // read a byte, then
        //Serial.write(c);                    // print it out to the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() < 1) {
            tempval1 = temp1;
            tempval2 = temp2;
            humidval = humid;
            presval = press;
            HTMLreply();
          }
          HTMLresponseline(currentLine, 10);
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
      else {
        Serial.println(F("breaking from loop"));
        break; // break from loop and disconnect client
      }
    } // while client.connected

    HTMLresponseline(currentLine, 9);

    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
  }
  client.stop();

}
