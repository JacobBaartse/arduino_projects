/*
 * RF-Nano, headers, RF24 module attached, servo's attached, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>
#include <Servo.h>

#define radioChannel 106


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(8, 7); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node00 = 00; // Address of this node in Octal format (04, 031, etc.)
const uint16_t node01 = 01; // Address of the other node in Octal format

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

Servo Servo1;  // create servo object to control a servo
Servo Servo2;  // create servo object to control a servo
Servo Servo3;  // create servo object to control a servo

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
  network.begin(radioChannel, node00);

  Servo1.attach(9);  // attaches the servo on pin 9 to the servo object
  Servo2.attach(9);  // attaches the servo on pin 9 to the servo object
  Servo3.attach(9);  // attaches the servo on pin 9 to the servo object

}
 
unsigned long currentmilli = 0;

//===== Receiving =====//
void receiveRFnetwork(){

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node != node01) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    if (Rxdata.keyword == keywordvalM){
      newdata = true;
      Serial.println(F("new data received"));
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
    // Txdata.xvalue = xValue;
    // Txdata.yvalue = yValue;
    // Txdata.bvalue = bValue;
    RF24NetworkHeader header1(node01); // (Address where the data is going)
    w_ok = network.write(header1, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      delay(50);
      w_ok = network.write(header1, &Txdata, sizeof(Txdata)); // Send the data
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

bool newdata = false;

void loop() {

  network.update();

  currentmilli = millis();

  receiveRFnetwork();

  //************************ actuators ****************//

  if (newdata){

  }
  // for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
  //   // in steps of 1 degree
  //   myservo.write(pos);              // tell servo to go to position in variable 'pos'
  //   delay(15);                       // waits 15ms for the servo to reach the position
  // }

  //************************ actuators ****************//
  newdata = false;

  transmitRFnetwork();

  delay(500); // for debugging, this can be removed in practice

}
