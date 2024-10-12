/*
  Arduino Wireless Network - Multiple NRF24L01 Tutorial
        == Node 01 (Child of Master node 00) ==

https://howtomechatronics.com/tutorials/arduino/how-to-build-an-arduino-wireless-network-with-multiple-nrf24l01-modules/#h-node-01-source-code

target old nano, with RF24 module Long Range
location JWF21
*/

#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
//#include <Servo.h>

//#define led 2

RF24 radio(10, 9);               // nRF24L01 (CE,CSN)
RF24Network network(radio);      // Include the radio in the network
const uint16_t this_node = 01;   // Address of our node in Octal format (04, 031, etc.)
const uint16_t master00 = 00;    // Address of the other node in Octal format

//Servo myservo;  // create servo object to control a servo

void setup() {
  Serial.begin(115200);
  SPI.begin();
  radio.begin();
  // radio.setPALevel(RF24_PA_HIGH, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network.begin(90, this_node); // (channel, node address)
  radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);
  //myservo.attach(3);   // (servo pin)
  //pinMode(led, OUTPUT);
}

unsigned long angleValue = 123;

void loop() {
  network.update();

  //===== Receiving =====//
  while (network.available()) {     // Is there any incoming data?
    RF24NetworkHeader header;
    unsigned long incomingData;
    network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    // if (header.from_node == 0) {    // If data comes from Node 02
    //   //myservo.write(incomingData);  // tell servo to go to a particular angle
    // }
    // if (header.from_node == 10) {    // If data comes from Node 012
    //   //digitalWrite(led, !incomingData);  // Turn on or off the LED 02
    // }
    Serial.print(F("received message, header.from_node: "));
    Serial.println(header.from_node);
    Serial.print(F("incomingData: "));
    Serial.println(incomingData);
    Serial.println(" ");
  }

  //===== Sending =====//
  angleValue++; 
  RF24NetworkHeader header0(master00);     // (Address where the data is going)
  bool ok = network.write(header0, &angleValue, sizeof(angleValue)); // Send the data
  if(!ok){
    Serial.print(F("Error sending message"));
  }
}