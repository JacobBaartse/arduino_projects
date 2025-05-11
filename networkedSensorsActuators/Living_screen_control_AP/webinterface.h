/*
 *
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

  client.print(F("<TABLE><TR><TH colspan=3>Coloring</TH></TR>"));
  // client.print(F("<TR><TD><a href=\"/C1\">Red</a></TD><TD><a href=\"/C2\">Green</a></TD><TD><a href=\"/C3\">Blue</a></TD></TR>"));
  // client.print(F("<TR><TD><a href=\"/C4\">Cyan</a></TD><TD><a href=\"/C5\">Magenta</a></TD><TD><a href=\"/C6\">Yellow</a></TD></TR>"));
  // client.print(F("<TR><TD><a href=\"/C7\">Orange</a></TD><TD><a href=\"/C8\">White</a></TD><TD><a href=\"/C9\">Black</a></TD></TR>"));
  client.print(F("<TR><TD><a href=\"/C1\"><span style=\"color: #ff0000\">Red</span></a></TD><TD><a href=\"/C2\"><span style=\"color: #00ff00\">Green</span></a></TD><TD><a href=\"/C3\"><span style=\"color: #0000ff\">Blue</span></a></TD></TR>"));
  client.print(F("<TR><TD><a href=\"/C4\"><span style=\"color: #33ccff\">Cyan</span></a></TD><TD><a href=\"/C5\"><span style=\"color: #9900ff\">Magenta</span></a></TD><TD><a href=\"/C6\"><span style=\"color: #ffff99\">Yellow</span></a></TD></TR>"));
  client.print(F("<TR><TD><a href=\"/C7\"><span style=\"color: #ffgg33\">Orange</span></a></TD><TD><a href=\"/C8\"><span style=\"color: #ffffff\">White</span></a></TD><TD><a href=\"/C9\"><span style=\"color: #000000\">Black</span></a></TD></TR>"));

  client.print(F("</TABLE><HR>&nbsp;<a href=\"/SL\">smaller</a>&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"/SU\">bigger</a>&nbsp;&nbsp;("));
  //client.print(text_size);
  client.print(F(")&nbsp;&nbsp;Text in:<FORM action=\"t\" method=\"post\">"));
  client.print(F("<input type=\"text\" id=\"t1\" name=\"t2\" required minlength=\"4\" maxlength=\"80\" size=\"30\"/>"));
  client.print(F("&nbsp;&nbsp;&nbsp;<input name=\"send\" type=\"submit\"/>")); // <input type=\"submit\" hidden />
  client.print(F("</FORM><HR></BODY></HTML>"));

  // The HTTP response ends with another blank line:
  client.println();
  Serial.println(F(" done"));
}

void HTMLresponseline(String requestline, int metadata){
  static bool postedtext = false;
  static int postedcolors = 0;

  if (requestline.length() > 0) {
    Serial.print(F("HTMLresp |"));
    Serial.print(requestline);
    Serial.println(F("|"));

    // check the line for actions
    if (currentLine.startsWith("GET /")) {

      if (currentLine.startsWith("GET /favicon.ico")) {
        client.println(F("HTTP/1.1 404 Not Found\nConnection: close\n\n"));
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
        //actiontext = ""; // clear text
      }  
      if (requestline.startsWith("GET /SL")) {  // size item
        //if (text_size > 1) text_size--;
      }
      if (requestline.startsWith("GET /SU")) {  // size item
        //if (text_size < 6) text_size++;
      }
    }
    if (requestline.startsWith("POST /")) {  // text input follows
      postedtext = true;
    } 
  }
  if (metadata == 9){ // last line of response
    Serial.print(F("posted colors: "));
    Serial.print(postedcolors);
    Serial.print(F(", 0x"));
    Serial.println(postedcolors, HEX);
    Serial.print(F("text input: |"));
    //Serial.print(actiontext);
    Serial.println(F("|"));
  }
}

void webinterfacing() {
  
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
            HTMLresponseline(currentLine, 10);
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

    HTMLresponseline(currentLine, 9);

    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
  }
  client.stop();

}
