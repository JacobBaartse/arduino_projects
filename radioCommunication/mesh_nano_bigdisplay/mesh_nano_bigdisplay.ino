/*
 * RF-Nano with 1.3 inch Display (big display)
 */

#include "RF24.h"
#include <SPI.h>

#include <Wire.h>
//#include <Adafruit_GFX.h> // already included from font file
//#include "FreeSerif12pt7b_special.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include <Adafruit_SH110X.h> // Adafruit SH110X by Adafruit

enum DisplayState {
    Off = 0,
    Dim = 1,
    On = 2,
};

// #########################################################

#define radioChannel 96
/** User Configuration per 'slave' node: nodeID **/
// #define slaveNodeID 5
// #define masterNodeID 0

/**** Configure the nrf24l01 CE and CSN pins ****/
// for the NANO with onboard RF24 module:
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external RF24 module:
// RF24 radio(8, 7); // nRF24L01 (CE, CSN)

const byte slaveAddress[5] = {'R','x','A','B','C'};
 
unsigned long const keywordvalM = 0xfeebbeef; 
unsigned long const keywordvalS = 0xbeeffeeb; 

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
 
uint32_t sleepTimer = 0;

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

String Line1 = "Welcome a"; 
String Line2 = "Demo B"; 
String Line3 = "Whats c up?"; 

int y1, y2, y3;

char dataReceived[10]; // this must match dataToSend in the TX
char rxNum = '0';
char ackData[6] = "Ack 0";

void setup() {
  //Serial.begin(115200);
  //Serial.println(F(" ***************"));  

  //SPI.begin();
  if (!radio.begin()){
    //Serial.println(F("Radio hardware error."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  //Serial.println("SimpleRxAckPayload Starting");
  radio.setDataRate( RF24_250KBPS );
  radio.openReadingPipe(1, slaveAddress);
  radio.enableAckPayload();
  radio.startListening();
  radio.writeAckPayload(1, &ackData, sizeof(ackData)); 

  //Wire.begin();
  display.begin(i2c_Address, true); // Address 0x3C default
  display.oled_command(SH110X_DISPLAYON);
  display.setContrast(0); // dim display
  displaystatus = DisplayState::Dim;
  display.clearDisplay();
  //display.setFont(&FreeSerif12pt7b);
  display.setTextSize(2); // 3 lines of 10-12 chars
  display.setTextColor(SH110X_WHITE);
  display.setTextWrap(false);
  display.display();

  y1 = 0;//16;
  y2 = 21;//38;
  y3 = 42;//60;

  display_oled(true, 0, y1, Line1); 
  display_oled(false, 2, y2, Line2); 
  display_oled(false, 4, y3, Line3);  

}
 
bool new_data = false;
bool no_data = false;
unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;

void loop() {

  currentmilli = millis();

  if (radio.available()){
    new_data = true;
    no_data = false;
    radio.read(&dataReceived, sizeof(dataReceived));
    rxNum += 1;
    if (rxNum > '9') {
      rxNum = '0';
    }
    ackData[4] = rxNum;
    radio.writeAckPayload(1, &ackData, sizeof(ackData)); // load the payload for the next time
    receiveTimer = currentmilli;
  }
  if(currentmilli - receiveTimer > 20000){
    //printing should be a once of action
    if (!no_data){
      no_data = true; // for printing/clearing previous data
      new_data = true; // print stuff
    }
  }

  if (new_data){
    new_data = false;
    //Serial.print(dataReceived);
    //Serial.print(F(" "));
    if (no_data){
      display_oled(true, 0, y1, "----");
    }
    else{
      display_oled(true, 0, y1, dataReceived);
    }
    //Serial.print(ackData);
    //Serial.print(F(" "));
    float tempval = (float)currentmilli / (float)1000;
    //Serial.println(tempval);
    display_oled(false, 4, y3, String(tempval, 1) + " s");
    // delay(1250);
  }

}
