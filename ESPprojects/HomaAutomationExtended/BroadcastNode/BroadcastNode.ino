extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Callback when data is received
void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  Serial.print("Received from ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" | Data: ");
  Serial.write(data, len);
  Serial.println();
}

// Callback when data is sent
void onDataSent(uint8_t *mac_addr, uint8_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == 0 ? "Success" : "Fail");
}

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

  // ESP-NOW requires WiFi in STA mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  // Set role and register callbacks
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  // Add broadcast peer (improves reliability)
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  Serial.println("ESP-NOW Transceiver Ready");
}

void loop() {
  const char msg[] = "Hello from transceiver!";
  esp_now_send(broadcastAddress, (uint8_t *)msg, sizeof(msg));
  delay(3000);
}
