/*
 * RF-Nano, no headers, USB-C with joystick connected, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>
//#include "printf.h"

#define radioChannel 106

#define VRX_PIN  A1 // Arduino pin connected to VRX pin
#define VRY_PIN  A0 // Arduino pin connected to VRY pin
#define SW_PIN   2  // Arduino pin connected to SW  pin, supporting interrupts
#define SW_PIN1  4  // Arduino pin connected to button 1
#define SW_PIN2  6  // Arduino pin connected to button 2

uint16_t xValue = 0; // To store value of the X axis
uint16_t yValue = 0; // To store value of the Y axis
uint8_t bValue = 0; // To store value of the button
uint8_t sw1Value = 0; // To store value of switch1
uint8_t sw2Value = 0; // To store value of switch2
uint16_t remx = 0;
uint16_t remy = 0;

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node01 = 01; // Address of this node in Octal format (04, 031, etc.)
const uint16_t node00 = 00; // Address of the home/host/controller node in Octal format

unsigned long const keywordvalM = 0xfeedbeef; 
unsigned long const keywordvalS = 0xbeeffeed; 

struct joystick_payload{
  uint32_t keyword;
  uint32_t timing;
  uint16_t xvalue;
  uint16_t yvalue;
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
      if (b1val < 0xff) b1val++;
    }
    else {
      Serial.println(F(" button 1"));
      b1val = 100; // debouncing
      newdata = true;
    }
  }
  else {
    if (b1val > 0) b1val--;
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
      if (b2val < 0xff) b2val++;
    }
    else {
      Serial.println(F(" button 2"));
      b2val = 100; // debouncing
      newdata = true;
    }
  }
  else {
    if (b2val > 0) b2val--;
  }
  remval = pressval;
  return b2val;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F(", creation/build time: "));
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

  pinMode(SW_PIN, INPUT_PULLUP);
  pinMode(SW_PIN1, INPUT_PULLUP);
  pinMode(SW_PIN2, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(SW_PIN), joyButton, FALLING);

  //printf_begin();
}
 
unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;

//===== Receiving =====//
void receiveRFnetwork(){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    network_payload Rxdata;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node != node00) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    if (Rxdata.keyword == keywordvalS){

      Serial.println(F("new data received"));
    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
}

//===== Sending =====//
bool transmitRFnetwork(bool fresh){
  static unsigned long sendingTimer = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  unsigned long currentRFmilli = millis();
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
    sendingTimer = currentRFmilli;

    joystick_payload Txdata;
    Txdata.keyword = keywordvalM;
    Txdata.timing = currentRFmilli;
    Txdata.xvalue = xValue;
    Txdata.yvalue = yValue;
    Txdata.bvalue = bValue;
    Txdata.sw1value = sw1Value;
    Txdata.sw2value = sw2Value;

    RF24NetworkHeader header0(node00, 'J'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    Serial.print(F("Message send ")); 
    if (w_ok){
      bValue = 0; 
      sw1Value = 0;
      sw2Value = 0;
      fresh = false;
    }    
    else{
      Serial.print(F("failed "));
    }
    Serial.println(currentRFmilli);

    if(!fresh){ // clear buttons status always after 5 seconds
      bValue = 0; 
      sw1Value = 0;
      sw2Value = 0;
    }

    // // print data using &Txdata, sizeof(Txdata)
    // //Serial.println((char*)&Txdata);
    // Serial.println(F("--:"));
    // char buff[3] = "";
    // uint8_t* ptr = (uint8_t*)&Txdata;
    // for (size_t i = 0; i < sizeof(Txdata); i++)
    // {
    //   printf(buff, "%02x", (*(ptr + i)));
    //   Serial.print(buff);
    // }
    // // for(int i = 0; i < sizeof(Txdata); i++)
    // // {
    // //   char byteval = ((char*)&Txdata)[i];
    // //   printf(buff, "%02X", (uint8_t)byteval);
    // //   Serial.print(buff);
    // // }
    // Serial.println(F("<--"));
  }

  return fresh;
}

int divX = 0;
int divY = 0;
int divXr = 0;
int divYr = 0;

void loop() {

  sw1Value = checkSwitchButton1(SW_PIN1);
  sw2Value = checkSwitchButton2(SW_PIN2);

  network.update();

  currentmilli = millis();

  receiveRFnetwork();

  //************************ sensors ****************//

  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);

  // calculate trigger, difference with previous measurements
  // this did not work when put in 1 line in the if statement
  // the abs value was printed several times < 0 (which should never happen)
  // that is why it is now over multiple lines
  divXr = xValue - remx;
  divYr = yValue - remy;
  divX = abs(divXr);
  divY = abs(divYr);
  // if ((divY > 1)||(divX > 1)){
  //   Serial.print(F("dX: "));
  //   Serial.print(divX);    
  //   Serial.print(F(", dY: "));
  //   Serial.println(divY);
  // }
  if ((divY > 4)||(divX > 4)){
    Serial.println(F("----"));  
    Serial.print(F("X: "));
    Serial.print(xValue);
    Serial.print(F(", rX: "));
    Serial.print(remx);
    Serial.print(F(", dX: "));
    Serial.println(divX);
    Serial.print(F("Y: "));
    Serial.print(yValue);
    Serial.print(F(", rY: "));
    Serial.print(remy);    
    Serial.print(F(", dY: "));
    Serial.println(divY);    
    remx = xValue; 
    remy = yValue;
    newdata = true;
  }

  //************************ sensors ****************//

  newdata = transmitRFnetwork(newdata);

}

void joyButton(){
  static unsigned long buttontime = 0;
  //static int buttonstate = HIGH;
  static unsigned long counter = 0;

  if (currentmilli - buttontime > 500){ // debounce to smaller than one press and processing per second
    buttontime = currentmilli;
    counter++;
    Serial.print(F("Joy button press: "));
    Serial.println(counter);
    //Serial.print(F(": "));
    //buttonstate = digitalRead(SW_PIN);
    //if (buttonstate == LOW) bValue = 0xfe; // button pressed
    bValue = 0xfe; // button pressed
    //Serial.println(bValue);
    newdata = true;
  }
}
