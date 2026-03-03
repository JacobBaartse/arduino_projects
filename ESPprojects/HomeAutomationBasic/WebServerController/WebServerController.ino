/*

 Dualmode wifi, webserver using links

 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// Station Mode Credentials
const char* sta_ssid = "T24_optout";
const char* sta_password = "T24T24T24";

// AP Mode Credentials
const char* ap_ssid = "Local_AP";
const char* ap_password = "local_148_AP";

// login credentials
const char* http_username = "so148";
const char* http_password = "pietcarla";

ESP8266WebServer server(80);

const int led = LED_BUILTIN;
int rootresponse = 0;
bool ledflash = false;

unsigned long runningtime = 0;
unsigned long requesttime = 0;
unsigned long pollingtime = 5000;
unsigned long norequesttime = pollingtime * 4.5;

// array of server and clients (server has id 0, clients starting from 1 and up)
int devices[] = {9, 9, 9, 9, 999}; // last one is 999, referring to 'all' in webpage
int pollsamples[] = {2, 5, 10, 30, 60, 999};

const String startsection = "<!DOCTYPE HTML><html><head><title>Home controller for Sickengaoord 148</title> \
      <style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }</style> \
      </head><h1>Home network SO 148</h1>LED control (onboard LEDs)<br><br>";
const String middlesection1 = "<br><br><table><tr><th>on</th><th>off</th><th>flash</th></tr>";
const String middlesection2 = "</table><br><table><tr><th>polling</th></tr>";
const String endsection = "</table></body></html>";

String clientline(int clientid){
  String cidstring = String(clientid);
  String cidref = cidstring;
  if (clientid == 0){
    cidref = "ctrl"; // "controller"; // "server/controller";
  }
  if (devices[clientid] > 900){
    cidref = "all"; // "all devices";
    cidstring = String(devices[clientid]);
  }
  String linehere = "<tr><td><a href=\"/?id="+cidstring+"&stat=0\">"+cidref+"</a></td><td><a href=\"/?id="+cidstring+"&stat=1\">"+cidref+"</a></td><td><a href=\"/?id="+cidstring+"&stat=2\">"+cidref+"</a></td></tr>";
  return linehere;
}

String pollline(int polinterval){
  String linehere = "<tr><td><a href=\"/?poll="+String(polinterval)+"\">"+String(polinterval)+" sec</a></td></tr>";
  return linehere;
}

String makewebpagehtml(String statusString){ // to be enhanced, array processing
  String htmlpage = startsection;
  String specificline = "";

  htmlpage += statusString;
  htmlpage += middlesection1;
  int i = 0;
  while (true){
    specificline = clientline(i);
    htmlpage += specificline;
    if (devices[i] > 900) break; // last line indicates 'all'
    i += 1;
  }
  htmlpage += middlesection2;
  i = 0;
  while (true){
    specificline = pollline(pollsamples[i]);
    htmlpage += specificline;
    i += 1;
    if (pollsamples[i] > 900) break; // nextline indicates end of array
  }
  htmlpage += endsection;
  
  return htmlpage;
}

void handleRoot() {
  int valnow = 9;
  int idval = 9;

  // Check if client credentials match
  if (!server.authenticate(http_username, http_password)) {
    // If not authenticated, request authentication
    Serial.println(F("Requesting credentials."));
    // Send 401 Unauthorized response with WWW-Authenticate header
    return server.requestAuthentication(BASIC_AUTH, "Sickengaoord 148 thuis netwerk");
  }

  Serial.print(millis());
  Serial.print(F(", handleRoot: "));
  // Serial.print("URI: "); 
  // Serial.print(server.uri()); 
  // Serial.print(", method: "); 
  // Serial.print(server.method() == HTTP_GET ? "GET" : "POST"); 
  // Serial.print(", arguments:"); 
  // for (uint8_t i = 0; i < server.args(); i++){ 
  //   Serial.printf(" %s = %s", server.argName(i).c_str(), server.arg(i).c_str()); 
  // }
  // Serial.println(F(" "));

  if (server.hasArg("poll")) {
    rootresponse = server.arg("poll").toInt();
  }
  if (server.hasArg("stat")) {
    valnow = server.arg("stat").toInt();
  }
  if (server.hasArg("id")) {
    idval = server.arg("id").toInt();
    if (idval > 10){ // all (clients and the server)
      int i = 0;
      while (true){
        devices[i] = valnow;
        i += 1;
        if (devices[i] > 900) break;
      }
    }
    else {
      devices[idval] = valnow;
    }
  }

  String ledString = "leds: ";
  int i = 0;
  String Split = "";
  while (devices[i] < 900){
    ledString += Split;
    ledString += String(devices[i]);
    i += 1;
    Split = ", ";
  }
  Serial.print(ledString);
  Serial.println(F(" !"));

  if (devices[0] < 3){ // local LED on this server board
    if (devices[0] < 2){ // local LED on this server board
      ledflash = false;
      digitalWrite(led, devices[0]);
    }
    else{ // value 2 means flashing
      ledflash = true;
    }
  }
  Serial.println(F("Server html page"));
  String webpage = makewebpagehtml(ledString); // include the current status information
  server.send(200, "text/html", webpage);
  Serial.println(F(" "));
}

void handleLEDjson() {
  static String remresponse = "";
  int clientval = 9;
  int clientstat = 9;
  int clientresp = 9;
  
  // Serial.print("URI: "); 
  // Serial.print(server.uri()); 
  // Serial.print(", method: "); 
  // Serial.print(server.method() == HTTP_GET ? "GET" : "POST"); 
  // Serial.print(", arguments:"); 
  // for (uint8_t i = 0; i < server.args(); i++){ 
  //   Serial.printf(" %s = %s", server.argName(i).c_str(), server.arg(i).c_str()); 
  // }
  // Serial.println(F(" "));

  String response = ""; // "{\"led\" : 9, \"value\" : 9"; // }"; // unknown client id

  if (server.hasArg("cid")) {
    clientval = server.arg("cid").toInt();
  } 
  if (server.hasArg("cstat")) {
    clientstat = server.arg("cstat").toInt();
  } 
  // Serial.print("clientstat: "); 
  // Serial.println(clientstat); 

  if (devices[clientval] > 2){
    devices[clientval] = clientstat;
  }
  clientresp = devices[clientval]; 
  
  response = "{\"led\" : " + String(clientval) + ", \"value\" : " + String(clientresp); // + "}";
  // response += ", \"servertime\" : " + String(runningtime) + ", \"pollingtime\" : " + String(pollingtime) + "}";
  response += ", \"pollingtime\" : " + String(pollingtime);
  bool printhere = (remresponse != response); // keep server time out of the printing
  remresponse = response;
  runningtime = millis();
  response += ", \"servertime\" : " + String(runningtime)  + "}"; // keep server time out of the printing

  if (printhere){
    Serial.print(runningtime);
    Serial.print(F(", handleLED, "));
    Serial.print(F("response: '"));
    Serial.print(response);
    Serial.println(F("'"));
  }
  server.send(200, "application/json", response);
  requesttime = runningtime;
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) { 
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; 
  }
  Serial.print(F("Response from server (unexpected request): '*'"));
  Serial.print(message);
  Serial.println(F("'*'"));
  server.send(404, "text/plain", message);
}

bool requesttimelapsed(unsigned long duration){
  static int lapsed = 0;
  if(runningtime < requesttime + duration){ 
    lapsed = 0;
    return lapsed;
  }
  if (lapsed > 0){
    Serial.print(F("No timely request received: "));
    Serial.print(lapsed);
    Serial.print(F(", "));
    Serial.println(runningtime);
  }
  lapsed += 1;
  requesttime = runningtime; // set the printing as request time, to get 1 message per duration
  return lapsed; 
}

void ledflashing(unsigned long timestamp, unsigned long duration){
  static unsigned long ledtime = 0;
  if(timestamp < ledtime) return;
  ledtime = millis() + duration; // make sure to get 'fresh' timestamp to avoid processing time influences
  digitalWrite(led, !digitalRead(led)); // toggle onboard LED
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0); // turn onboard LED on
  Serial.begin(115200);

  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.println(F("Starting ESP device"));
  Serial.flush();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  WiFi.mode(WIFI_AP_STA);  // Enable both AP and Station modes
  WiFi.softAP(ap_ssid, ap_password);  // Start the local access point
  IPAddress local_ip(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.begin(sta_ssid, sta_password); // Connect to existing network (router, internet)
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("");
  Serial.print("Connected to: ");
  Serial.print(sta_ssid);
  Serial.print(", IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(" ");

  Serial.print("Local Access Point: ");
  Serial.print(ap_ssid);
  Serial.print(", IP address: ");
  Serial.println(local_ip);
  Serial.println(" ");

  if (MDNS.begin("esp8266")) { 
    Serial.println("MDNS responder started"); 
  }

  server.on("/", handleRoot);
  server.on("/led", handleLEDjson);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(led, 1); // turn onboard LED off
}

int norequest = 0;

void loop(void) {

  runningtime = millis();

  if (ledflash){
    ledflashing(runningtime, 1000);
  }

  server.handleClient();
  if(rootresponse > 0){
    Serial.print("Root responsing: ");
    Serial.println(rootresponse);
    pollingtime = rootresponse * 1000;
    norequesttime = pollingtime * 4.5;
    rootresponse = 0; // handle once
  }

  norequest = requesttimelapsed(norequesttime);
  // if (norequest > 0){ // no request received within time period
  //   pollingtime = 2000;
  // }

}
