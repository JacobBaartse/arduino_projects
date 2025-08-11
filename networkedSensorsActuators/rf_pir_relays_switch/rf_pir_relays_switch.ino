/*
 * Nano with external RF module, PIR sensor, switch bistable relays
 */

#include "networking.h"
#include <SPI.h>

// #########################################################

#define pinPIR1 6        // PIR pin connection
#define pinPIR2 A3       // PIR pin connection
#define pinLight 3       // light driver pin
#define pinStatus 4      // check relays position

#define pinPressButton 5 // light off button
//#define pinDetection 7   // light off button

unsigned long runningtime = 0;
unsigned long lighttime = 0;

void changelight(){
    digitalWrite(pinLight, HIGH);
    delay(500);
    digitalWrite(pinLight, LOW);
}

int statuslight(){
    return digitalRead(pinStatus);
}

void setLight(bool LightOn){
  int cursttat = statuslight();
  if (LightOn){
    digitalWrite(pinLight, HIGH);
    // if (cursttat == HIGH){
    //   changelight();
    //   delay(1000);
    // }
  }
  else {
    digitalWrite(pinLight, LOW);
    // if (cursttat == HIGH){
    //   changelight();
    //   delay(1000);
    // }
  }
  //return statuslight();
}

void switchlight(bool plightON, bool plightOFF){
  static int lightstatus = 0;
  if(plightON && plightOFF){
    if (lightstatus != 1) {
      Serial.println(F("ERROR: Light ON OFF"));  
    }
    lightstatus = 1;
  }
  if(plightON){
    if (lightstatus != 2) {
      Serial.print(F("Light ON - ")); 
      setLight(true);
      lighttime = millis();
      Serial.println(lighttime); 
    } 
    lightstatus = 2;
  }
  if(plightOFF){
    if (lightstatus != 3) {
      Serial.print(F("Light OFF - ")); 
      setLight(false);
      Serial.println(millis()); 
    }
    lightstatus = 3;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F(" ***** <> *****"));  
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 
  Serial.println(F(" *************>>"));  

  pinMode(pinPIR1, INPUT);
  pinMode(pinPIR2, INPUT);
  pinMode(pinLight, OUTPUT);
  pinMode(pinPressButton, INPUT_PULLUP);

  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network.begin(radioChannel, this_node); // (channel, node address)
  radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);

  Serial.print(F("radio node: "));  
  Serial.println(this_node);  
  Serial.println(F(" *************"));  
  Serial.flush(); 
  //if(!radio.testRPD()) // detect carrier?
  //radio.printDetails(); 
}
 
bool sendDirect = false;
unsigned int readaction = 0;
unsigned int writeaction = 0;
bool lightON = false;
bool lightOFF = false;
int pirstatus1 = LOW;
int rempirstatus1 = 2;
int pirstatus2 = LOW;
int rempirstatus2 = 2;


void loop() {

  network.update();

  runningtime = millis();
  if (lighttime > 0){
    if (runningtime - lighttime > 20000){ // 20 seconds
      lightON = false;
      lightOFF = true;
    }
  }

  if(sendDirect){
    Serial.println(F(" send direct about to happen")); 
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

  // // Send to the base node every x seconds or immediate
  // writeaction = transmitRFnetwork(sendDirect);
  sendDirect = false;

  // if (writeaction > 10)
  // {
  //   sendDirect = false;
  //   delay(5000);
  // }

  pirstatus1 = digitalRead(pinPIR1);
  pirstatus2 = digitalRead(pinPIR2);
  if ((pirstatus1 != rempirstatus1)||(pirstatus2 != rempirstatus2)){
    rempirstatus1 = pirstatus1;
    rempirstatus2 = pirstatus2;

    Serial.print(F("PIR 1 detection "));
    Serial.print(pirstatus1);
    Serial.print(F(", PIR 2 detection "));
    Serial.print(pirstatus2);
    Serial.print(F(" - time: "));
    Serial.println(runningtime);
    if ((pirstatus1 == HIGH)&&(pirstatus2 == HIGH)){
      lightON = true;
      lightOFF = false;
      tdata1 = 0xff;
      sendDirect = true;
    }
  }

  if (digitalRead(pinPressButton) == LOW){ // no debounce necessary here
    lightON = false;
    lightOFF = true;
  }

  switchlight(lightON, lightOFF);

}
