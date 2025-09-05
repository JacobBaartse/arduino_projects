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

const uint16_t repeaternode = 01;

// RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 
uint8_t radiolevel = RF24_PA_MIN;

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)

RF24Network network(radio);
 
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
    //detectornode = 2;
  }
  if (digitalRead(CFG_PIN1) == LOW){ // PIN active
    //detectornode = detectornode + 2;
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
unsigned long netupdate = 0;

void loop() {

  runtiming = millis();

  if ((unsigned long)(runtiming - netupdate) > 100){ // regularly update networking
    network.update();
    netupdate = runtiming;
  }

}
