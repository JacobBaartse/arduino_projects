/*
 * RF-Nano, no headers, USB-C with keypad connected, using RF24network library
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>
//#include "printf.h"
#include <Keypad.h>

#define radioChannel 106


/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node01 = 02; // Address of this node in Octal format (04, 031, etc.)
const uint16_t node00 = 00; // Address of the home/host/controller node in Octal format

unsigned long const keywordvalM = 0xfeefdeed; 
unsigned long const keywordvalS = 0xbeebedde; 

struct keypad_payload{
  uint32_t keyword;
  uint32_t timing;
  char keys[11];
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
void transmitRFnetwork(bool fresh){
  static unsigned long sendingTimer = 0;
  static bool w_ok;

  // Every 5 seconds, or on new data
  unsigned long currentRFmilli = millis();
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
    sendingTimer = currentRFmilli;

    keypad_payload Txdata;
    Txdata.keyword = keywordvalM;
    Txdata.timing = currentRFmilli;

    // Txdata.xvalue = xValue;
    // Txdata.yvalue = yValue;
    // Txdata.bvalue = bValue;
    // Txdata.sw1value = sw1Value;
    // Txdata.sw2value = sw2Value;

    RF24NetworkHeader header0(node00, 'K'); // address where the data is going
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
    Serial.println(currentRFmilli);

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
}

void loop() {

  network.update();

  currentmilli = millis();

  receiveRFnetwork();

  //************************ sensors ****************//


  //************************ sensors ****************//

  transmitRFnetwork(newdata);
  newdata = false;

  //delay(500); // for debugging, this can be removed in practice
}
