
#include "WiFiS3.h"

WiFiServer server(80);
WiFiClient client;

char c = '\n';
String currentLine = "";
String timval = "-";
String actionval = "-";
unsigned long actiontiming = 0;

void HTMLreply() {
  Serial.print(F("HTMLreply"));

  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-type:text/html"));
  client.println();
  // the content of the HTTP response follows the header:
  
  client.print(F("<HTML><HEAD><TITLE>Arduino UNO R4 WiFi with RTC module</TITLE><META content=\"text/html; charset=iso-8859-1\" http-equiv=Content-Type>"));
  client.print(F("<META HTTP-EQUIV=Expires CONTENT=\"Sun, 16-Apr-2028 01:00:00 GMT\"><link rel=\"icon\" href=\"data:,\"></HEAD>")); 
  client.print(F("<BODY TEXT=\"#006600\" LINK=\"#1f7a1f\" VLINK=\"#1f7a1f\" ALINK=\"#1f7a1f\" BGCOLOR=\"#bb99ff\">"));

  client.print(F("<TABLE style=\"width:100%\"><TR style=\"height:150px; font-size:4em;\"><TH colspan=2 style=\"text-align: center\">Measured values</TH><TH>&nbsp;&nbsp;</TH></TR>"));
  client.print(F("<TR style=\"height:100px; font-size:4em;\"><TD style=\"text-align: right\">Temp 1</TD><TD style=\"text-align: right\">"));
  //client.print(tempval1);
  client.print(F(" &deg;C</TD><TD>&nbsp;&nbsp;</TD></TR>"));
  client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: right\">Temp 2</TD><TD style=\"text-align: right\">"));
  //client.print(tempval2);
  client.print(F(" &deg;C</TD><TD>&nbsp;&nbsp;</TD></TR>"));
  client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: right\">rel. Humidity</TD><TD style=\"text-align: right\">"));
  //client.print(humidval);
  client.print(F("%</TD><TD>&nbsp;&nbsp;</TD></TR>"));
  client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD style=\"text-align: right\">Pressure</TD><TD style=\"text-align: right\">"));
  //client.print(presval);
  client.print(F(" hPa</TD><TD>&nbsp;&nbsp;</TD></TR>"));
  client.print(F("<TR style=\"height:200px; font-size:4em;\"><TD colspan=2 style=\"text-align: center\">"));
  client.print(timval);
  client.print(F("</TD><TD>"));
  client.print(actionval);
  client.print(F("</TD></TR>"));
  client.print(F("</TABLE>"));

  client.print(F("<TABLE style=\"width:100%\"><TR style=\"height:150px; font-size:4em;\"><TH colspan=2 style=\"text-align: center\"><a href=\"/action\">Action</a></TH></TR>"));
  client.print(F("<TR style=\"height:100px; font-size:4em;\"><TD style=\"text-align: center\"><a href=\"/on\">On</a></TD><TD style=\"text-align: center\"><a href=\"/off\">off</a></TD></TR>"));
  client.print(F("</TABLE>"));

  client.print(F("</BODY></HTML>"));

  // The HTTP response ends with another blank line:
  client.println();
  client.flush();
  Serial.println(F(" done"));  
}

String PROGMEM getRequests[][3] = {
  { "/favicon.ico", "-", "HTTP/1.1 404 Not Found\nConnection: close\n\n"},
  { "/action", "Action", "" }, 
  { "/on", "On", "" }, 
  { "/off", "Off", "" }, 
  
  { "EOR", "", ""} // End Of Array indication
}; 

void HTMLresponseline(String requestline, int metadata){

  if (requestline.length() > 0) {
    
    // check the line for actions
    if (requestline.startsWith(F("GET /"))){ 

      Serial.print(F("HTMLresp |"));
      Serial.print(requestline);
      Serial.print(F("| action: "));

      for (int actionid = 0; getRequests[actionid][0] != F("EOR"); actionid++){
        if (requestline.indexOf(getRequests[actionid][0]) > 0){
          actionval = getRequests[actionid][1];
          String followup = getRequests[actionid][2];
          if (followup == ""){
            actiontiming = millis();
          }
          else {
            client.println(followup);
          }
        }
      }
      Serial.println(actionval);

    //   //if (requestline.startsWith(F("GET /favicon.ico"))) {
    //   if (requestline.indexOf(F("/favicon.ico")) > 0) {
    //     client.println(F("HTTP/1.1 404 Not Found\nConnection: close\n\n"));
    //   } 
    //   // if (requestline.startsWith("POST /")) {  // text input follows
    //   // }    
    //   String remactionval = actionval;
    //   //if (requestline.startsWith(F("GET /action"))) {  // action
    //   if (requestline.indexOf(F("/action")) > 0) {
    //     actionval = F("Action");
    //     actiontiming = millis();
    //   } 
    //   //if (requestline.startsWith(F("GET /on"))) {  // on
    //   if (requestline.indexOf(F("/on")) > 0) {  // on
    //     actionval = F("On");
    //     actiontiming = millis();
    //   }     
    //   //if (requestline.startsWith(F("GET /off"))) {  // off
    //   if (requestline.indexOf(F("/off")) > 0) {  // off
    //     actionval = F("Off");
    //     actiontiming = millis();
    //   } 

    //   if (remactionval != actionval){
    //     actiontiming = millis();
    //   }
    //   Serial.println(actionval);
    }
  }

  // if (metadata == 9){ // last line of response
  //   Serial.print(F("posted info: "));
  //   // Serial.print(postedcolors);
  //   // Serial.print(F(", 0x"));
  //   // Serial.println(postedcolors, HEX);
  //   // Serial.print(F("text input: |"));
  //   // //Serial.print(actiontext);
  //   Serial.println(F("|"));
  // }
}

void webpagehandling(String timinfo) {
  unsigned long prev_wifi_time = 0;

  client = server.available();              // listen for incoming clients

  if (client) {                             // if you get a client,
    prev_wifi_time = millis();
    // Serial.print(prev_wifi_time);        // print a message out the serial port
    Serial.println(F("client"));            // print a message out the serial port
    currentLine = "";                       // make a String to hold incoming data from the client                
    //timval = timinfo;                      
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        c = client.read();                  // read a byte, then
        //Serial.write(c);                    // print it out to the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() < 1) {
            HTMLreply();
            Serial.println(F("Completed: HTMLreply()"));
          }
          else{
            HTMLresponseline(currentLine, 10);
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
      else {
        Serial.println(F("breaking from loop"));
        break; // break from loop and disconnect client
      }
      if ((millis() - prev_wifi_time) > 2000){
        Serial.println(F("timeout from client loop"));
        break;
      }
    } // while client.connected

    HTMLresponseline(currentLine, 9);

    // close the connection:
    client.stop();
    Serial.println(F("client disconnected"));
  }

  if ((millis() - actiontiming) > 5000) // remove actionval, any action should have been addressed by this time
  {
    if (actionval != "-"){
      Serial.println(F("clearing actionval"));
    }
    actionval = "-";
  }

}
