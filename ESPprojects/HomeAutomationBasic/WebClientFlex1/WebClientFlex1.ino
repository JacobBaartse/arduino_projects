/*
  
  WebClient receiving/fetching data from WebServer

*/

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <Arduino_JSON.h>


ESP8266WiFiMulti WiFiMulti;

const int led = LED_BUILTIN;
int led_val = 2;
int clientid = 1;

unsigned long runningtime = 0;
unsigned long pollinginterval = 5000;
unsigned long pollingfromserver = 5000;
unsigned long servertime = 0;

int parseresult(int clientnumber, String payloadstring){
  static unsigned long prevservertime = 0;
  int rvalue = 2;
  JSONVar myObject = JSON.parse(payloadstring);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.print(F("Received payload: '"));
    Serial.print(payloadstring);
    Serial.println(F("'"));
    Serial.println("Parsing input failed!");
    return rvalue;
  }

  if (clientnumber == int(myObject["led"])){ // check if the response is for this client
    rvalue = int(myObject["value"]);
    servertime = (unsigned long)(myObject["servertime"]);
    pollingfromserver = (unsigned long)(myObject["pollingtime"]);
    Serial.print(F("Time between send server messages (on server): "));
    Serial.print((servertime - prevservertime));
    Serial.println(F(" milliseconds"));
    prevservertime = servertime;
  }
  else{
    Serial.print("JSON object = ");
    Serial.println(myObject);
    Serial.println(F("Wrong message received, not intended for this client"));
  }

  // // myObject.keys() can be used to get an array of all the keys in the object
  // JSONVar keys = myObject.keys();
  // for (int i = 0; i < keys.length(); i++) {
  //   JSONVar value = myObject[keys[i]];
    
  //   Serial.print(keys[i]);
  //   Serial.print(" = ");
  //   Serial.println(value);
  //   if (keys[i] == "led"){ // 1st item
  //     correctclient = clientnumber == value;
  //   }
  //   else { // 2nd item
  //     if (correctclient){
  //       rvalue = value;
  //     }
  //   }
  // }
  return rvalue;
}

bool timelapsed(unsigned long timestamp, bool newval=false){
  static unsigned long tracktime = 0;
  if (newval){
    pollinginterval = pollingfromserver;
  }
  else {
    if(timestamp < tracktime) return false;
  }
  tracktime = millis() + pollinginterval; // make sure to get 'fresh' timestamp to avoid processing time influences
  Serial.print(F("Scheduling next request for: "));
  Serial.println(tracktime);
  return true;
}

void setup() {

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

  // Serial.println();
  // for (uint8_t t = 4; t > 0; t--) {
  //   Serial.printf("[SETUP] WAIT %d...\n", t);
  //   Serial.flush();
  //   delay(1000);
  // }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("Local_AP", "local_148_AP"); // local Ap from controller

  Serial.println("HTTP client started");
  digitalWrite(led, 1); // turn onboard LED off
}

String composerequest(int composeval){
  return "http://192.168.4.1/led?led" + String(clientid) + "=" + String(composeval);
}

bool dorequest = false; // request updates regularly
WiFiClient client;
HTTPClient http;
String ledrequest = ""; // composerequest(9); // "http://192.168.4.1/led?led" + String(clientid) + "=9";

int remledval = 9;

void loop() {

  runningtime = millis();

  dorequest = timelapsed(runningtime);

  if (dorequest){
    if ((WiFiMulti.run() == WL_CONNECTED)) { // check WiFi connection
      ledrequest = composerequest(led_val);
      Serial.print(F("Request to server: "));
      Serial.println(ledrequest);
      
      http.begin(client, ledrequest);
      int httpCode = http.GET();
      if (httpCode > 0) { // httpCode will be negative on error
        // HTTP header has been send and Server response header has been handled

        // data found in response from server
        if (httpCode == HTTP_CODE_OK) {
          String payload = http.getString();

          remledval = led_val;
          led_val = parseresult(clientid, payload);
          if (led_val != remledval){
            //pollingtime = 1000;
            Serial.print(F("Changed led to: "));
            Serial.println(led_val);
          }
          // else {
          //   if (pollingtime < 5000){
          //     pollingtime += 1000;
          //   }
          // }
          if (led_val < 2){ // local LED on this client board
            digitalWrite(led, led_val);
          }
        }
      } else {
        Serial.printf("[HTTP] GET... code: %d, error: %s\n", httpCode, http.errorToString(httpCode).c_str());
      }

      http.end();
      if (pollinginterval != pollingfromserver){ // if value update, calculate new request time
        Serial.print(F("Changed polling to: "));
        Serial.println(pollingfromserver);
        dorequest = timelapsed(runningtime, true);
      }
    } // WiFi connection
  } // dorequest

}
