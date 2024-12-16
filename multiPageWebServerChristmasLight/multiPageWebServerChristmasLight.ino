#include <WiFiS3.h>
#include "WiFiSSLClient.h"
#include "arduino_secrets.h"
#include "index.h"
#include "temperature.h"
#include "light.h"
#include "lamp.h"
#include "RTC.h"
#include "clock.h"
#include "error_404.h"
#include "error_405.h"
#include <Adafruit_SH110X.h>  //Adafruit SH110X by Adafruit
#include <Adafruit_GFX.h>
#include "FreeSerif12pt7b_special.h"  //https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include "IR_receiver.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define display_i2c_Address 0x3c //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define display_i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define OLED_RESET     -1
#include "DHT.h"
#include "rfzender.h"
#define DHT22_PIN 12
#define RF_PIN 8
#define RELAY_PIN 9
#define PIR_PIN 11

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

const int lightSensorPin = A0;   // select the input pin of the light sensor.
int sensorValue = 0;  // variable to store the value coming from the sensor


const char ssid[] = SECRET_SSID;  // change your network SSID (name)
const char pass[] = SECRET_PASS;   // change your network password (use for WPA, or use as key for WEP)
String lamp_state1 = "uit";
String lamp_state2 = "uit";
String lamp_state3 = "uit";

int status = WL_IDLE_STATUS;
bool charging = false;
float temperature_start_battery = 0.0;
bool auto_lights_on = false;
long prev_wifi_time = 0;

WiFiServer server(80);

String getLight_value(){
  sensorValue = analogRead(lightSensorPin);
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

void display_oled( bool clear, int x, int y, String text, bool activate){
  if (debug) Serial.println(text);
  if (clear) display.clearDisplay();
  display.setCursor(x,y);
  display.print(text);
  if (activate) display.display();
}

int activate_display = 5;  // number of seconds the display will be active
long last_update_time = 0;

void update_display(){
  long millis_now = millis();
  if (activate_display > 0){
    if ((millis_now -last_update_time) > 1000){  // only update the screen once a second
      if (debug) Serial.println("update display");
      last_update_time = millis_now;
      activate_display -= 1;
      float humid  = dht22.readHumidity();
      float tempC = dht22.readTemperature();
    
      display_oled(true, 0, 16,String(tempC, 1) + " \x7F"+"C", false);  // } \x7F is converted to degrees in this special font.

      if (charging) {
        display_oled(false, 0, 40,String(humid, 0) + " % " + String(temperature_start_battery, 1), false);
      }
      else {
        display_oled(false, 0, 40,String(humid, 0) + " %" , false);
      }

      if (Minutes<10) display_oled(false, 0, 63, String(Hour) + ":0" + String(Minutes), false);
      else display_oled(false, 0, 63, String(Hour) + ":" + String(Minutes), false);
      display_oled(false, 70, 63, getLight_value(), true);
    }
  }
  else
  {
    if ((millis_now -last_update_time) > 1000){ // only clear the screen once a second
      last_update_time = millis_now;
      if (debug) Serial.println("clear display");
      display.clearDisplay();   
      display.setCursor(0,0);
      display.print("  "); // always send some data to the screen to keep it alive.
      display.display();
    }  
  }
}

void printWifiStatus() {
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  display_oled(false, 0, 18, WiFi.localIP().toString(), true);

  Serial.print("signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void setup() {
  Serial.begin(115200);
  display.begin(display_i2c_Address, true);
  display.setContrast (0); // dim display

  pinMode(GREEN_LED_PIN, INPUT);
  pinMode(GREEN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  display.clearDisplay();
  display.setFont(&FreeSerif12pt7b);
  display.setTextSize(1);  // 3 lines of 11 chars
  display.setTextColor(SH110X_WHITE);
  display.display();

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
  setup_ir_receiver();
}


void  battery_charge(boolean activate){
  if (activate){
    charging = true;
    temperature_start_battery = dht22.readTemperature();
    digitalWrite(RELAY_PIN, HIGH);
  }
  else{
    charging = false;
    digitalWrite(RELAY_PIN, LOW);
  }
}

void webserver(){
   // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    String HTTP_req = "";
    prev_wifi_time = millis();
    while (client.connected()) {
      if (client.available()) {
        if (debug) Serial.println("New HTTP Request");
        HTTP_req = client.readStringUntil('\n');  // read the first line of HTTP request
        if (debug) Serial.print("<< ");
        if (debug) Serial.println(HTTP_req);  // print HTTP request to Serial Monitor
        break;
      }
      businessLogic();
      if ((millis() - prev_wifi_time) > 30000){
        client.stop();
        break;
      }
    }

    // read the remaining lines of HTTP request header
    prev_wifi_time = millis();
    while (client.connected()) {
      if (client.available()) {
        String HTTP_header = client.readStringUntil('\n');  // read the header line of HTTP request

        if (HTTP_header.equals("\r"))  // the end of HTTP request
          break;

        //Serial.print("<< ");
        if (debug) Serial.println(HTTP_header);  // print HTTP request to Serial Monitor
      }
      businessLogic();
      if ((millis() - prev_wifi_time) > 30000){
        client.stop();
        break;
      }
    }

    int page_id = 0;

    page_id = PAGE_ERROR_405;
    if (HTTP_req.indexOf("GET") == 0) {  // check if request method is GET
      page_id = PAGE_ERROR_404;
      if (HTTP_req.indexOf("GET / ") > -1 || HTTP_req.indexOf("GET /index") > -1) page_id = PAGE_HOME;
      if (HTTP_req.indexOf("GET /temperature") > -1)                              page_id = PAGE_TEMPERATURE;
      if (HTTP_req.indexOf("GET /light") > -1)                                    page_id = PAGE_LIGHT;

      if (HTTP_req.indexOf("GET /lamp?1") > -1) light(true, 1);
      if (HTTP_req.indexOf("GET /lamp?2") > -1) light(false, 1);
      if (HTTP_req.indexOf("GET /lamp?3") > -1) light(true, 2);
      if (HTTP_req.indexOf("GET /lamp?4") > -1) light(false, 2);
      if (HTTP_req.indexOf("GET /lamp?5") > -1) light(true, 3);
      if (HTTP_req.indexOf("GET /lamp?6") > -1) light(false, 3);
      if (HTTP_req.indexOf("GET /lamp?7") > -1) all_lights(true);
      if (HTTP_req.indexOf("GET /lamp?8") > -1) all_lights(false);
      if (HTTP_req.indexOf("GET /lamp") > -1 )                                   page_id = PAGE_LAMP;

      if (HTTP_req.indexOf("GET /battery?1") > -1 ) battery_charge(true);
      if (HTTP_req.indexOf("GET /battery?0") > -1 ) battery_charge(false);
      if (HTTP_req.indexOf("GET /battery") > -1 )                                page_id = PAGE_HOME;
    }
    if (page_id == PAGE_ERROR_404) client.println("HTTP/1.1 404 Not Found");
    else if (page_id == PAGE_ERROR_405) client.println("HTTP/1.1 405 Method Not Allowed");
    else client.println("HTTP/1.1 200 OK");

    client.println("Content-Type: text/html");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println();                     // the separator between HTTP header and body

    // send the HTTP response body
    String html;
    String lamp_state;
    switch (page_id) {
      case PAGE_HOME:
        if (debug) Serial.println("return the hompeage");
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
    if (debug) Serial.println("client stop");
    client.stop();
    if (debug) Serial.println("client stop done");
  }
}

void light(bool turn_on, int light_id){
  if (turn_on){
    switch(light_id){
      case 1: send_code(RF_LIGHT_ON1);lamp_state1 = "aan"; break;
      case 2: send_code(RF_LIGHT_ON2);lamp_state2 = "aan"; break;
      case 3: send_code(RF_LIGHT_ON3);lamp_state3 = "aan"; break;
    }
  }
  else{
    switch(light_id){
    case 1: send_code(RF_LIGHT_OFF1);lamp_state1 = "uit"; break;
    case 2: send_code(RF_LIGHT_OFF2);lamp_state2 = "uit"; break;
    case 3: send_code(RF_LIGHT_OFF3);lamp_state3 = "uit"; break;
    }
  }
}

bool get_light_state(int light_id){
  switch(light_id){
    case 1: if (lamp_state1 == "uit") return false; break;
    case 2: if (lamp_state2 == "uit") return false; break;
    case 3: if (lamp_state3 == "uit") return false; break;
  }
  return true;
}

void toggle_light(int light_id){
  if (get_light_state(light_id)) light(false, light_id);
  else light(true, light_id);
}

void all_lights(bool turn_on){
  if (turn_on){
    send_code(RF_LIGHT_ALL_ON);
    lamp_state1 = "aan";
    lamp_state2 = "aan";
    lamp_state3 = "aan";
  }
  else{
    send_code(RF_LIGHT_ALL_OFF);
    lamp_state1 = "uit";
    lamp_state2 = "uit";
    lamp_state3 = "uit";
  }
}

void businessLogic(){
  update_clock();
  bool low_light = false;
  if (analogRead(lightSensorPin) < 50) low_light = true;
  bool eavening = false;
  if (Hour>15 && Hour<23) eavening = true;
  if (Hour==22 && Minutes >=30) eavening = false;
  if (auto_lights_on){
    if (!eavening){
      all_lights(false);
      auto_lights_on = false;
    }
  }
  if (!auto_lights_on){
    if (eavening){
      if (low_light){
        all_lights(true);
        auto_lights_on = true;
      }
    }
  }
  if (digitalRead(GREEN_BUTTON_PIN)==PUSHED){
    if (debug) Serial.println("green button pressed");
    activate_display=6;
    pinMode(GREEN_LED_PIN, OUTPUT);
    digitalWrite(GREEN_LED_PIN, HIGH);  
    light(true, 1);
    if (digitalRead(GREEN_BUTTON_PIN)==PUSHED) delay(500);
    if (digitalRead(GREEN_BUTTON_PIN)==PUSHED) light(true, 2);
    if (digitalRead(GREEN_BUTTON_PIN)==PUSHED) delay(500);
    if (digitalRead(GREEN_BUTTON_PIN)==PUSHED) light(true, 3);
  } 
  else{
    pinMode(GREEN_LED_PIN, INPUT);
  }
  if (digitalRead(PIR_PIN)==HIGH){
    activate_display=5;
    pinMode(GREEN_LED_PIN, INPUT_PULLUP);
    pinMode(RED_LED_PIN, INPUT_PULLUP);
  }
  else{
    pinMode(GREEN_LED_PIN, INPUT);
    pinMode(RED_LED_PIN, OUTPUT);   
    digitalWrite(RED_LED_PIN, LOW); 
  }
  if (digitalRead(RED_BUTTON_PIN)==PUSHED){
    if (debug) Serial.println("red button pressed");
    activate_display=5;
    pinMode(RED_LED_PIN, OUTPUT);  
    digitalWrite(RED_LED_PIN, HIGH);
    all_lights(false);
  } 

  if (charging)
    if ( (dht22.readTemperature() - temperature_start_battery) > 5.0) battery_charge(false);

  update_display();
  int ir_value = one_loop_irreceiver();
  if (ir_value>0){
      if (debug) Serial.print("0x");
      if (debug) Serial.println(ir_value, HEX);
      if (ir_value == 0x1cc) all_lights(false);
      else if (ir_value == 0x1cd) all_lights(true);      
      else if (ir_value == 0x80) toggle_light(1);
      else if (ir_value == 0x81) toggle_light(2);
      else if (ir_value == 0x82) toggle_light(3);
      else light(true, 1);
  }
}


void loop() {
  webserver();
  businessLogic();
}

