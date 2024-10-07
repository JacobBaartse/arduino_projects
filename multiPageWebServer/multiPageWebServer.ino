/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server-multiple-pages
 */

#include <WiFiS3.h>
#include "arduino_secrets.h"
#include "index.h"
#include "temperature.h"
#include "door.h"
#include "led.h"
#include "error_404.h"
#include "error_405.h"

#include "DHT.h"
#define DHT22_PIN 12

DHT dht22(DHT22_PIN, DHT22);

#define PAGE_HOME 0
#define PAGE_TEMPERATURE 1
#define PAGE_DOOR 2
#define PAGE_LED 3

#define PAGE_ERROR_404 -1
#define PAGE_ERROR_405 -2

int wifi_ready_led = 8;
int wifi_client_active = 9;
int relay = 13;


const char ssid[] = SECRET_SSID;  // change your network SSID (name)
const char pass[] = SECRET_PASS;   // change your network password (use for WPA, or use as key for WEP)
String led_state = "OFF";

int status = WL_IDLE_STATUS;

WiFiServer server(80);


String getTemperature_humidity() {

    // read humidity
  float humid  = dht22.readHumidity();
  // read temperature as Celsius
  float tempC = dht22.readTemperature();
  if (isnan(humid) || isnan(tempC) ){
    return String(-5.0, 1) + "," + String(0.0, 1);
  }
  // // read temperature as Fahrenheit
  // float tempF = dht22.readTemperature(true);

  // // check if any reads failed
  // if (isnan(humi) || isnan(tempC) || isnan(tempF)) {
  //   Serial.println("Failed to read from DHT22 sensor!");
  // } else {
  //   Serial.print("DHT22# Humidity: ");
  //   Serial.print(humi);
  //   Serial.print("%");

  //   Serial.print("  |  "); 

  //   Serial.print("Temperature: ");
  //   Serial.print(tempC + 0);  // my sensor has some deviation so correct it.
  //   Serial.println("°C");
  // }
  return String(tempC, 1) + "," + String(humid, 1);              // return the simulated temperature value from 0 to 100 in float
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  dht22.begin(); 

  pinMode(wifi_ready_led, OUTPUT);
  digitalWrite(wifi_ready_led, LOW);  
  
  pinMode(wifi_client_active, OUTPUT);
  digitalWrite(wifi_client_active, LOW);

  pinMode(relay, OUTPUT);
  digitalWrite(relay, LOW);

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    Serial.println("Please upgrade the firmware");

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    if (status == WL_CONNECTED){
          delay(2000);
      break;
    }
    // wait 10 seconds for re-connecting to the wifi network:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    digitalWrite(wifi_client_active, HIGH);
    // read the first line of HTTP request header
    String HTTP_req = "";
    while (client.connected()) {
      if (client.available()) {
        // Serial.println("New HTTP Request");
        HTTP_req = client.readStringUntil('\n');  // read the first line of HTTP request
        // Serial.print("<< ");
        // Serial.println(HTTP_req);  // print HTTP request to Serial Monitor
        break;
      }
    }

    // read the remaining lines of HTTP request header
    while (client.connected()) {
      if (client.available()) {
        String HTTP_header = client.readStringUntil('\n');  // read the header line of HTTP request

        if (HTTP_header.equals("\r"))  // the end of HTTP request
          break;

        //Serial.print("<< ");
        //Serial.println(HTTP_header);  // print HTTP request to Serial Monitor
      }
    }

    // ROUTING
    // This example supports the following:
    // - GET /
    // - GET /index
    // - GET /index.html
    // - GET /temperature
    // - GET /temperature.html
    // - GET /door
    // - GET /door.html
    // - GET /led
    // - GET /led.html

    int page_id = 0;

    if (HTTP_req.indexOf("GET") == 0) {  // check if request method is GET
      if (HTTP_req.indexOf("GET / ") > -1 || HTTP_req.indexOf("GET /index") > -1) {
        // Serial.println("home page");
        page_id = PAGE_HOME;
      } else if (HTTP_req.indexOf("GET /temperature") > -1) {
        // Serial.println("temperature page");
        page_id = PAGE_TEMPERATURE;
      } else if (HTTP_req.indexOf("GET /door") > -1) {
        // Serial.println("door page");
        page_id = PAGE_DOOR;
      }else if (HTTP_req.indexOf("GET /led?1") > -1) {
          //Serial.println("led on page");
          digitalWrite(relay, HIGH);
          led_state = "aan";
          page_id = PAGE_LED;
      } else if (HTTP_req.indexOf("GET /led?0") > -1) {
          //Serial.println("led off page");
          digitalWrite(relay, LOW);
          led_state = "uit";
          page_id = PAGE_LED;
      } else if (HTTP_req.indexOf("GET /led") > -1 ) {
        // Serial.println("led page");
        page_id = PAGE_LED;
      } else {  // 404 Not Found
        // Serial.println("404 Not Found");
        page_id = PAGE_ERROR_404;
      }
    } else {  // 405 Method Not Allowed
      // Serial.println("405 Method Not Allowed");
      page_id = PAGE_ERROR_405;
    }    

    // send the HTTP response
    // send the HTTP response header
    if (page_id == PAGE_ERROR_404)
      client.println("HTTP/1.1 404 Not Found");
    else if (page_id == PAGE_ERROR_405)
      client.println("HTTP/1.1 405 Method Not Allowed");
    else
      client.println("HTTP/1.1 200 OK");

    client.println("Content-Type: text/html");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println();                     // the separator between HTTP header and body

    // send the HTTP response body
    String html;
    switch (page_id) {
      case PAGE_HOME:
        html = String(HTML_CONTENT_HOME);
        break;
      case PAGE_TEMPERATURE:
        html = String(HTML_CONTENT_TEMPERATURE);
        html.replace("TEMPERATURE_HUMID_MARKER", getTemperature_humidity());  // replace the marker by a real value
        break;
      case PAGE_DOOR:
        html = String(HTML_CONTENT_DOOR);
        html.replace("DOOR_STATE_MARKER", "OPENED");  // replace the marker by a real value
        break;
      case PAGE_LED:
        html = String(HTML_CONTENT_LED);

        html.replace("LED_STATE_MARKER", led_state);  // replace the marker by a real value
        break;
      case PAGE_ERROR_404:
        html = String(HTML_CONTENT_404);
        break;
      case PAGE_ERROR_405:
        html = String(HTML_CONTENT_405);
        break;
    }

    client.println(html);
    client.flush();
    
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    digitalWrite(wifi_client_active, LOW);
  }
}

void printWifiStatus() {
  // print your board's IP address:
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // print the received signal strength:
  Serial.print("signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  digitalWrite(wifi_ready_led, HIGH);
}
