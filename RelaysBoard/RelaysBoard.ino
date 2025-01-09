/*

*/

#include <RF24Network.h>
#include <RF24.h>
//#include <SPI.h>

const uint16_t this_node = 031;   // Address of our node in Octal format (04, 031, etc.)

#define releayPin1 5  
#define releayPin2 6  

RF24 radio(10, 9);               // onboard nRF24L01 (CE, CSN)
RF24Network network(radio);      // Include the radio in the network

void setup() {
  Serial.begin(115200); 

  pinMode(releayPin1, OUTPUT);
  pinMode(releayPin2, OUTPUT);

  //SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network.begin(60, this_node); // (channel, node address)
  radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);

  //delay(1000);
  Serial.println(" ");  
  Serial.println(" *************** ");  
  Serial.println(" "); 
  Serial.flush();  
}

void loop() {
  
  network.update();

  //currentMillis = millis();   // capture the value of millis() only once in the loop
  digitalWrite(releayPin1, HIGH);
  delay(1500);
  digitalWrite(releayPin1, LOW);
  digitalWrite(releayPin2, HIGH);
  delay(1500);
  digitalWrite(releayPin2, LOW);
  delay(1500);

}
