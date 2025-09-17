/*
 * part of the tool/demo to measure the RF distance with the standardized RF24 hardware, build into the RF-NANO
 */

#include "RF24Network.h"
#include "RF24.h"
#include <SPI.h>
 
#define radioChannel 96
#define CE_PIN 10
#define CSN_PIN 9

// #define CFG_PIN0 A0
// #define CFG_PIN1 A1
// #define CFG_PIN2 A2
// #define CFG_PIN3 A4

const uint16_t endpointnode = 00;
const uint16_t repeaternode = 01;

// RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 
uint8_t radiolevel = RF24_PA_MAX;

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
uint8_t receivedbvalue = 0;
uint8_t receivedsvalue = 0;
uint8_t receivedrvalue = 0;
uint16_t rtotal = 0;
uint16_t mtotal = 0;
uint16_t mfail = 0;
uint16_t mpass = 0;

//===== Receiving =====//
bool receiveRFnetwork(unsigned long currentRFmilli){
  bool mreceived = false;

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
    //if (true){
      mreceived = true;
      rtotal++;
      Serial.print(F("Data received from endpoint "));
      Serial.println(currentRFmilli);
      // in case a message is received, some action can be taken
      receivedbvalue = Rxdata.bvalue;
      receivedsvalue = Rxdata.svalue;
      receivedrvalue = Rxdata.rvalue;
    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
  return mreceived;
}

//===== Sending =====//
bool transmitRFnetwork(bool fresh, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every second, or on new data
  //if ((fresh)||(failcount > 0)||((unsigned long)(currentRFmilli - sendingTimer) > 1000)){
  //if ((fresh)||(failcount > 0)){
  if (fresh){
    sendingTimer = currentRFmilli;

    dist_payload Txdata;
    Txdata.keyword = keywordvalD;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    Txdata.bvalue = receivedbvalue;
    Txdata.svalue = receivedsvalue;
    Txdata.rvalue = receivedrvalue;

    RF24NetworkHeader header0(endpointnode, 'D'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    // if (!w_ok){ // retry
    //   delay(50);
    //   w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    // }
    Serial.print(F("Message send ")); 
    mtotal++; 
    if (w_ok){
      fresh = false;
      failcount = 0;
      mpass++; 
    }    
    else{
      Serial.print(F("failed "));
      failcount++;
      mfail++;
    }
    Serial.print(Txdata.count);
    Serial.print(F(", "));
    Serial.println(currentRFmilli);
    
    receivedbvalue = 0;
    receivedsvalue = 0;
    receivedrvalue = 0;

    if (failcount > 4){
      failcount = 0; // do not send/retry a lot of messages continously
    }
  }
  return fresh;
}

void showstatistics(unsigned long curmilli){
  static unsigned long statTimer = 0;

  if ((unsigned long)(curmilli - statTimer) > 100000){
    statTimer = curmilli;
    Serial.print(F("Messages statistics. mtotal: "));
    Serial.print(mtotal);
    Serial.print(F(", fail: "));
    Serial.print(mfail);
    Serial.print(F(", pass: "));
    Serial.print(mpass);
    Serial.print(F(", received: "));
    Serial.print(rtotal);
    Serial.println(F(" !"));
  }
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

  // // multiple PINs for reading the config
  // pinMode(CFG_PIN0, INPUT_PULLUP);
  // pinMode(CFG_PIN1, INPUT_PULLUP);
  // pinMode(CFG_PIN2, INPUT_PULLUP);
  // pinMode(CFG_PIN3, INPUT_PULLUP);

  // if (digitalRead(CFG_PIN0) == LOW){ // PIN active

  // }
  // if (digitalRead(CFG_PIN1) == LOW){ // PIN active

  // }

  // // RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 
  // if (digitalRead(CFG_PIN2) == LOW){ // PIN active
  //   radiolevel = 1;
  // }
  // if (digitalRead(CFG_PIN3) == LOW){ // PIN active
  //   radiolevel = radiolevel + 2;
  // }

  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  radio.setPALevel(radiolevel, 0);
  radio.setDataRate(RF24_2MBPS); // RF24_1MBPS, RF24_2MBPS, RF24_250KBPS
  network.begin(radioChannel, repeaternode);

  Serial.println();  
  Serial.println(F(" ***************"));  
  Serial.println(); 
  Serial.flush(); 
}
 
unsigned long runtiming = 0;
bool fresh = false;

void loop() {

  network.update();

  runtiming = millis();

  fresh = receiveRFnetwork(runtiming);

  transmitRFnetwork(fresh, runtiming);

  showstatistics(runtiming);

}
