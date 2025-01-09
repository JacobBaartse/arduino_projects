/*
 * RF-Nano with 1.3 inch Display (big display)
 */

#include "RF24Network.h"
#include "RF24.h"

#include "FreeSerif12pt7b_special.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include <Adafruit_SH110X.h> // Adafruit SH110X by Adafruit

enum DisplayState {
    Off = 0,
    Dim = 1,
    On = 2,
};

// #########################################################

#define radioChannel 100
/** User Configuration per 'slave' node: nodeID **/
#define slaveNodeID 11
#define masterNodeID 0

/**** Configure the nrf24l01 CE and CSN pins ****/
// for the NANO with onboard RF24 module:
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external RF24 module:
// RF24 radio(8, 7); // nRF24L01 (CE, CSN)

//RF24Network network(radio);

// Payload from/for MASTER
struct payload_from_master {
  unsigned long keyword;
  uint32_t counter;
  uint32_t dummy1 = 0;
  uint32_t dummy2 = 0;
};
 
// Payload from/for SLAVE
struct payload_from_slave {
  unsigned long keyword;
  uint32_t timing;
  uint8_t nodeId;
};
 
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

  // if (!radio.begin()){
  //   Serial.println(F("Radio hardware error."));
  //   while (1) {
  //     // hold in an infinite loop
  //   }
  // }
  // radio.setPALevel(RF24_PA_MIN, 0);
  //network.begin(radioChannel, slaveNodeID);  //(channel, node address)

  display.begin(i2c_Address, true); // Address 0x3C default
  display.oled_command(SH110X_DISPLAYON);
  display.setContrast(0); // dim display
  displaystatus = DisplayState::Dim;
  display.clearDisplay();
  display.setFont(&FreeSerif12pt7b);
  display.setTextSize(1); // 3 lines of 10-12 chars
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);
  display.display();

  x = display.width();
  y1 = 16;
  y2 = 38;
  y3 = 60;
  // minX = -128;
  minX = -200; // depends on length of the text
  minY = -22;

  display_oled(true, 0, y1, Line1); 
  display_oled(false, 2, y2, Line2); 
  display_oled(false, 4, y3, Line3);  
  prevx = x;

  delay(1500);

}
 
float increment = 0.001;
float tempval = 34.567;
unsigned long loopc = 0;
char words[] = "asdf";

// https://howtomechatronics.com/tutorials/arduino/how-to-build-an-arduino-wireless-network-with-multiple-nrf24l01-modules/?utm_content=cmp-true

  // RF24NetworkHeader header(node01);     // (Address where the data is going)
  // bool ok = network.write(header, &angleValue, sizeof(angleValue)); // Send the data

// {
//   while (network.available()) {     // Is there any incoming data?
//     RF24NetworkHeader header;
//     unsigned long incomingData;
//     network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
//   }
// }

void loop() {

  //network.update();

  display_oled(true, 0, 16, String(tempval, 3) + " \x7F"+"C");  // } \x7F is converted to degrees in this special font.
  if ((loopc % 10) == 0){
    display_oled(false, 40, y2, String(words)); 
  }
  else {
    display_oled(false, 40, y2, String(loopc));  
  }
  display_oled(false, 10, y3, String(millis()));  
  delay(2000);

  tempval += increment;
  loopc++;

}
