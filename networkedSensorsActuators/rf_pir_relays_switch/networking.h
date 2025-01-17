/*
 * RF24 networking 'library'
 */

#include "RF24Network.h"
#include "RF24.h"

//===== Radio =====//

const uint16_t this_node = 03; // Address of this node in Octal format (04, 031, etc.)
const uint16_t base_node = 00; // Address of the base node in Octal format

#define radioChannel 96

/**** Configure the nrf24l01 CE and CSN pins ****/
// for the NANO with onboard RF24 module:
// RF24 radio(10, 9); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external RF24 module:
//RF24 radio(8, 7); // nRF24L01 (CE, CSN)

#define CE_PIN 8
#define CSN_PIN 7
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t wrappingcounter = 255;

unsigned long const keywordval_03 = 0xabcdfedc; 

unsigned long updatecounter(unsigned long countval, unsigned long wrapping=wrappingcounter){
  // countval++;
  // if (countval > wrapping) countval = 1;
  countval >= wrapping ? countval=1 : countval++;
  return countval;
}

// max 32 bytes can be in the FIFO of the nRF24L01
// that means maximum 8 data items of unsigned long
struct network_payload {
  unsigned long keyword;
  unsigned long counter;
  unsigned long timing;
  unsigned long data1;
  unsigned long data2;
  unsigned long data3;
  unsigned long data4;
  unsigned long data5;
};

unsigned long rdata1 = 0;
unsigned long rdata2 = 0;
unsigned long rdata3 = 0;
unsigned long rdata4 = 0;
unsigned long rdata5 = 0;

unsigned long tdata1 = 0;
unsigned long tdata2 = 0;
unsigned long tdata3 = 0;
unsigned long tdata4 = 0;
unsigned long tdata5 = 0;

//===== Receiving =====//
unsigned int receiveRFnetwork(){
  static unsigned long receivingCounter = 0;
  static unsigned long receivedmsg = 0;
  unsigned int reaction = 0;

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    network_payload incomingData;
    network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    if (header.from_node != base_node) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    receivedmsg++;
    if (incomingData.keyword == keywordval_03){
      // Serial.println(incomingData.counter);
      // Serial.println(incomingData.timing);
      rdata1 = incomingData.data1;
      rdata2 = incomingData.data2;
      rdata3 = incomingData.data3;
      rdata4 = incomingData.data4;
      rdata5 = incomingData.data5;
      reaction = 5;
    }
    else{
      Serial.print(F("Keyword failure: "));
      Serial.println(incomingData.keyword);
    }
  }
  return reaction;
}

//===== Sending =====//
unsigned int transmitRFnetwork(bool immediate){
  static unsigned long sendingTimer = 0;
  static unsigned long sendingCounter = 0;
  static unsigned long sendmsg = 0;
  unsigned int traction = 0;

  // Every x seconds...
  unsigned long currentmilli = millis();
  if((immediate)||(currentmilli - sendingTimer > 5000)){
    sendingTimer = currentmilli;
    sendingCounter = updatecounter(sendingCounter); 
    RF24NetworkHeader headerb(base_node); // (Address where the data is going)
     network_payload outgoing = {keywordval_03, sendingCounter, currentmilli, tdata1, tdata2, tdata3, tdata4, tdata5};
    bool ok = network.write(headerb, &outgoing, sizeof(outgoing)); // Send the data
    if (!ok) {
      Serial.print(F("Retry sending message: "));
      Serial.println(sendingCounter);      
      ok = network.write(headerb, &outgoing, sizeof(outgoing)); // retry once
    }
    if (ok) {
      sendmsg++;
      tdata1 = 0;
      tdata2 = 0;
      tdata3 = 0;
      tdata4 = 0;
      tdata5 = 0;
      traction = 0x1ff;
    }
    else{
      Serial.print(F("Error sending message: "));
      Serial.println(sendingCounter);
      traction = 3;
    }
  }
  return traction;
}
