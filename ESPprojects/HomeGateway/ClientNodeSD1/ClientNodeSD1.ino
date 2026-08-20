extern "C" {
  #include <espnow.h>
}
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h> 
//#include "FreeSerif12pt7b_special.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include "font_16pix_high.h"
#include <Adafruit_SSD1306.h> // Adafruit SSD 1306 by Adafruit
#include <qrcode.h>
#include "display_sd.h"
#include "esp_now.h"

// #define SCREEN_WIDTH 128 // OLED display width, in pixels
// #define SCREEN_HEIGHT 64 // OLED display height, in pixels
// #define i2c_Address 0x3C //initialize with the I2C addr 0x3C Typically eBay OLED's
// //#define i2c_Address 0x3D //initialize with the I2C addr 0x3D Typically Adafruit OLED's
// #define OLED_RESET -1

// Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// enum DisplayState { Off, Dim, On };

// uint8_t displaystatus = DisplayState::Off;
// // void display_oled(bool clear, int x, int y, const char* text) {
// //   if (displaystatus == DisplayState::Off) return;
// //   // if (clear) display.clearDisplay();
// //   display.setCursor(x, y);
// //   display.print(text);
// //   // display.display();
// // }

// DisplayState setDisplay(DisplayState statustoset) {
//   static DisplayState displaystatus = DisplayState::Dim;
//   switch(statustoset){
//     case DisplayState::Dim:
//       display.ssd1306_command(SSD1306_DISPLAYON);
//       //display.dim(true); // dim display
//       //displaystatus = DisplayState::Dim; // this display does not support dim
//       //displaystatus = DisplayState::On;
//       display.ssd1306_command(SSD1306_SETCONTRAST);
//       display.ssd1306_command(1);
//       displaystatus = DisplayState::Dim; // this display does not support dim ??
//       break;
//     case DisplayState::On:
//       display.ssd1306_command(SSD1306_DISPLAYON);
//       display.dim(false);
//       displaystatus = DisplayState::On;
//       break;
//     //case DisplayState::Off:
//     default:
//       display.ssd1306_command(SSD1306_DISPLAYOFF);
//       displaystatus = DisplayState::Off;
//   }
//   return displaystatus;
// }

// // void clear_display(){
// //   display.clearDisplay();
// //   display.display();
// // }

// char Lines[4][101] = {
//   "Welcome Leo",
//   "Demo {small disp.}", 
//   "Whats up?",
//   "Hello World"
// };  
// uint8_t LinesYPos[4] = { 16, 32, 48, 64 };
// uint8_t upddisplay = 200;

const int led = LED_BUILTIN;
const int buttonPin = D3; 


// function to indicate the passing of certain duration
bool timepassing(unsigned long curtime, unsigned long duration) {
  static unsigned long rtime = 0;
  if(rtime + duration > curtime) return false;
  rtime = millis(); // get fresh time to base the new interval on
  return true;
}

// --------------------
// Setup
// --------------------
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0); // turn onboard LED on
  Serial.begin(115200);

  display.begin(SSD1306_SWITCHCAPVCC, i2c_Address);
  displaystatus = setDisplay(DisplayState::Dim);
  display.clearDisplay();
  //display.setFont(&FreeSerif12pt7b);
  display.setFont(&font_16_pix);
  display.setTextSize(1); // 3 lines of 10-12 chars
  display.setTextColor(SSD1306_WHITE);
  display.setTextWrap(true); 
  display.display();

  Serial.println(F(" "));
  Serial.println(F(" "));
  Serial.println(F("Starting ESP device"));
  Serial.flush();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.println(reftext);
  Serial.flush(); 

  // ESP-NOW requires WiFi in STA mode
  WiFi.mode(WIFI_STA);
  wifi_promiscuous_enable(1);   // required to allow channel change
  wifi_set_channel(4);          // choose your channel (1–13)
  wifi_promiscuous_enable(0);

  if (esp_now_init() != 0) {
    Serial.println(F("ESP-NOW init failed"));
    return;
  }

  // Set role and register callbacks
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(onDataRecv);
  esp_now_register_send_cb(onDataSent);

  // use the button on an interrupt hadling
  // attachInterrupt(digitalPinToInterrupt(buttonPin), buttonPress, FALLING); // trigger when button pressed

  updateDisplay();
  // display_oled(true, 0, 16, Lines[0]); 
  // display_oled(false, 0, 32, Lines[1]); 
  // display_oled(false, 0, 48, Lines[2]);  
  // display_oled(false, 0, 64, Lines[3]); 

  // Display the QR code at setup time
  generateQRCode("QR code and text");

  Serial.print(F("ESP-NOW channel 4, "));
  Serial.println(F("ESP-NOW Transceiver Ready"));
  digitalWrite(led, 1); // turn onboard LED off
}

const char buttonmsg[] = "Button pressed (BC1).";
unsigned long runningtime = 0;
bool action = false;
int actionid = 0;
// bool buttonpressed = false;

// void handle_button(bool pressed, unsigned long timing){
//   static unsigned long btime = 0;
//   static bool buttonstate = false;
//   bool bpress = pressed;

//   if (buttonstate){
//     int butstate = digitalRead(buttonPin); // check current status of the button
//     if (butstate == LOW) {  // button still pressed within the time period
//       btime = timing;
//       // Serial.println(F("Button press extension"));
//       return;
//     }
//     if (btime + 2000 < timing){
//       buttonstate = false;
//       Serial.print(F("Button can be pressed again "));
//       Serial.println(millis());
//       buttonpressed = false;
//     }
//     else {
//       bpress = false;
//     }
//   }
//   if (bpress) {
//     buttonpressed = true;
//     btime = millis();
//     buttonstate = true;
//     Serial.print(F("Button press: "));
//     Serial.println(btime);
//     sendonesp((uint8_t *)buttonmsg, sizeof(buttonmsg));
//     //esp_now_send(Server_Address, (uint8_t *)buttonmsg, sizeof(buttonmsg));
//   }
// }

const int qrlen = 50;
char qstr[qrlen]; // maximum 50 characters

// --------------------
// Main Loop
// --------------------
void loop() {

  runningtime = millis();

  action = timepassing(runningtime, 60000); // every minute
  if (action) {

    // sprintf(qstr, "%s %05d", "George", runningtime);
    // DisplayProgress(qstr);

    if (devicepaired) {
      //sendonesp((uint8_t *)msg, sizeof(msg));
      //esp_now_send(Server_Address, (uint8_t *)msg, sizeof(msg));

      pairingheartbeat--;
      devicepaired = pairingheartbeat > 0;
    }
    if (!devicepaired) {
      sendpairingsequence(0);
    }
  }

  //handle_button(false, runningtime);

  if (upddisplay > 10) {
    updateDisplay();
    upddisplay = 0;
  }
  else {
    // generateQRCode("wifi: ESP_NOW_CH_4 192.169.4.1"); // connect to local AP
    // upddisplay = 1000;
  }

  // heartbeat(runningtime, false); // not needed, heartbeat comes from ESP-NOW server
  
}

// ICACHE_RAM_ATTR void buttonPress(){
//   // Serial.print(F("Button press: "));
//   // Serial.println(millis());
//   handle_button(true, millis());
// }

// void DisplayProgress(const char* ptext) {
//   static bool toggle = false;
//   static bool toggle2 = false;
//   static bool toggle3 = false;
//   if (toggle) {
//     if (toggle2) {
//       generateQRCode(ptext);
//     }
//     else {
//       generateQRCode("Other text that can be reasonable long (enough) QR");
//     }
//     toggle2 = !toggle2;
//   }
//   else{
//     if (toggle3) {
//       textdisplay(ptext);
//     }
//     else {
//       textdisplay("Very long text that is readable but not very nice on the display to read");
//     }
//     toggle3 = !toggle3;
//   }
//   toggle = !toggle; // toggle
// }

// void generateQRCode(const char* text) {
//   // Create a QR code object
//   QRCode qrcode;
  
//   // Define the size of the QR code (1-40, higher means bigger size), this value is the QR code version
//   uint8_t qrcodeData[qrcode_getBufferSize(3)];
//   qrcode_initText(&qrcode, qrcodeData, 3, 0, text);

//   // Clear the display
//   display.clearDisplay();

//   Serial.print(F("QR info: "));
//   Serial.println(text);

//   // Calculate the scale factor
//   int scale = min(SCREEN_WIDTH / qrcode.size, SCREEN_HEIGHT / qrcode.size);
  
//   // Calculate horizontal shift
//   int shiftX = (SCREEN_WIDTH - qrcode.size*scale)/2;
  
//   // Calculate vertical shift
//   int shiftY = (SCREEN_HEIGHT - qrcode.size*scale)/2;

//   // Draw the QR code on the display
//   for (uint8_t y = 0; y < qrcode.size; y++) {
//     for (uint8_t x = 0; x < qrcode.size; x++) {
//       if (qrcode_getModule(&qrcode, x, y)) {
//         display.fillRect(shiftX + x*scale, shiftY + y*scale, scale, scale, WHITE);
//       }
//     }
//   }

//   // Update the display
//   display.display();
// }

// void textdisplay(const char* text) {
//   // Clear the display
//   display.clearDisplay();

//   Serial.print(F("textdisplay: "));
//   Serial.println(text);

//   //display.setCursor(0, LinesYPos[0]);
//   display.setCursor(0, 16);
//   display.print(text);

//   // Update the display
//   display.display();
// }
