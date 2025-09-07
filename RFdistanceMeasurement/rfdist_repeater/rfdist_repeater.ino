/*
 * part of the tool/demo to measure the RF distance with the standardized RF24 hardware, build into the RF-NANO
 */

#include "RF24Network.h"
#include "RF24.h"
#include <SPI.h>
 
#define radioChannel 96
#define CE_PIN 10
#define CSN_PIN 9

#define CFG_PIN0 A0
#define CFG_PIN1 A1
#define CFG_PIN2 A2
#define CFG_PIN3 A4

const uint16_t endpointnode = 00;
const uint16_t repeaternode = 01;

// RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 
uint8_t radiolevel = RF24_PA_MIN;

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)

RF24Network network(radio);
 

struct dist_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t bvalue;
  uint8_t svalue;
  uint8_t rvalue;
};

unsigned long const keywordvalD = 0x12348765; 

//===== Receiving =====//
bool receiveRFnetwork(unsigned long currentRFmilli){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    dist_payload Rxdata;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if ((header.from_node != endpointnode)||(header.type != 'D')) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.print(header.from_node);
      Serial.print(F(", type: "));
      Serial.println(header.type);
      break;
    }
    if (Rxdata.keyword == keywordvalD){
      Serial.print(F("Data received from base "));
      Serial.println(currentRFmilli);
      // in case a message is received, some action can be taken
      // if (Rxdata.bvalue == 0x0a){
      //   digitalWrite(LED_PIN, HIGH);
      // }
      // else {
      //   digitalWrite(LED_PIN, LOW);
      // }
      //Rxdata.rvalue = 0x05;
      //Rxdata.svalue = 0x05;

    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
}

//===== Sending =====//
//bool transmitRFnetwork(bool pfresh, unsigned long currentRFmilli, bool pping){
bool transmitRFnetwork(bool pfresh, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;
  bool fresh = pfresh;

  // Every second, or on new data
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 1000)){
    sendingTimer = currentRFmilli;

    dist_payload Txdata;
    Txdata.keyword = keywordvalD;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;

    RF24NetworkHeader header0(endpointnode, 'D'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    Serial.print(F("Message send ")); 
    if (w_ok){
      fresh = false;
      failcount = 0;
    }    
    else{
      Serial.print(F("failed "));
      failcount++;
    }
    Serial.print(Txdata.count);
    Serial.print(F(", "));
    Serial.println(currentRFmilli);

    if (failcount > 4){
      fresh = false; // do not send a lot of messages continously
    }
  }
  // if (pping){
  //   fresh = false; // 
  // }
  return fresh;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
    delay(10);
  }
  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  // multiple PINs for reading the config
  pinMode(CFG_PIN0, INPUT_PULLUP);
  pinMode(CFG_PIN1, INPUT_PULLUP);
  pinMode(CFG_PIN2, INPUT_PULLUP);
  pinMode(CFG_PIN3, INPUT_PULLUP);

  if (digitalRead(CFG_PIN0) == LOW){ // PIN active

  }
  if (digitalRead(CFG_PIN1) == LOW){ // PIN active

  }

  // RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 
  if (digitalRead(CFG_PIN2) == LOW){ // PIN active
    radiolevel = 1;
  }
  if (digitalRead(CFG_PIN3) == LOW){ // PIN active
    radiolevel = radiolevel + 2;
  }

  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  radio.setPALevel(radiolevel, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, repeaternode);

  Serial.println();  
  Serial.println(F(" ***************"));  
  Serial.println(); 
  Serial.flush(); 
}
 
unsigned long runtiming = 0;
//unsigned long netupdate = 0;
bool fresh = false;

void loop() {

  network.update();

  runtiming = millis();

  // if ((unsigned long)(runtiming - netupdate) > 100){ // regularly update networking
  //   network.update();
  //   netupdate = runtiming;
  // }

  fresh = receiveRFnetwork(runtiming);

  transmitRFnetwork(fresh, runtiming);

}
