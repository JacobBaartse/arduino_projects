extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>

const int led = LED_BUILTIN;
const int buttonPin = D3; 
bool devicepaired = false;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t GW1_Address[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const char ackmsg[] = "Acknowledge!";
const char pairmsg[] = "PAIR";

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
  // String datahere(data);
  // Serial.println(datahere);

  if (!devicepaired) {
    if (datahere == "HIERO") { // consider pairing found
      for ( int id = 0; id < 6; id++ ){
        GW1_Address[id] = mac[id];
      }
      // Add broadcast peer (improves reliability)
      esp_now_add_peer(GW1_Address, ESP_NOW_ROLE_COMBO, 4, NULL, 0);
      devicepaired = true;
    }
  }

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

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}

// --------------------
// Setup
// --------------------
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
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
  wifi_promiscuous_enable(1);   // required to allow channel change
  wifi_set_channel(4);         // choose your channel (1–13)
  wifi_promiscuous_enable(0);

  // local MAc address
  readMacAddress();

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  // Set role and register callbacks
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPress, FALLING); // trigger when button pressed

  Serial.print(F("ESP-NOW channel 4, "));
  Serial.println(F("ESP-NOW Transceiver Ready"));
  digitalWrite(led, 1); // turn onboard LED off
}

const char msg[] = "Hello from transceiver!";
const char buttonmsg[] = "Button pressed (BC1).";
unsigned long runningtime = 0;
bool action = false;
bool buttonpressed = false;

void handle_button(bool pressed, unsigned long timing) {
  static unsigned long btime = 0;
  static bool buttonstate = false;

  if(buttonstate){
    int butstate = digitalRead(buttonPin); // check current status of the button
    if (butstate == LOW) {  // button still pressed within the time period
      btime = timing;
      // Serial.println(F("Button press extension"));
      return;
    }
    if (btime + 2000 < timing){
      buttonpressed = false;
      buttonstate = false;
      Serial.print(F("Button can be pressed again "));
      Serial.println(millis());
    }
  }
  if (pressed) {
    btime = timing;
    buttonstate = true;
    buttonpressed = true;
    Serial.print(F("Button press: "));
    Serial.println(millis());
    esp_now_send(GW1_Address, (uint8_t *)buttonmsg, sizeof(buttonmsg));
    return;
  }
}

// --------------------
// Main Loop
// --------------------
void loop() {

  runningtime = millis();

  action = timepassing(runningtime, 9000);
  if (action){
    if (devicepaired){
      esp_now_send(broadcastAddress, (uint8_t *)msg, sizeof(msg));
    }
    else{
      esp_now_send(GW1_Address, (uint8_t *)pairmsg, sizeof(pairmsg));
    }
  }

  handle_button(false, runningtime);
  
}

ICACHE_RAM_ATTR void buttonPress(){
  // Serial.print(F("Button press: "));
  // Serial.println(millis());
  handle_button(true, millis());
}
