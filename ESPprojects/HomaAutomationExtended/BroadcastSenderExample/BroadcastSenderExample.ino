extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void OnDataSent(uint8_t *mac_addr, uint8_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == 0 ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);

  // Add broadcast peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}

void loop() {
  const char msg[] = "Hello broadcast!";
  esp_now_send(broadcastAddress, (uint8_t *)msg, sizeof(msg));
  delay(2000);
}
