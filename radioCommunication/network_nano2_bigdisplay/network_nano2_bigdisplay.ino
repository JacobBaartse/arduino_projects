/*
 * RF-Nano with 1.3 inch Display (big display)
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#include <Wire.h>
#include <Adafruit_GFX.h> // already included from font file
//#include "FreeSerif12pt7b_special.h" // https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include <Adafruit_SH110X.h> // Adafruit SH110X by Adafruit

enum DisplayState {
    Off = 0,
    Dim = 1,
    On = 2,
};

// #########################################################

#define radioChannel 100
/** User Configuration per 'slave' node: nodeID **/
// #define slaveNodeID 5
// #define masterNodeID 0

const uint16_t node01 = 02;   // Address of this node in Octal format (04, 031, etc.)
const uint16_t node00 = 00;   // Address of the other node in Octal format

/**** Configure the nrf24l01 CE and CSN pins ****/
// for the NANO with onboard RF24 module:
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external RF24 module:
// RF24 radio(8, 7); // nRF24L01 (CE, CSN)
RF24Network network(radio);                                                       // Include the radio in the network

//const byte addresses [][6] = {"00001", "00002"};  //Setting the two addresses. One for transmitting and one for receiving

unsigned long const keywordvalM = 0xfeebbeef; 
unsigned long const keywordvalS = 0xbeeffeeb; 

// // Payload from/for MASTER
// struct payload_from_master {
//   unsigned long keyword;
//   uint32_t counter;
//   uint32_t dummy1 = 0;
//   uint32_t dummy2 = 0;
// };
 
// // Payload from/for SLAVE
// struct payload_from_slave {
//   unsigned long keyword;
//   uint32_t timing;
//   uint8_t nodeId;
// };
 
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

String Line1 = "Welcome George!"; 
String Line2 = "Demo B"; 
String Line3 = "Whats c up?"; 

int y1, y2, y3;

char dataToSend[10] = "Sender1 0";
char rxNum = '0';

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.flush(); 
  Serial.println(__TIMESTAMP__);
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 
  
  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  //Serial.println("SimpleRxAckPayload Starting");
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, node01);

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

void updateMessage() {
  // so you can see that new data is being sent
  rxNum += 1;
  if (rxNum > '9') {
    rxNum = '0';
  }
  dataToSend[8] = rxNum;
}

// typedef struct
// {
//   unsigned long timestamp;
//   char dataText[10];
// }
// sendingdata;
// sendingdata Txdata;
// sendingdata Rxdata;

typedef struct {
  unsigned long keyword;
  unsigned long timing;
  char dataText1[10];
  char dataText2[10];
} network_payload;
network_payload Txdata;
network_payload Rxdata;

//===== Receiving =====//
void receiveRFnetwork(){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node != node00) {
      // Serial.print(F("received unexpected message, from_node: "));
      // Serial.println(header.from_node);
      break;
    }

    if (Rxdata.keyword == keywordvalM){

    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
}

//===== Sending =====//
void transmitRFnetwork(){
  static unsigned long sendingTimer = 0;

  // Every x seconds...
  unsigned long currentmilli = millis();
  if(currentmilli - sendingTimer > 15000){
    sendingTimer = currentmilli;
    Txdata.keyword = keywordvalM;
    Txdata.timing = currentmilli;
    for(int i=0; i<10; i++)
    {
      Txdata.dataText1[i] = dataToSend[i];
      Txdata.dataText2[i] = dataToSend[i];
    }
    RF24NetworkHeader header0(node00); // (Address where the data is going)
    network.write(header0, &Txdata, sizeof(Txdata)); // Send the data

    updateMessage();
  }
}

void loop() {

  currentmilli = millis();

  network.update();

  receiveRFnetwork();

  transmitRFnetwork();

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
      display_oled(true, 0, y1, Rxdata.dataText1);
    }
    float timeval = (float)Rxdata.timing / (float)1000;
    display_oled(false, 4, y2, String(timeval, 1) + " s");

    //Serial.print(ackData);
    //Serial.print(F(" "));
    float tempval = (float)currentmilli / (float)1000;
    //Serial.println(tempval);
    display_oled(false, 4, y3, String(tempval, 1) + " s");
    // delay(1250);
  }

}
