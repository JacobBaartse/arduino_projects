extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>

void OnDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  Serial.print("Received from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.print(" | Message: ");
  Serial.write(data, len);
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Nothing needed here — callback handles everything
}
