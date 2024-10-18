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

RF24 radio(10, 9);               // nRF24L01 (CE,CSN)
RF24Network network(radio);      // Include the radio in the network
const uint16_t this_node = 01;   // Address of our node in Octal format (04, 031, etc.)
const uint16_t master00 = 00;    // Address of the other node in Octal format
const uint16_t wrappingcounter = 255;

unsigned long const keywordval= 0xabcdfedc; 
unsigned long const command_none = 0x00; 
unsigned long const command_clear_counters = 0x01; 
unsigned long const command_status = 0x02; 
//unsigned long const command_reboot = 0x04; 
//unsigned long const command_status = 0x08; 
unsigned long const response_none = 0x00; 

struct network_payload {
  unsigned long keyword;
  unsigned long counter;
  unsigned long timing;
  unsigned long command;
  unsigned long response;
};

void setup() {
  Serial.begin(115200);
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network.begin(70, this_node); // (channel, node address)
  radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);
  delay(1000);
  // Serial.println(" ");  
  // Serial.println(" *************** ");  
  // Serial.println(" "); 
  // Serial.flush();  
}

unsigned long updatecounter(unsigned long countval, unsigned long wrapping=wrappingcounter) {
  countval++;
  if (countval > wrapping) countval = 1;
  return countval;
}

unsigned long receivedmsg = 0;
unsigned long sendmsg = 0;
unsigned long droppedmsg = 0;
unsigned long failedmsg = 0;

void messageStatus(unsigned long interval)
{
  static unsigned long statustime = 0;
  if (interval > 0){
    if (millis() < statustime) return;
    statustime = millis() + interval;
  }
  Serial.print(F("Network messages "));
  Serial.print(F("received: "));
  Serial.print(receivedmsg);
  Serial.print(F(", send: "));
  Serial.print(sendmsg);
  Serial.print(F(", missed: "));
  Serial.print(droppedmsg);
  Serial.print(F(", failed: "));
  Serial.print(failedmsg);
  Serial.println(" ");  
}

unsigned long sendingTimer = 0;
unsigned long sendingCounter = 0;
unsigned long receiveCounter = 0;
unsigned long rcvmsgcount = 10000;

unsigned long commanding = command_none;
unsigned long responding = response_none;
bool printstatus = false;
unsigned long receivedcommand = command_none;
unsigned long commandfrombase = command_none;

void loop() {
  network.update();

  if (printstatus) {
    messageStatus(0);
    printstatus = false;
  }

  //===== Receiving =====//
  while (network.available()) {     // Is there any incoming data?
    RF24NetworkHeader header;
    network_payload incomingData;
    network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    // if (header.from_node == 0) {    // If data comes from Node 02
    //   //myservo.write(incomingData);  // tell servo to go to a particular angle
    // }
    // if (header.from_node == 10) {    // If data comes from Node 012
    //   //digitalWrite(led, !incomingData);  // Turn on or off the LED 02
    // }
    if (header.from_node != 0) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    receivedmsg++;
    // Serial.print(F("IncomingData: "));
    // Serial.println(incomingData.counter);
    // check keyword and sequencenumber
    if (incomingData.keyword == keywordval){
      receiveCounter = incomingData.counter;
      receivedcommand = incomingData.command;
      if (rcvmsgcount > wrappingcounter) { // initialisation
        rcvmsgcount = receiveCounter;
      }
      else { // check received message value
        if (rcvmsgcount != receiveCounter) {
          if (receivedmsg > 1) droppedmsg++; // this could be multiple as well
          rcvmsgcount = receiveCounter; // re-synchronize
        }
      }
      rcvmsgcount = updatecounter(rcvmsgcount); // calculate next expected message 
    }
    else{
      Serial.println(F("Keyword failure"));
      receivedcommand = command_none;
    }
    if (receivedcommand > command_none) {
      commandfrombase = receivedcommand;
    }
  }

  //===== Sending =====//
    // Meanwhile, every x seconds...
  unsigned long currentmilli = millis();
  if(currentmilli - sendingTimer > 15000) {
    sendingTimer = currentmilli;
    sendingCounter = updatecounter(sendingCounter); 
    RF24NetworkHeader header0(master00); // (Address where the data is going)

    if (commandfrombase > command_none) {
      if ((commandfrombase & command_clear_counters) > 0) {
        printstatus = true;
        receivedmsg = 0;
        sendmsg = 0;
        droppedmsg = 0;
        failedmsg = 0;
      }
      if ((commandfrombase & command_status) > 0) {
        printstatus = true;
        responding = receivedmsg;
        // Serial.print(F("1 responding: "));
        // Serial.println(responding, HEX);
        responding = (responding << 8) + sendmsg;
        // Serial.print(F("2 responding: "));
        // Serial.println(responding, HEX);
        responding = (responding << 8) + droppedmsg;
        // Serial.print(F("3 responding: "));
        // Serial.println(responding, HEX);
        responding = (responding << 8) + failedmsg;
        // Serial.print(F("4 responding: "));
        // Serial.println(responding, HEX);
      }
      else {
        Serial.print(F("TBD commandfrombase: "));
        Serial.println(commandfrombase);
      }
      commandfrombase = command_none;
    }

    network_payload outgoing = {keywordval, sendingCounter, currentmilli, commanding, responding};
    bool ok = network.write(header0, &outgoing, sizeof(outgoing)); // Send the data
    if (!ok) {
      Serial.print(F("Retry sending message: "));
      Serial.println(sendingCounter);
      ok = network.write(header0, &outgoing, sizeof(outgoing)); // retry once
    }
    if (ok) {
      sendmsg++;
    }
    else{
      Serial.print(F("Error sending message: "));
      Serial.println(sendingCounter);
      failedmsg++;
    }
    commanding = command_none;
    responding = response_none;
  }
}
