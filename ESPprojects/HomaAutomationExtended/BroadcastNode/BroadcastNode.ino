extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>

const int led = LED_BUILTIN;
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// uint8_t gateWayAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// uint8_t GW1_Address[] = { 0x48, 0x3F, 0xDA, 0x69, 0xCB, 0x61};
uint8_t BC1_Address[] = { 0x68, 0xC6, 0x3A, 0xFC, 0x23, 0x76};
uint8_t GW1_Address[] = { 0x4A, 0x3F, 0xDA, 0x69, 0xCB, 0x61};
//uint8_t BC1_Address[] = { 0x6A, 0xC6, 0x3A, 0xFC, 0x23, 0x76};

const char ackmsg[] = "Acknowledge!";

// Callback when data is received
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
  Serial.write(data, len - 1);
  Serial.print(" at: ");
  Serial.println(millis());

  // send acknowledge message
  esp_now_send(mac, (uint8_t *)ackmsg, sizeof(ackmsg));
}

// Callback when data is sent
void onDataSent(uint8_t *mac_addr, uint8_t status) {
  static unsigned long scount = 0;
  scount += 1;
  Serial.print("Send Status ");
  Serial.print(scount);
  Serial.print(": ");
  Serial.print(status == 0 ? "Success" : "Fail");
  Serial.print(" at: ");
  Serial.println(millis());
}

bool timepassing(unsigned long curtime, unsigned long duration){
  static unsigned long rtime = 0;
  if(rtime + duration > curtime) return false;
  rtime = millis();
  return true;
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

  // ESP-NOW requires WiFi in STA mode
  WiFi.mode(WIFI_STA);
  //WiFi.mode(WIFI_AP_STA);
  wifi_promiscuous_enable(1);   // required to allow channel change
  wifi_set_channel(4);         // choose your channel (1–13)
  wifi_promiscuous_enable(0);
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
  //uint8_t ch = 1; // from the gateway node WiFi.channel();
  //esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 4, NULL, 0);
  esp_now_add_peer(GW1_Address, ESP_NOW_ROLE_COMBO, 4, NULL, 0);

  Serial.print(F("ESP-NOW channel 4, "));
  Serial.println(F("ESP-NOW Transceiver Ready"));
  digitalWrite(led, 1); // turn onboard LED off
}

const char msg[] = "Hello from transceiver!";
unsigned long runningtime = 0;
bool action = false;

// --------------------
// Main Loop
// --------------------
void loop() {

  runningtime = millis();

  action = timepassing(runningtime, 9000);
  if (action){
    esp_now_send(GW1_Address, (uint8_t *)msg, sizeof(msg));
  }
}
