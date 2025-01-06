/*
 * RF-Nano, no headers, USB-C with joystick connected, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#define radioChannel 106

#define VRX_PIN  A1 // Arduino pin connected to VRX pin
#define VRY_PIN  A0 // Arduino pin connected to VRY pin
#define SW_PIN   2  // Arduino pin connected to SW  pin, supporting interrupts

unsigned int xValue = 0; // To store value of the X axis
unsigned int yValue = 0; // To store value of the Y axis
unsigned int bValue = 0; // To store value of the button
unsigned int remx = 0;
unsigned int remy = 0;

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(7, 8); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node01 = 01; // Address of this node in Octal format (04, 031, etc.)
const uint16_t node00 = 00; // Address of the other node in Octal format

unsigned long const keywordvalM = 0xfeebbeef; 
unsigned long const keywordvalS = 0xbeeffeeb; 

typedef struct {
  unsigned long keyword;
  unsigned long timing;
  unsigned int xvalue;
  unsigned int yvalue;
  unsigned int bvalue;
} network_payload;
network_payload Txdata;
network_payload Rxdata;

void setup() {
  Serial.begin(115200);
  Serial.println(F(" *****<>*****"));  

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_250KBPS);
  network.begin(radioChannel, node01);

  //pinMode(SW_PIN, INPUT); // this toggles too much
  pinMode(SW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SW_PIN), joyButton, FALLING);
}
 
unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;

//===== Receiving =====//
void receiveRFnetwork(){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node != node00) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    if (Rxdata.keyword == keywordvalS){

    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
}

//===== Sending =====//
void transmitRFnetwork(bool fresh){
  static unsigned long sendingTimer = 0;
  static bool w_ok;

  // Every 5 seconds, or on new data
  unsigned long currentmilli = millis();
  if ((fresh)||(currentmilli - sendingTimer > 5000)){
    sendingTimer = currentmilli;

    Txdata.keyword = keywordvalM;
    Txdata.timing = currentmilli;
    Txdata.xvalue = xValue;
    Txdata.yvalue = yValue;
    Txdata.bvalue = bValue;
    RF24NetworkHeader header0(node00); // (Address where the data is going)
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    if (w_ok){
      Serial.print(F("Message send "));
    }    
    else{
      Serial.print(F("Message not send "));
    }
    Serial.println(currentmilli);
  }
}

int divX = 0;
int divY = 0;
int divXr = 0;
int divYr = 0;
bool newdata = false;

void loop() {

  network.update();

  currentmilli = millis();

  receiveRFnetwork();

  //************************ sensors ****************//

  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);

  // calculate trigger, difference with previous measurements
  // this did not work when put in 1 line in the if statement
  // the abs value wass printed several times < 0 (which should never happen)
  // that is why it is now over multiple lines
  divXr = xValue - remx;
  divYr = yValue - remy;
  divX = abs(divXr);
  divY = abs(divYr);
  if ((divY > 1)||(divX > 1)){
  //   Serial.print(F("dX: "));
  //   Serial.print(divX);    
  //   Serial.print(F(", dY: "));
  //   Serial.println(divY);
  // }
  // if ((divY > 2)||(divX > 2)){
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

  transmitRFnetwork(newdata);
  newdata = false;

  //delay(500); // for debugging, this can be removed in practice

}

void joyButton(){
  static unsigned long buttontime = 0;
  static int buttonstate = HIGH;
  static int counter = 0;
  if (currentmilli - buttontime > 500){ // debounce to one press per second
    buttontime = currentmilli;
    counter++;
    Serial.print(F("Button pressed "));
    Serial.print(counter);
    Serial.print(F(": "));
    buttonstate = digitalRead(SW_PIN);
    if (buttonstate == HIGH) bValue = 0; // button not pressed
    else bValue = 0xff; // button pressed
    Serial.println(bValue);
    newdata = true;
  }
}
