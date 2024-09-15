/*
  Web client
 This sketch connects to a website (http://worldtimeapi.org)
 using a WiFi 
 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.
 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.
 */
#include <SPI.h>
#include <WiFi.h>
#include "RTC.h"
#include "Arduino_LED_Matrix.h"
#include "clock.h"
#include "arduino_secrets.h"


const char ssid[] = SECRET_SSID;  // change your network SSID (name)
const char pass[] = SECRET_PASS;   // change your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiClient client;

String get_time_form_worldtimeapi_org(){
  String readString; 
  char server[] = "worldtimeapi.org"; 

  if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // send the HTTP request:
    client.println("GET /api/timezone/Europe/Amsterdam HTTP/1.1");
    client.println("Host: worldtimeapi.org");
    client.println("Connection: close");
    client.println();
    Serial.println("get request send to worldtimeapi.org.");

  };
  int counter = 0;
  while (!client.available()){
    counter += 1;
    delay(1000);
  }
  Serial.print("waited for website: ");
  Serial.print(counter);
  Serial.println(" seconds");

  while (client.available()) {
    char c = client.read();
    readString += c;
  }
  Serial.println(readString);

  // todo parse the readString to get the utc time.

  Serial.println("disconnecting from worldtimeapi.org.");
  client.stop();

  int day_of_week_start =  readString.indexOf("\"day_of_week\":") + 14;
  String date_time = readString.substring(day_of_week_start, day_of_week_start+2);

  int date_time_start = readString.indexOf("\"datetime\":\"") + 12;
  date_time += readString.substring(date_time_start, date_time_start+26);

  return date_time;
}

void update_clock_time(){
  String world_time;
  world_time = get_time_form_worldtimeapi_org();
  clock_update(world_time);
}



void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(2000);
  }
  Serial.println(String("Connected to wifi ") + ssid);
  printWifiStatus();
  Serial.println("\nStarting connection to server...");

  clock_setup();
  update_clock_time();
}

unsigned long previousMillis = 0; 

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 30*60*1000)  // eens per half uur
  {
    previousMillis = currentMillis;
    update_clock_time();
  }
  clock_loop_once();
  delay(1000);
  
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}