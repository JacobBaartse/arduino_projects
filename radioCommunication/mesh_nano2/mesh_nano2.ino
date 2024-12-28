/*
 * RF-Nano with 1.3 inch Display (big display)
 */
#include "RF24Network.h"
#include "RF24.h"
#include "RF24Mesh.h"
#include <SPI.h>

#include <Wire.h>
//#include <Adafruit_GFX.h> // already included from font file
#include "FreeSerif12pt7b_special.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include <Adafruit_SH110X.h> // Adafruit SH110X by Adafruit

enum DisplayState {
    Off = 0,
    Dim = 1,
    On = 2,
};

// #########################################################

#define radioChannel 96
/** User Configuration per 'slave' node: nodeID **/
#define slaveNodeID 5
#define masterNodeID 0

/**** Configure the nrf24l01 CE and CSN pins ****/
// for the NANO with onboard RF24 module:
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external RF24 module:
// RF24 radio(8, 7); // nRF24L01 (CE, CSN)

RF24Network network(radio);
RF24Mesh mesh(radio, network);
 
unsigned long const keywordvalM = 0xfeebbeef; 
unsigned long const keywordvalS = 0xbeeffeeb; 

// Payload from/for MASTER
struct payload_from_master {
  unsigned long keyword;
  uint32_t counter;
};
 
// Payload from/for SLAVE
struct payload_from_slave {
  unsigned long keyword;
  uint32_t timing;
  uint8_t nodeId;
};
 
uint32_t sleepTimer = 0;
bool meshrunning = false;

void restart_arduino(){
  Serial.println(F("Restart the Arduino board..."));
  delay(2000);
  //NVIC_SystemReset(); // TBD
}

bool meshstartup(){
  if (meshrunning){
    Serial.println(F("Radio issue, turn op PA level?"));
  }
  return mesh.begin(radioChannel, RF24_250KBPS);
}

// #########################################################

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3C //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3D //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define OLED_RESET -1

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool displaystatus = DisplayState::Off;
void display_oled(bool clear, int x, int y, String text) {
  if (displaystatus == DisplayState::Off) return;
  if (clear) display.clearDisplay();
  display.setCursor(x, y);
  display.print(text);
  display.display();
}

// move text, write old location in background color
void display_move(int x, int y, int nx, int ny, String text) {
  if (displaystatus == DisplayState::Off) return;
  display.setCursor(x, y);
  display.setTextColor(SH110X_BLACK);
  display.print(text);
  display.setCursor(nx, ny);
  display.setTextColor(SH110X_WHITE);
  display.print(text);
  display.display();
}

DisplayState setDisplay(DisplayState statustoset){
  static DisplayState displaystatus = DisplayState::Dim;
  switch(statustoset){
    case DisplayState::Dim:
      display.oled_command(SH110X_DISPLAYON);
      display.setContrast(0); // dim display
      displaystatus = DisplayState::Dim;
      break;
    case DisplayState::On:
      display.oled_command(SH110X_DISPLAYON);
      displaystatus = DisplayState::On;
      break;
    //case DisplayState::Off:
    default:
      display.oled_command(SH110X_DISPLAYOFF);
      displaystatus = DisplayState::Off;
  }
  return displaystatus;
}

void clear_display(){
  display.clearDisplay();
  display.display();
}

// #########################################################

String Line1 = "Welcome \x81"; 
String Line2 = "Demo \x81{characters} \x81"; 
String Line3 = "Whats \x81 up?"; 

int prevx, x, minX;
int y1, y2, y3, minY;
bool oncecompleted = false;

void setup() {
  Serial.begin(115200);
  Serial.println(F(" ***************"));  

  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);

  // Set the nodeID manually
  mesh.setNodeID(slaveNodeID);
  // Serial.print(F("Setup node: "));
  // Serial.print(slaveNodeID);
  Serial.println(F(", connecting to the mesh..."));
  // Connect to the mesh
  meshrunning = meshstartup();

  // display.begin(i2c_Address, true); // Address 0x3C default
  // displaystatus = setDisplay(DisplayState::Dim);
  // display.clearDisplay();
  // display.setFont(&FreeSerif12pt7b);
  // display.setTextSize(1); // 3 lines of 10-12 chars
  // display.setTextColor(SH110X_WHITE);
  // display.setTextWrap(false);
  // display.display();

  // x = display.width();
  // y1 = 16;
  // y2 = 38;
  // y3 = 60;
  // // minX = -128;
  // minX = -200; // depends on length of the text
  // minY = -22;

  // display_oled(true, 0, y1, Line1); 
  // display_oled(false, 2, y2, Line2); 
  // display_oled(false, 4, y3, Line3);  
  // prevx = x;

}
 
unsigned int mesherror = 0;
uint8_t meshupdaterc = 0;
uint8_t rem_meshupdaterc = 200;

void loop() {
  if (mesherror > 8) {
    meshrunning = meshstartup();
    mesherror = 0;
  }
  // Call mesh.update to keep the network updated
  meshupdaterc = mesh.update();
  if (meshupdaterc != rem_meshupdaterc) {
    Serial.print(F("meshupdaterc: "));
    Serial.println(meshupdaterc);
    rem_meshupdaterc = meshupdaterc;
  } 

  // //// Receive a message from master if available - START
  // while (network.available()) {
  //   RF24NetworkHeader header;
  //   payload_from_master payload;
  //   network.read(header, &payload, sizeof(payload));
  //   Serial.print(F("Received packet #"));
  //   Serial.print(payload.counter);
  //   if (payload.keyword == keywordvalM) {

  //   }
  //   else{
  //     Serial.println(F("Wrong keyword")); 
  //   }
  // }
  // //// Receive a message from master if available - END

  // //// Send to the master node every x seconds - BEGIN
  // if (millis() - sleepTimer > 10000) {
  //   sleepTimer = millis();
  //   payload_from_slave payloadM = {keywordvalS, sleepTimer, slaveNodeID};
 
  //   // Send an 'M' type message containing the current millis()
  //   if (!mesh.write(&payloadM, 'M', sizeof(payloadM))) {
  //     // If a write fails, check connectivity to the mesh network
  //     if (!mesh.checkConnection()) {
  //       //refresh the network address
  //       Serial.println(F("Renewing Address"));
  //       if (mesh.renewAddress() == MESH_DEFAULT_ADDRESS) {
  //         // If address renewal fails, reconfigure the radio and restart the mesh
  //         // This allows recovery from most, if not all radio errors
  //         meshstartup();
  //       }
  //     }
  //     else {
  //       //Serial.println(F("Send fail, Test OK"));
  //       mesherror++;
  //     }
  //   } else {
  //     // Serial.print(F("Send to Master OK: "));
  //     // Serial.println(payloadM.timing);
  //     mesherror = 0;
  //   }
  // }
  // //// Send to the master node every x seconds - END

  // float tempval = 24.567;
  // display_oled(true, 0, 16, String(tempval, 1) + " \x7F"+"C");  // } \x7F is converted to degrees in this special font.
  // delay(2000);

}
