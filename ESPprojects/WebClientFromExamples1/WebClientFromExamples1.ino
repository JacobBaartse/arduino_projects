/*
    This sketch sends a string to a TCP server, and prints a one-line response.
    You must run a TCP server in your local network.
    For example, on Linux you can use this command: nc -v -l 3000
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#ifndef STASSID
#define STASSID "ESP_AP"
#define STAPSK "12345678"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

const char* host = "192.168.4.1";
const uint16_t port = 80;

ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);

  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.println(F("Starting ESP device"));
  Serial.flush();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  delay(500);
}

void loop() {
  Serial.print("connecting to: ");
  Serial.print(host);
  Serial.print(':');
  Serial.println(port);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }

  // This will send the request to the server
  client.println("hello from ESP8266");

  // read back one line from server
  Serial.println("receiving from remote server");
  //String line = client.readStringUntil('\r');
  String line = client.readStringUntil('\n');
  Serial.println(line);

  Serial.println("closing connection");
  client.stop();

  Serial.println("wait 5 sec...");
  delay(5000);
}


// // /*
// //  * This ESP32 code is created by esp32io.com
// //  *
// //  * This ESP32 code is released in the public domain
// //  *
// //  * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-http-request
// //  */

// // #include <ESP8266WiFi.h>
// // //#include <WiFi.h>
// // #include <HTTPClient.h>

// // const char WIFI_SSID[] = "ESP_AP";         // CHANGE IT 
// // const char WIFI_PASSWORD[] = "12345678"; // CHANGE IT

// // String HOST_NAME   = "http://192.168.4.1"; // CHANGE IT
// // String PATH_NAME   = "/";  // "/products/arduino.php";      // CHANGE IT

// // void setup() {
// //   Serial.begin(115200);

// //   Serial.println(F(" "));
// //   Serial.println(F(" "));
// //   Serial.println(F("Starting ESP device"));
// //   Serial.flush();
// //   Serial.print(__FILE__);
// //   Serial.print(F(", creation/build time: "));
// //   Serial.println(__TIMESTAMP__);
// //   Serial.flush(); 

// //   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
// //   Serial.println("Connecting");
// //   while(WiFi.status() != WL_CONNECTED) {
// //     delay(500);
// //     Serial.print(".");
// //   }
// //   Serial.println("");
// //   Serial.print("Connected to WiFi network with IP Address: ");
// //   Serial.println(WiFi.localIP());
  
// //   HTTPClient http;

// //   http.begin(HOST_NAME + PATH_NAME); //HTTP
// //   int httpCode = http.GET();

// //   // httpCode will be negative on error
// //   if(httpCode > 0) {
// //     // file found at server
// //     if(httpCode == HTTP_CODE_OK) {
// //       String payload = http.getString();
// //       Serial.println(payload);
// //     } else {
// //       // HTTP header has been send and Server response header has been handled
// //       Serial.printf("[HTTP] GET... code: %d\n", httpCode);
// //     }
// //   } else {
// //     Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
// //   }

// //   http.end();
// // }

// // void loop() {

// // }


// https://github.com/PedroFnseca/esp32-http-client

// #include <ESP8266WiFi.h>
// #include "ESP32HTTPClient.h"

// ESP32HTTPClient client("https://jsonplaceholder.typicode.com");

// void setup() {
//     Serial.begin(115200);
//     WiFi.begin("SSID", "PASS");
    
//     while (WiFi.status() != WL_CONNECTED) delay(100);

//     // Prepare a variable
//     int userId = 0;

//     // Fetch data
//     // API returns: { "userId": 1, "id": 1, "title": "..." }
//     client.get("/todos/1").getBody("userId", &userId);

//     Serial.printf("User ID fetched from API: %d\n", userId);
// }

// void loop() {


// }
