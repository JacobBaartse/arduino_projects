
#include <WiFiS3.h>
#include "arduino_secrets.h"
#include "index.h"
#include "temperature.h"
#include "light.h"
#include "lamp.h"
#include "RTC.h"
#include "clock.h"
#include "error_404.h"
#include "error_405.h"
#include <Adafruit_SH110X.h>
#include <Adafruit_GFX.h>
#include "FreeSerif12pt7b_special.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define OLED_RESET     -1
#include "DHT.h"
#include "rfzender.h"
#define DHT22_PIN 12
#define RF_PIN 8
#define RELAY_PIN 9

DHT dht22(DHT22_PIN, DHT22);

#define PAGE_HOME 0
#define PAGE_TEMPERATURE 1
#define PAGE_LIGHT 2
#define PAGE_LAMP 3

#define GREEN_LED_PIN 6
#define RED_LED_PIN 4
#define RED_BUTTON_PIN 5
#define GREEN_BUTTON_PIN 7
#define PUSHED LOW

const int PAGE_ERROR_404 = -1;
const int PAGE_ERROR_405 = -2;
const bool debug = false;

int lightSensorPin = A0;   // select the input pin to be measured
int sensorValue = 0;  // variable to store the value coming from the sensor


const char ssid[] = SECRET_SSID;  // change your network SSID (name)
const char pass[] = SECRET_PASS;   // change your network password (use for WPA, or use as key for WEP)
String lamp_state1 = "---";
String lamp_state2 = "---";
String lamp_state3 = "---";

int status = WL_IDLE_STATUS;
bool charging = true;
float temperature_start_battery=0.0;

WiFiServer server(80);

unsigned long prev_seconds = 0;
bool elapsed_seconds(int seconds_delay){
  unsigned long cur_seconds = millis() / 1000;
  if (cur_seconds > (prev_seconds + seconds_delay)){
    prev_seconds = cur_seconds;
    return true;
  }
  return false;
}


String getLight_value(){
  sensorValue = analogRead(lightSensorPin);
  //Serial.println(sensorValue);
  return String(sensorValue);
}


String getTemperature_humidity() {
  float humid  = dht22.readHumidity();
  float tempC = dht22.readTemperature();
  if (isnan(humid) || isnan(tempC) ){
    return String(-5.0, 1) + "," + String(0.0, 1);
  }
  return String(tempC, 1) + "," + String(humid, 1);
}

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void display_oled( bool clear, int x, int y, String text){
  if (debug){
      Serial.println(text);
  }
  else{
    if (clear) display.clearDisplay();
    display.setCursor(x,y);
    display.print(text);
    display.display();
    }
}


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
      // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!debug){
    display.begin(i2c_Address, true); // Address 0x3C default
    display.setContrast (0); // dim display
  }

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(GREEN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);

  pinMode(RELAY_PIN, OUTPUT);

  if (!debug){
    display.clearDisplay();
    display.setFont(&FreeSerif12pt7b);
    display.setTextSize(1);  // 2 lines of 11 chars
    display.setTextColor(SH110X_WHITE);
    display.display();
  }

  dht22.begin(); 
  rf_setup(RF_PIN);

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
  get_time_form_worldtimeapi_org();
}

bool auto_lights_on = false;
void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
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


    int page_id = 0;

    if (HTTP_req.indexOf("GET") == 0) {  // check if request method is GET
      if (HTTP_req.indexOf("GET / ") > -1 || HTTP_req.indexOf("GET /index") > -1) {
        // Serial.println("home page");
        page_id = PAGE_HOME;
      } else if (HTTP_req.indexOf("GET /temperature") > -1) {
        // Serial.println("temperature page");
        page_id = PAGE_TEMPERATURE;
      } else if (HTTP_req.indexOf("GET /light") > -1) {
        // Serial.println("light page");
        page_id = PAGE_LIGHT;
      }else if (HTTP_req.indexOf("GET /lamp?1") > -1) {
          //Serial.println("lamp on page");
          // digitalWrite(relay, HIGH);
          send_code(RF_LIGHT_ON1);
          lamp_state1 = "aan";
          page_id = PAGE_LAMP;
      } else if (HTTP_req.indexOf("GET /lamp?2") > -1) {
          //Serial.println("lamp off page");
          send_code(RF_LIGHT_OFF1);
          lamp_state1 = "uit";
          page_id = PAGE_LAMP;
      } else if (HTTP_req.indexOf("GET /lamp?3") > -1) {
          //Serial.println("lamp off page");
          send_code(RF_LIGHT_ON2);
          lamp_state2 = "aan";
          page_id = PAGE_LAMP;
      } else if (HTTP_req.indexOf("GET /lamp?4") > -1) {
          //Serial.println("lamp off page");
          send_code(RF_LIGHT_OFF2);
          lamp_state2 = "uit";
          page_id = PAGE_LAMP;
      } else if (HTTP_req.indexOf("GET /lamp?5") > -1) {
          //Serial.println("lamp off page");
          send_code(RF_LIGHT_ON3);
          lamp_state3 = "aan";
          page_id = PAGE_LAMP;
      } else if (HTTP_req.indexOf("GET /lamp?6") > -1) {
          //Serial.println("lamp off page");
          send_code(RF_LIGHT_OFF3);
          lamp_state3 = "uit";
          page_id = PAGE_LAMP;
      } else if (HTTP_req.indexOf("GET /lamp?7") > -1) {
          //Serial.println("lamp off page");
          send_code(RF_LIGHT_ALL_ON);
          lamp_state1 = "aan";
          lamp_state2 = "aan";
          lamp_state3 = "aan";
          page_id = PAGE_LAMP;
      } else if (HTTP_req.indexOf("GET /lamp?8") > -1) {
          //Serial.println("lamp off page");
          send_code(RF_LIGHT_ALL_OFF);
          lamp_state1 = "uit";
          lamp_state2 = "uit";
          lamp_state3 = "uit";
          page_id = PAGE_LAMP;
      } else if (HTTP_req.indexOf("GET /lamp") > -1 ) {
        // Serial.println("lamp page");
        page_id = PAGE_LAMP;
      } else if (HTTP_req.indexOf("GET /battery?1") > -1 ) {
        Serial.println("battery on page");
        charging = true;
        temperature_start_battery = dht22.readTemperature();
        digitalWrite(RELAY_PIN, HIGH);
        page_id = PAGE_HOME;
      } else if (HTTP_req.indexOf("GET /battery?0") > -1 ) {
        Serial.println("battery off page");
        charging = false;
        digitalWrite(RELAY_PIN, LOW);
        page_id = PAGE_HOME;
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
    String lamp_state;
    switch (page_id) {
      case PAGE_HOME:
        html = String(HTML_CONTENT_HOME);
        break;
      case PAGE_TEMPERATURE:
        html = String(HTML_CONTENT_TEMPERATURE);
        html.replace("TEMPERATURE_HUMID_MARKER", getTemperature_humidity());  // replace the marker by a real value
        break;
      case PAGE_LIGHT:
        html = String(HTML_CONTENT_LIGHT);
        html.replace("LIGHT_STATE_MARKER", getLight_value());  // replace the marker by a real value
        break;
      case PAGE_LAMP:
        html = String(HTML_CONTENT_LAMP);

        lamp_state = String(" "+lamp_state1 + " " + lamp_state2 + " " + lamp_state3);
        html.replace("LAMP_STATE_MARKER", lamp_state);  // replace the marker by a real value
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
    delay(10);

    // close the connection:
    client.stop();
  }
  if (elapsed_seconds(5)){
    update_clock();
    float humid  = dht22.readHumidity();
    float tempC = dht22.readTemperature();
  
    display_oled(true, 0, 16,String(tempC, 1) + " }C ");  // } is converted to degrees in this font.

    if (charging) {
      display_oled(false, 0, 40,String(humid, 0) + " % " + String(temperature_start_battery, 1));
    }
    else {
      display_oled(false, 0, 40,String(humid, 0) + " %" );
    }

    if (Minutes<10) display_oled(false, 0, 63, String(Hour) + ":0" + String(Minutes));
    else display_oled(false, 0, 63, String(Hour) + ":" + String(Minutes));
    display_oled(false, 70, 63, getLight_value());

    bool low_light = false;
    if (analogRead(lightSensorPin) < 50) low_light = true;
    bool eavening = false;
    if (Hour>16 && Hour<23) eavening = true;
    if (Hour==22 && Minutes >=30) eavening = false;
    if (auto_lights_on)
      if (!eavening){
        send_code(RF_LIGHT_ALL_OFF);
        auto_lights_on = false;
      }
    if (!auto_lights_on)
      if (eavening)
        if (low_light){
          send_code(RF_LIGHT_ALL_ON);
          auto_lights_on = true;
        }
    } // elapsed_seconds(5)   

    if (digitalRead(GREEN_BUTTON_PIN)==PUSHED){
      digitalWrite(GREEN_LED_PIN, HIGH);  
      send_code(RF_LIGHT_ALL_ON);
    } 
    else digitalWrite(GREEN_LED_PIN, LOW);  
    if (digitalRead(RED_BUTTON_PIN)==PUSHED){
      digitalWrite(RED_LED_PIN, HIGH);
      send_code(RF_LIGHT_ALL_OFF);
    } 
    else digitalWrite(RED_LED_PIN, LOW);  

    if (charging){
      if ( (dht22.readTemperature() - temperature_start_battery) > 3.0){
        charging = false;
        digitalWrite(RELAY_PIN, LOW);
      }
    }

  }

void printWifiStatus() {
  // print your board's IP address:
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  display_oled(false, 0, 18, WiFi.localIP().toString());

  // print the received signal strength:
  Serial.print("signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}
