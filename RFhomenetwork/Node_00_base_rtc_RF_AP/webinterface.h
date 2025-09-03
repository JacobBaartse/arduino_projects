/*
 * webpage for providing information and command input
 */

WiFiServer server(80);
WiFiClient client;

char c = '\n';
String currentLine = "";


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

  client.print(F("<TABLE><TR><TH colspan=3>Licht bediening</TH></TR>"));
  client.print(F("<TR><TD>Keuken</TD><TD><a href=\"/Kaan\">aan</a></TD><TD><a href=\"/Kuit\">uit</a></TD></TR>"));
  client.print(F("<TR><TD>Schuur</TD><TD><a href=\"/Saan\">aan</a></TD><TD><a href=\"/Suit\">uit</a></TD></TR>"));
  client.print(F("</TABLE><HR><BR>"));

  //client.print(F("</TABLE><HR>&nbsp;<a href=\"/SL\">smaller</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"/SU\">bigger</a>&nbsp;&nbsp;("));
  //client.print(text_size);
  // client.print(F(")&nbsp;&nbsp;Text in:<FORM action=\"t\" method=\"post\">"));
  // client.print(F("<input type=\"text\" id=\"t1\" name=\"t2\" required minlength=\"4\" maxlength=\"80\" size=\"30\"/>"));
  // client.print(F("&nbsp;&nbsp;&nbsp;<input name=\"send\" type=\"submit\"/>")); // <input type=\"submit\" hidden />
  // client.print(F("</FORM><HR>"));

  client.print(F("</BODY></HTML>"));

  // The HTTP response ends with another blank line:
  client.println();
  Serial.println(F(" done"));
}

int HTMLresponseline(String requestline, int linenumber){
  //static bool postedtext = false;
  //static int postedcolors = 0;
  int webcommand = 0;

  if (requestline.length() > 0) {
    Serial.print(linenumber);
    Serial.print(F(", HTMLresp |"));
    Serial.print(requestline);
    Serial.println(F("|"));

    // check the line for actions
    if (currentLine.startsWith("GET /")) {
      if (currentLine.startsWith("GET /favicon.ico")) {
        client.println(F("HTTP/1.1 404 Not Found\nConnection: close\n\n"));
      } 
      if (requestline.startsWith("GET /Kaan")){  
        webcommand += 1;
      } 
      if (requestline.startsWith("GET /Kuit")){  
        webcommand += 2;
      } 
      if (requestline.startsWith("GET /Saan")){  
        webcommand += 4;
      } 
      if (requestline.startsWith("GET /Suit")){  
        webcommand += 8;
      } 
    }
    // if (requestline.startsWith("POST /")) {  // text input follows
    //   postedtext = true;
    // } 
  }
  // if (linenumber > 1000){ // last line of response
  //   Serial.print(F("posted colors: "));
  //   Serial.print(postedcolors);
  //   Serial.print(F(", 0x"));
  //   Serial.println(postedcolors, HEX);
  //   Serial.print(F("text input: |"));
  //   //Serial.print(actiontext);
  //   Serial.println(F("|"));
  // }
  return webcommand;
}

int webinterfacing() {
  unsigned int linecount = 0;
  int commandfromweb = 0;
  int tempval = 0;

  client = server.available();              // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println(F("new client:"));       // print a message out the serial port
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
          else { // if there is a newline, then print, process and clear currentLine
            tempval = HTMLresponseline(currentLine, ++linecount);
            if (tempval > 0){
              commandfromweb += tempval;
            }
            // currentmeta = 1;
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
      else {
        Serial.println(F("\nBreaking from loop"));
        break; // break from loop and disconnect client
      }
    } // while client.connected

    tempval = HTMLresponseline(currentLine, 1001); // process last line
    if (tempval > 0){
      commandfromweb += tempval;
    }

    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
  }
  client.stop();

  return commandfromweb;
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
