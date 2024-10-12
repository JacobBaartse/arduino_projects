/*
  Arduino Wireless Network - Multiple NRF24L01 Tutorial
          == Base/ Master Node 00==
  by Dejan, www.HowToMechatronics.com
  Libraries:
  nRF24/RF24, https://github.com/nRF24/RF24
  nRF24/RF24Network, https://github.com/nRF24/RF24Network

  https://howtomechatronics.com/tutorials/arduino/how-to-build-an-arduino-wireless-network-with-multiple-nrf24l01-modules/#h-base-00-source-code

target (clone) UNO R4 Wifi, with RF24 module Long Range
location SO148
*/

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

//#define button 2
//#define led 3

RF24 radio(10, 9);               // nRF24L01 (CE,CSN)
RF24Network network(radio);      // Include the radio in the network
const uint16_t this_node = 00;   // Address of this node in Octal format (04, 031, etc.)
const uint16_t node01 = 01;      // Address of the other node in Octal format
//const uint16_t node012 = 012;
//const uint16_t node022 = 022;

void setup() {
  Serial.begin(115200);
  SPI.begin();
  radio.begin();
  // radio.setPALevel(RF24_PA_HIGH, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network.begin(90, this_node);  // (channel, node address)
  radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);
  //pinMode(button, INPUT_PULLUP);
  //pinMode(led, OUTPUT);
}

unsigned long angleValue = 543;

void loop() {
  network.update();

  //===== Receiving =====//
  while (network.available()) {     // Is there any incoming data?
    RF24NetworkHeader header;
    unsigned long incomingData;
    network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    //analogWrite(led, incomingData);    // PWM output to LED 01 (dimming)
    Serial.print(F("received message, header.from_node: "));
    Serial.println(header.from_node);
    Serial.print(F("incomingData: "));
    Serial.println(incomingData);
    Serial.println(" ");
  }

  //===== Sending =====//
  // Servo control at Node 01
  //unsigned long potValue = analogRead(A0);
  //unsigned long angleValue = map(potValue, 0, 1023, 0, 180); // Suitable for servo control
  angleValue++; 
  RF24NetworkHeader header1(node01);     // (Address where the data is going)
  bool ok = network.write(header1, &angleValue, sizeof(angleValue)); // Send the data
  if(!ok){
    Serial.print(F("Error sending message"));
  }
  // // LED Control at Node 012
  // unsigned long buttonState = digitalRead(button);
  // RF24NetworkHeader header4(node012);    // (Address where the data is going)
  // bool ok3 = network.write(header4, &buttonState, sizeof(buttonState)); // Send the data

  // // LEDs control at Node 022
  // unsigned long pot2Value = analogRead(A1);
  // RF24NetworkHeader header3(node022);    // (Address where the data is going)
  // bool ok2 = network.write(header3, &pot2Value, sizeof(pot2Value)); // Send the data
}