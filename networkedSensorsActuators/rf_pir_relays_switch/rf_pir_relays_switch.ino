/*
 * Nano with external RF module, PIR sensor
 */

#include "networking.h"
// #include <SPI.h>

// #########################################################

#define pinPIR 21 //A7 // PIR pin connection
#define pinPressButton 8  

void switchlight(bool lightON, bool lightOFF){
  if(lightON && lightOFF){
    Serial.println(F("ERROR: Light ON OFF"));  
  }
  if(lightON){
    Serial.println(F("Light ON"));  
  }
  if(lightOFF){
    Serial.println(F("Light OFF"));  
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F(" *************>>"));  

  pinMode(pinPIR, INPUT);
  pinMode(pinPressButton, INPUT_PULLUP);

  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network.begin(radioChannel, this_node); // (channel, node address)
  radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);

  Serial.print(F("radio node: "));  
  Serial.println(this_node);  
  Serial.println(F(" *************"));  
  Serial.flush();  
}
 
bool remdetectionval = false;
bool sendDirect = false;
unsigned int readaction = 0;
unsigned int writeaction = 0;
bool lightON = false;
bool lightOFF = false;

void loop() {

  network.update();

  if(sendDirect){
    Serial.print(F(" send direct about to happen")); 
  }
  else {
    // Receive a message from base if available
    readaction = receiveRFnetwork();
    if(readaction > 0){
      if(rdata1 == 0x5aa55aa5){
        lightON = true;
        lightOFF = false;
      }
      if(rdata1 == 0x12345678){
        lightON = false;
        lightOFF = true;
      }
    }
  }

  // Send to the base node every x seconds or immediate
  writeaction = transmitRFnetwork(sendDirect);

  if (digitalRead(pinPIR) == HIGH){
    lightON = true;
    if (!remdetectionval){
      sendDirect = true;
      Serial.print(F("PIR detection"));
      Serial.println(millis());
      remdetectionval = true;
    }
    tdata1 = 0xff;
  }

  if (digitalRead(pinPressButton) == LOW){ // no debounce necessary here
    lightON = false;
    lightOFF = true;
  }

  switchlight(lightON, lightOFF);

}
