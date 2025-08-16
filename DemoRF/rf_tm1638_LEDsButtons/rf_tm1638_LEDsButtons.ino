/*
 * RF-Nano, TM1638 for UI

   https://www.makerguides.com/connecting-arduino-uno-with-tm1638-7-segment-led-driver/

 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#include <TM1638.h>

//choose digital pins compatibles with your board
#define STB 5 // Strobe digital pin
#define CLK 6 // clock digital pin
#define DIO 7 // data digital pin

#define radioChannel 102

TM1638 tm(CLK, DIO, STB);

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node00 = 00; // Address of the home/host/controller node in Octal format
const uint16_t tmnode = 04; // Address of the home/host/controller node in Octal format

unsigned long const keywordvalT = 0x12345678; 

struct tm_payload{
  uint32_t keyword;
  uint32_t timing;
  uint32_t counter;
  uint8_t buttons;
  bool SW[8];
};

struct tm_ack_payload{
  uint32_t keyword;
  uint32_t timing;
  uint32_t counter;
  uint8_t leds;
  uint8_t TXT[8];
};

bool button_list[8];
const uint8_t text[] = {0xff, 0x7c, 0x1c, 0x78, 0x78, 0x5c, 0x54, 0xff};

void setup() {
  Serial.begin(115200);
  for (uint8_t i=0;i<8;i++) // define no switches
    button_list[i] = false;

  tm.reset();
  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, tmnode);

  tm.test();
  for (uint8_t i=0;i<sizeof(text);i++) {
    tm.displayDig(7-i, text[i]);
  }
  delay(2000);
  tm.reset();
  tm.displaySetBrightness(1);
}

uint8_t buttonsvalue = 0;

//===== Receiving =====//
bool receiveRFnetwork(unsigned long currentRFmilli){
  bool mreceived = false;

  // Check for incoming data details
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      case 'T': 
        tm_ack_payload rf_payload;
        network.read(header, &rf_payload, sizeof(rf_payload));
        Serial.print(F("Received from base nodeId: "));
        Serial.print(header.from_node);
        Serial.print(F(", timing: "));
        Serial.println(rf_payload.timing);
        if (rf_payload.keyword == keywordvalT) {
          // message/response/ack received from base
          tm.writeLeds(rf_payload.leds);
          Serial.println(rf_payload.leds);
          for (uint8_t i=0;i<8;i++) {
            if (rf_payload.TXT[i] > 0){
              tm.displayDig(7-i, rf_payload.TXT[i]);
              Serial.println(rf_payload.TXT[i]);
            }
          }
          delay(500);
          // end of ack message collection      
          mreceived = true;
        }
        else{
          Serial.println(F("Wrong T keyword")); 
        }
        break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.println(header.type);
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

  // Every 60 seconds, or on new data
  //if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 60000)){
  if ((fresh)&&(buttonsvalue > 0)){ // only send buttons pressed (not when released)
    // if ((unsigned long)(currentRFmilli - sendingTimer) > 5000){

    sendingTimer = currentRFmilli;

    tm_payload Txdata;
    Txdata.keyword = keywordvalT;
    Txdata.timing = currentRFmilli;
    Txdata.counter = counter++;
    Txdata.buttons = buttonsvalue;
    for (uint8_t i=0;i<8;i++) // capture the switches
      Txdata.SW[i] = button_list[i];

    RF24NetworkHeader header0(node00, 'T'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      failcount++;
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    Serial.print(F("Message send ")); 
    if (w_ok){
      failcount = 0;
      fresh = false;
    }    
    else{
      Serial.print(F("failed "));
      failcount++;
    }
    Serial.print(Txdata.counter);
    Serial.print(F(", "));
    Serial.println(currentRFmilli);

  }
  return fresh;
}

void buttonnumber(uint8_t pbuttonint){
  static uint8_t buttonint = 255;

  if (pbuttonint != buttonint){
    buttonint = pbuttonint;
    uint8_t bitcheck = 1;
    for (uint8_t i=0;i<8;i++){ // i+1 is the SW number
      uint8_t pos = 7 - i;
      if ((bitcheck & buttonint) > 0){
        //tm.displayVal(pos, i+1);
        Serial.print(F("Button SW"));
        Serial.println(i+1);
        button_list[i] = true;
      }
      else {
        button_list[i] = false;
        //tm.displayDig(pos, 0);
      }
      //bitcheck *= 2; // shift the bit left
      bitcheck <<= 1; // shift the bit left
    }
  }
}

uint8_t pulsefrombuttons = 7;

bool handlebuttons(uint8_t ppulse){
  static uint8_t buttons = 0;
  static uint8_t dotpulse = 7;
  bool change = false;
  
  buttonsvalue = tm.getButtons();
  if (buttonsvalue > 0){
    //Serial.println(buttonsvalue);
  
    //change = (buttonsvalue & buttons) == 0; 
    change = buttonsvalue != buttons;
    if (change){
      dotpulse = 7;
      //buttons = buttons | buttonsvalue;
      buttons = buttonsvalue;
      //tm.reset();
      //tm.displaySetBrightness(dotpulse);
      buttonnumber(buttons);
      //tm.writeLeds(buttons);
    }
  }
  else{
    if (ppulse != dotpulse){
      if (ppulse > dotpulse)
        dotpulse = ppulse;
      else
        if (dotpulse > 0)
          dotpulse -= 1;
      //tm.displaySetBrightness(dotpulse);
      buttonnumber(buttons);
      //tm.writeLeds(buttons);
    }
  }

  pulsefrombuttons = dotpulse;
  return change;
}

bool handledots(uint8_t pdotpulse, unsigned long currentdotmilli){
  static unsigned long dottime = 0;
  static uint8_t dotpulse = 4;

  if (pdotpulse > dotpulse){
      dottime = currentdotmilli;
      dotpulse = pdotpulse;
  }
  else
    if ((unsigned long)(currentdotmilli - dottime) > 2000){
      dottime = currentdotmilli;

      if (dotpulse > 0){
        dotpulse -= 1;
        Serial.print(F("dotpulse "));
        Serial.println(dotpulse); 
      }
      // pgfedcba // what is the DP?
    }
  return dotpulse;
}

pulse_t pulse = PULSE1_16;
unsigned long timer = 0;

unsigned long currentmilli = 0;
bool fresh = false;
uint8_t pulsefromdots = 7;

void loop() {

  network.update();

  currentmilli = millis();

  receiveRFnetwork(currentmilli);

  fresh = handlebuttons(pulsefromdots);

  pulsefromdots = handledots(pulsefrombuttons, currentmilli);

  // if (millis() - timer > 1000){
  //   timer = millis();
  //   Serial.print(F("Pulse: "));
  //   Serial.println(pulse);
  //   tm.displaySetBrightness(pulse);
  //   pulse = (pulse==PULSE1_16) ? PULSE14_16 : PULSE1_16;
  // }
  
  fresh = transmitRFnetwork(fresh, currentmilli);

}
