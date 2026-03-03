extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --------------------
// WiFi + MQTT Settings
// --------------------
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* mqtt_server = "192.168.1.10";   // Your MQTT broker IP
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "espnow/incoming";

WiFiClient espClient;
PubSubClient client(espClient);

// --------------------
// ESP-NOW Receive Callback
// --------------------
void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  Serial.print("ESP-NOW Received from ");
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(macStr);
  Serial.print(" | Data: ");
  Serial.write(data, len);
  Serial.println();

  // Build MQTT topic: espnow/incoming/<MAC>
  String topic = String(mqtt_topic) + "/" + macStr;

  // Publish raw data to MQTT
  client.publish(topic.c_str(), data, len);
}

// --------------------
// MQTT Reconnect
// --------------------
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP8266_Gateway")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 3 seconds");
      delay(3000);
    }
  }
}

// --------------------
// Setup
// --------------------
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

  // WiFi STA mode (required for ESP-NOW)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Connect to WiFi for MQTT
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // MQTT setup
  client.setServer(mqtt_server, mqtt_port);

  // ESP-NOW init
  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onDataRecv);

  Serial.println("ESP-NOW Gateway Ready");
}

// --------------------
// Main Loop
// --------------------
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
}
