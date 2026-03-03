extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const int led = LED_BUILTIN;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// --------------------
// WiFi Settings
// --------------------
const char* ssid = "T24_optout";
const char* password = "T24T24T24";

ESP8266WebServer server(80);

// --------------------
// ESP-NOW Receive Callback
// --------------------
void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  static unsigned long rcount = 0;
  rcount += 1;
  Serial.print("ESP-NOW Received ");
  Serial.print(rcount);
  Serial.print(" from ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(macStr);
  Serial.print(" | Data: ");
  Serial.write(data, len);
  Serial.println();
}

// Callback when data is sent
void onDataSent(uint8_t *mac_addr, uint8_t status) {
  static unsigned long scount = 0;
  scount += 1;
  Serial.print("Send Status ");
  Serial.print(scount);
  Serial.print(": ");
  Serial.println(status == 0 ? "Success" : "Fail");
}

// --------------------
// Setup
// --------------------
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

  // WiFi STA mode (required for ESP-NOW)
  WiFi.mode(WIFI_STA);
  //WiFi.mode(WIFI_AP_STA);  // Enable both AP and Station modes
  WiFi.disconnect();

  // Connect to WiFi for webserver
  Serial.print("Connecting to WiFi ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // ESP-NOW init
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  // Add broadcast peer (improves reliability)
  //esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  Serial.println("ESP-NOW Gateway Ready");
  digitalWrite(led, 1); // turn onboard LED off
}

// --------------------
// Main Loop
// --------------------
void loop() {
  const char msg[] = "Hello from Gateway !";
  esp_now_send(broadcastAddress, (uint8_t *)msg, sizeof(msg));
  delay(5000);
}
