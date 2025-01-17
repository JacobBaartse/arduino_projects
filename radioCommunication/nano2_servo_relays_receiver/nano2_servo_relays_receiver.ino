/*
 * RF-Nano, headers, RF24 module attached, servo's attached, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>
#include <Servo.h>

#define radioChannel 106

const int buzzer = 9; //buzzer to arduino pin 9

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(7, 8); // nRF24L01 (CE, CSN)
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

void setup() {
  Serial.begin(115200);
  Serial.println(F(" ***<-->***"));  

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

  Servo1.attach(5);  // attaches the servo on pin 5 to the servo object
  Servo2.attach(6);  // attaches the servo on pin 6 to the servo object

  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output
}
 
unsigned long currentmilli = 0;
unsigned long buzzertime = 0;
bool newdata = false;
int numX = 0;
int numY = 0;

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
void transmitRFnetwork(unsigned long millitime){
  static unsigned long sendingTimer = 0;
  static bool w_ok;

  // Every 15 seconds
  if (millitime - sendingTimer > 15000){
    sendingTimer = millitime;

    Txdata.keyword = keywordvalM;
    Txdata.timing = millitime;
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


void loop() {

  network.update();

  currentmilli = millis();

  transmitRFnetwork(currentmilli);

  receiveRFnetwork();

  //************************ actuators ****************//

  if (newdata){
    if (Rxdata.bvalue > 0){
      if (currentmilli - buzzertime > 5000){ // create a silence time of 5 seconds
        tone(buzzer, 500); // Send a sound signal...
        buzzertime = currentmilli;
        Serial.println(F("Start buzzing"));
      }
    }
    // maybe first integrate the x end y value before mapping
    numX = map(Rxdata.xvalue, 0, 1023, 0, 180);
    numY = map(Rxdata.yvalue, 0, 1023, 0, 180);
    Servo1.write(numX);
    Servo2.write(numY);
  }
  if (currentmilli - buzzertime > 2000){ // create a buzzing time of 2 seconds
    noTone(buzzer); // Stop sound...
    buzzertime = currentmilli;
    Serial.println(F("Stop buzzing"));
  }

  newdata = false;
  //************************ actuators ****************//

  delay(200); // for debugging, this can be removed in practice

}
