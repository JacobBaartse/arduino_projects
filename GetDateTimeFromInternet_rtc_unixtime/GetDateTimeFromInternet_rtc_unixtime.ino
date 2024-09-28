#include <SPI.h>
#include <WiFi.h>
#include "RTC.h"
#include "Arduino_LED_Matrix.h"
#include "clock.h"
#include "arduino_secrets.h"


const char ssid[] = SECRET_SSID;  // change your network SSID (name)
const char pass[] = SECRET_PASS;   // change your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

WiFiClient client;

unsigned long startup_unix_time_internet;


String get_time_form_worldtimeapi_org(bool debug, bool first_time){
  String readString; 
  char server[] = "worldtimeapi.org"; 

  if (client.connect(server, 80)) {
    // Serial.println("connected to server");
    // send the HTTP request:
    client.println("GET /api/timezone/Europe/Amsterdam HTTP/1.1");
    client.println("Host: worldtimeapi.org");
    client.println("Connection: close");
    client.println();
    if (debug){
      Serial.println("get request send to worldtimeapi.org.");
    }

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
  if (debug){
    Serial.println(readString);
  }

  // todo parse the readString to get the utc time.

  // Serial.println("disconnecting from worldtimeapi.org.");
  client.stop();

  int unix_time_start = readString.indexOf("\"unixtime\":") + 11;
  String date_time = readString.substring(unix_time_start, unix_time_start+12);
  int date_time_start = readString.indexOf("\"datetime\":\"") + 13;
  date_time += readString.substring(date_time_start, date_time_start+26);
  int unix_time_len = date_time.indexOf(",");

  int utc_offset_start = readString.indexOf("\"utc_offset\":\"") + 14;
  String utc_offset_string = readString.substring(utc_offset_start, utc_offset_start + 6);
  Serial.print("offset ");
  Serial.println(utc_offset_string);
  int timezone_minutes = utc_offset_string.substring(1,3).toInt()*60 + utc_offset_string.substring(4,6).toInt();


  if (utc_offset_string.substring(0,1) == "-") timezone_minutes = 0-timezone_minutes;


  if (first_time){
    startup_unix_time_internet = date_time.substring(0, unix_time_len).toInt();

    startup_unix_time_internet += timezone_minutes * 60;

    // correct startup value for clock for internet lag..
    startup_unix_time_internet += 1;
    set_clock(startup_unix_time_internet);   
  }
  return date_time;
}


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(2000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
  Serial.println("\nStarting connection to server...");

  clock_setup();
}


String world_time;

void loop() {
  world_time = get_time_form_worldtimeapi_org(true, true);
  while (true)
  {

      //world_time = get_time_form_worldtimeapi_org(false, false);
      //Serial.println(world_time);    
      clock_loop_once();
      //Serial.println(get_clock());
      //Serial.println("----------------------");
      delay(1000);
  }
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
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}