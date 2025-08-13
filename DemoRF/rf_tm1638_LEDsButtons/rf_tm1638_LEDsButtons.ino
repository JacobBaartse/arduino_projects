/*
 * RF-Nano, base node for demo purposes

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
};

void setup() {
  Serial.begin(115200);

  tm.reset();
  tm.test();
  
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
      // Display the incoming millis() values from sensor nodes
      // case 'J': 
      //   joystick_payload payload;
      //   network.read(header, &payload, sizeof(payload));
      //   Serial.print(F("Received from Joystick nodeId: "));
      //   joynode = header.from_node;
      //   Serial.print(header.from_node);
      //   Serial.print(F(", timing: "));
      //   Serial.println(payload.timing);
      //   if (payload.keyword == keywordvalJ) {
      //     // message received from joystick 
      //     mcount = payload.count;

      //     xmvalue = payload.xmvalue;
      //     ymvalue = payload.ymvalue;
      //     xpvalue = payload.xpvalue;
      //     ypvalue = payload.ypvalue;
      //     jbvalue = payload.bvalue;
      //     sw1value = payload.sw1value;
      //     sw2value = payload.sw2value;
      //     Serial.print(F("Button "));
      //     bool printbutton = false;
      //     if (jbvalue > 0){
      //       Serial.print(F("J, "));
      //       printbutton = true;
      //     }
      //     if (sw1value > 0){
      //       Serial.print(F("1, "));
      //       printbutton = true;
      //     }
      //     if (sw2value > 0){
      //       Serial.print(F("2, "));
      //       printbutton = true;
      //     }
      //     if (!printbutton){
      //       Serial.print(F("-, "));
      //     }
      //     Serial.print(F("xmvalue: "));
      //     Serial.print(xmvalue);
      //     Serial.print(F(", xpvalue: "));
      //     Serial.print(xpvalue);
      //     Serial.print(F(", ymvalue: "));
      //     Serial.print(ymvalue);
      //     Serial.print(F(", ypvalue: "));
      //     Serial.println(ypvalue);

      //     // end of joystick message collection      
      //     mreceived = true;
      //   }
      //   else{
      //     Serial.println(F("Wrong Joystick keyword")); 
      //   }
      //   break;
      // // Display the incoming millis() values from sensor nodes
      // case 'K': 
      //   bool keyfollowup = false;
      //   keypad_payload kpayload;
      //   network.read(header, &kpayload, sizeof(kpayload));
      //   Serial.print(F("Received from Keypad nodeId: "));
      //   keynode = header.from_node;
      //   Serial.print(header.from_node);
      //   Serial.print(F(", timing: "));
      //   Serial.println(kpayload.timing);
      //   if (kpayload.keyword == keywordvalK) {
      //     // message received from keypad
      //     Serial.print(F("Key(s): '"));        
      //     for (int i=0;i<=maxkeys;i++){
      //       keytracking[i] = kpayload.keys[i];
      //       if (keytracking[i] > 0){
      //         Serial.print(keytracking[i]); 
      //         keyfollowup = true;       
      //       }
      //     }
      //     Serial.println(F("'")); 
      //     if (keyfollowup){
      //       // process the received characters, interpret as commands etc.



      //     }       

      //     // end of keypad message collection      
      //     mreceived = true;
      //   }
      //   else{
      //     Serial.println(F("Wrong Keypad keyword")); 
      //   }
      //   break;
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
  if ((fresh)&&(buttonsvalue > 0)) {
    // if ((unsigned long)(currentRFmilli - sendingTimer) > 5000){

    sendingTimer = currentRFmilli;

    tm_payload Txdata;
    Txdata.keyword = keywordvalT;
    Txdata.timing = currentRFmilli;
    Txdata.counter = counter++;
    Txdata.buttons = buttonsvalue;

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

bool handlebuttons(){
  static uint8_t buttons = 0xff;
  bool change = false;
  
  buttonsvalue = tm.getButtons();
  change = buttonsvalue != buttons;
  if (change){
    buttons = buttonsvalue;
    tm.writeLeds(buttons);
    Serial.print(F("Buttons "));
    Serial.print(buttons);    
    Serial.print(F(", "));
    Serial.println(buttons, BIN);
  }
  return change;
}

pulse_t pulse = PULSE1_16;
const uint8_t text[] = {0x7c, 0x1c, 0x78, 0x78, 0x5c, 0x54};
unsigned long timer = 0;



unsigned long currentmilli = 0;
bool fresh = false;

void loop() {

  network.update();

  currentmilli = millis();

  receiveRFnetwork(currentmilli);

  // for (uint8_t i=0;i<sizeof(text);i++) {
  //   tm.displayDig(7-i, text[i]);
  // }

  fresh = handlebuttons();

  // if (millis() - timer > 1000){
  //   timer = millis();
  //   Serial.print(F("Pulse: "));
  //   Serial.println(pulse);
  //   tm.displaySetBrightness(pulse);
  //   pulse = (pulse==PULSE1_16) ? PULSE14_16 : PULSE1_16;
  // }
  
  fresh = transmitRFnetwork(fresh, currentmilli);

}
