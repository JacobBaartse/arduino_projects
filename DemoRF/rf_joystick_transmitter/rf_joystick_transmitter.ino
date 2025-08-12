/*
 * RF-Nano, no headers, USB-C with joystick connected, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#define radioChannel 102

#define VRX_PIN  A1 // Arduino pin connected to VRX pin
#define VRY_PIN  A0 // Arduino pin connected to VRY pin
#define SW_PIN   2  // Arduino pin connected to SW  pin, supporting interrupts
#define SW_PIN1  4  // Arduino pin connected to button 1
#define SW_PIN2  6  // Arduino pin connected to button 2

uint8_t xnValue = 0; // To store value of the X axis to the left
uint8_t ynValue = 0; // To store value of the Y axis to the bottom
uint8_t xpValue = 0; // To store value of the X axis to the right
uint8_t ypValue = 0; // To store value of the Y axis to the top

uint16_t xValue = 0; // To store value of the X axis
uint16_t yValue = 0; // To store value of the Y axis
uint16_t RefxValue = 0; // To store startup value of the X axis
uint16_t RefyValue = 0; // To store startup value of the Y axis

uint8_t bValue = 0; // To store value of the joystick button
uint8_t sw1Value = 0; // To store value of switch1
uint8_t sw2Value = 0; // To store value of switch2

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node01 = 01; // Address of this node in Octal format (04, 031, etc.)
const uint16_t node00 = 00; // Address of the home/host/controller node in Octal format

unsigned long const keywordvalM = 0xfeedbeef; 
unsigned long const keywordvalJ = 0xbcdffeda; 

struct joystick_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t xmvalue;
  uint8_t xpvalue;
  uint8_t ymvalue;
  uint8_t ypvalue;
  uint8_t count;
  uint8_t bvalue;
  uint8_t sw1value;
  uint8_t sw2value;
};

struct network_payload {
  unsigned long keyword;
  unsigned long counter;
  unsigned long timing;
  unsigned long command;
  unsigned long response;
  unsigned long data1;
  unsigned long data2;
  unsigned long data3;
};

bool newdata = false;

uint8_t checkSwitchButton1(uint8_t DigPin){
  static bool remval = false;
  static uint8_t b1val = 0;

  bool pressval = (digitalRead(DigPin) == LOW);
  if (pressval){
    if (remval){
      if (b1val < 0xff) 
        b1val++;
    }
    else {
      Serial.println(F(" button 1"));
      b1val = 100; // debouncing
      newdata = true;
    }
  }
  else {
    if (b1val > 0) 
      b1val--;
  }
  remval = pressval;
  return b1val;
}

uint8_t checkSwitchButton2(uint8_t DigPin){
  static bool remval = false;
  static uint8_t b2val = 0;

  bool pressval = (digitalRead(DigPin) == LOW);
  if (pressval){
    if (remval){
      if (b2val < 0xff) 
        b2val++;
    }
    else {
      Serial.println(F(" button 2"));
      b2val = 100; // debouncing
      newdata = true;
    }
  }
  else {
    if (b2val > 0) 
      b2val--;
  }
  remval = pressval;
  return b2val;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F(" ***** <> *****"));  
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
  network.begin(radioChannel, node01);

  RefxValue = analogRead(VRX_PIN);
  RefyValue = analogRead(VRY_PIN);

  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(SW_PIN1, INPUT_PULLUP);
  pinMode(SW_PIN2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(SW_PIN), joyButton, FALLING); // trigger when joystick button pressed
}
 
unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;

//===== Receiving =====//
void receiveRFnetwork(unsigned long currentRFmilli){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    network_payload Rxdata;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node != node00) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    if (Rxdata.keyword == keywordvalM){
      Serial.print(F("Timestamp: "));
      Serial.print(currentRFmilli);
      Serial.println(F(", RF data received"));

    }
    else{
      Serial.println(F("Received keyword failure"));
    }
  }
}

//===== Sending =====//
bool transmitRFnetwork(bool fresh, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
    sendingTimer = currentRFmilli;

    joystick_payload Txdata;
    Txdata.keyword = keywordvalJ;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    Txdata.xmvalue = xnValue;
    Txdata.xpvalue = xpValue;    
    Txdata.ymvalue = ynValue;
    Txdata.ypvalue = ypValue;
    Txdata.bvalue = bValue;
    Txdata.sw1value = sw1Value;
    Txdata.sw2value = sw2Value;

    Serial.print(F("Data: "));
    if (xnValue > 0){
      Serial.print(F(" xnvalue: "));
      Serial.print(Txdata.xmvalue);
    }
    if (xpValue > 0){
      Serial.print(F(" xpvalue: "));
      Serial.print(Txdata.xpvalue);
    }
    if (ynValue > 0){
      Serial.print(F(" ynvalue: "));
      Serial.print(Txdata.ymvalue);
    }
    if (ypValue > 0){
      Serial.print(F(" ypvalue: "));
      Serial.print(Txdata.ypvalue);
    }    
    if (bValue > 0){
      Serial.print(F(" bvalue: "));
      Serial.print(Txdata.bvalue);
    }
    if (sw1Value > 0){
      Serial.print(F(" sw1value: "));
      Serial.print(Txdata.sw1value);        
    }
    if (sw2Value > 0){
      Serial.print(F(" sw2value: "));
      Serial.print(Txdata.sw2value);
    }
    Serial.println();

    RF24NetworkHeader header0(node00, 'J'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      failcount++;
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    Serial.print(F("Message send ")); 
    if (w_ok){
      bValue = 0; 
      sw1Value = 0;
      sw2Value = 0;
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

    if (failcount > 10){
      fresh = false; // do not send a lot of messages continously
    }

    if(!fresh){ // clear buttons status always after 5 seconds
      bValue = 0; 
      sw1Value = 0;
      sw2Value = 0;
    }
  }

  return fresh;
}


void loop() {

  sw1Value = checkSwitchButton1(SW_PIN1);
  sw2Value = checkSwitchButton2(SW_PIN2);

  network.update();

  currentmilli = millis();

  receiveRFnetwork(currentmilli);

  //************************ sensors ****************//

  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);

  if (xValue < RefxValue){
    xnValue = map(xValue, 0, RefxValue, 255, 0);
    xpValue = 0;
  }
  else {
    xnValue = 0;
    xpValue = map(xValue, RefxValue, 1023, 0, 255);
  }
  if (yValue > RefyValue){
    ynValue = map(yValue, RefyValue, 1023, 0, 255);
    ypValue = 0;
  }
  else {
    ynValue = 0;
    ypValue = map(yValue, 0, RefyValue, 255, 0);
  }

  if ((xpValue > 2)||(xnValue > 2)||(ypValue > 2)||(ynValue > 2)){ // small threshold for sending data
    Serial.println(F("----"));  
    Serial.print(F("Xr: "));
    Serial.print(RefxValue);
    Serial.print(F(", X: "));
    Serial.print(xValue);
    Serial.print(F(", Yr: "));
    Serial.print(RefyValue);
    Serial.print(F(", Y: "));
    Serial.println(yValue);

    Serial.print(F("Xn: "));
    Serial.print(xnValue);
    Serial.print(F(", Xp: "));
    Serial.print(xpValue);
    Serial.print(F(", Yn: "));
    Serial.print(ynValue);
    Serial.print(F(", Yp: "));
    Serial.println(ypValue);    
    newdata = true;
  }

  //************************ sensors ****************//

  newdata = transmitRFnetwork(newdata, currentmilli);

}

void joyButton(){
  static unsigned long buttontime = 0;
  static unsigned long counter = 0;

  if (currentmilli - buttontime > 500){ // debounce to smaller than one press and processing per second
    buttontime = currentmilli;
    counter++;
    Serial.print(F("Joy button press: "));
    Serial.println(counter);
    bValue = 0xfe; // button pressed
    newdata = true;
  }
}
