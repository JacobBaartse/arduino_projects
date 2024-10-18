/*
  Arduino Wireless Network - Multiple NRF24L01 Tutorial
          == Base/Master Node 00 ==
  by Dejan, www.HowToMechatronics.com
  Libraries:
  nRF24/RF24, https://github.com/nRF24/RF24
  nRF24/RF24Network, https://github.com/nRF24/RF24Network


  https://howtomechatronics.com/tutorials/arduino/how-to-build-an-arduino-wireless-network-with-multiple-nrf24l01-modules/#h-base-00-source-code

target (clone) UNO R4 Wifi, with RF24 module Long Range
location SO148
*/

#include "matrix.h"
#include "networking.h"
#include "WiFiS3.h"
#include "RTC.h"
#include <NTPClient.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>


RF24 radio(8, 7);               // nRF24L01 (CE,CSN)
RF24Network network(radio);      // Include the radio in the network
const uint16_t this_node = 00;   // Address of this node in Octal format (04, 031, etc.)
const uint16_t node01 = 01;      // Address of the other node in Octal format
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

WiFiServer server(80);
IPAddress IPhere;

WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);

void setup() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  RTC.begin();
  matrix.begin();
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network.begin(70, this_node); // (channel, node address)
  radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);

  String timestamp = __TIMESTAMP__;
  Serial.print("Creation/build time: ");
  Serial.println(timestamp);

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    Serial.println("Please upgrade the firmware for the WiFi module");

  // attempt to connect to WiFi network:
  int wifistatus = WifiConnect();
  if (wifistatus == WL_CONNECTED){
    server.begin();
    IPhere = printWifiStatus(connection);
  }
  else{ // stop the wifi connection
    WiFi.disconnect();
  }
  startupscrollingtext(String("-->: ") + IPhere.toString());

  Serial.println("\nStarting connection to NTP server...");
  timeClient.begin();
  timeClient.update();
  // Get the current date and time from an NTP server and convert
  // it to UTC +2 by passing the time zone offset in hours.
  // You may change the time zone offset to your local one.
  auto timeZoneOffsetHours = 2;
  auto unixTime = timeClient.getEpochTime() + (timeZoneOffsetHours * 3600);
  Serial.print("Unix time = ");
  Serial.println(unixTime);
  RTCTime timeToSet = RTCTime(unixTime);
  RTC.setTime(timeToSet);
  // Retrieve the date and time from the RTC and print them
  RTCTime currentTime;
  RTC.getTime(currentTime); 
  Serial.println("The RTC was just set to: " + String(currentTime));

  Serial.println(" ");  
  Serial.println(" *************** ");  
  Serial.println(" ");  
  Serial.flush(); 

  digitalWrite(LED_BUILTIN, LOW);
}

unsigned long updatecounter(unsigned long countval, unsigned long wrapping=wrappingcounter) {
  countval++;
  if (countval > wrapping) countval = 1;
  return countval;
}

// void restart_arduino(){
//   Serial.println("Restart the Arduino UNO board...");
//   delay(2000);
//   NVIC_SystemReset();
// }

unsigned long receivedmsg = 0;
unsigned long sendmsg = 0;
unsigned long droppedmsg = 0;
unsigned long failedmsg = 0;

bool messageStatus(unsigned long interval)
{
  static unsigned long statustime = 0;
  if (millis() < statustime) return false;
  statustime = millis() + interval;
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
  // if (statustime > 0xf0000000){
  //   restart_arduino();
  // }
  return true;
}

unsigned long sendingTimer = 0;
unsigned long sendingCounter = 0;
unsigned long receiveCounter = 0;
unsigned long rcvmsgcount = 2 * wrappingcounter;

unsigned long commanding = command_none;
unsigned long responding = response_none;
unsigned long responsefromremote = response_none;

void loop() {
  network.update();

  bool statusprinted = messageStatus(60000);
  if (statusprinted) {
    commanding = command_status;
  }

  //===== Receiving =====//
  while (network.available()) {     // Is there any incoming data?
    RF24NetworkHeader header;
    network_payload incomingData;
    network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    if (header.from_node != 1) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    receivedmsg++;
    // Serial.print(F("incomingData: "));
    // Serial.println(incomingData.counter);
    // check keyword and sequencenumber
    if (incomingData.keyword == keywordval){
      receiveCounter = incomingData.counter;
      responsefromremote = incomingData.response;
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
    }
  }

  if (responsefromremote > response_none) {
    Serial.print(F("responsefromremote: "));
    Serial.println(responsefromremote, HEX);
    unsigned long fails = responsefromremote & 0xff;
    unsigned long drops = (responsefromremote >> 8) & 0xff;
    unsigned long rsend = (responsefromremote >> 16) & 0xff;
    unsigned long rcoll = (responsefromremote >> 24) & 0xff;
    Serial.print(F("Remote network messages "));
    Serial.print(F("received: "));
    Serial.print(rcoll);
    Serial.print(F(", send: "));
    Serial.print(rsend);
    Serial.print(F(", missed: "));
    Serial.print(drops);
    Serial.print(F(", failed: "));
    Serial.print(fails);
    Serial.println(" ");  
    responsefromremote = response_none;
  }

  //===== Sending =====//
  // Meanwhile, every x seconds...
  unsigned long currentmilli = millis();
  if(currentmilli - sendingTimer > 5000) {
    sendingTimer = currentmilli;
    sendingCounter = updatecounter(sendingCounter); 
    RF24NetworkHeader header1(node01); // (Address where the data is going)
    network_payload outgoing = {keywordval, sendingCounter, currentmilli, commanding, responding};
    bool ok = network.write(header1, &outgoing, sizeof(outgoing)); // Send the data
    if (!ok) {
      Serial.print(F("Retry sending message: "));
      Serial.println(sendingCounter);      
      ok = network.write(header1, &outgoing, sizeof(outgoing)); // retry once
    }
    if (ok) {
      sendmsg++;
      commanding = command_none;
    }
    else{
      Serial.print(F("Error sending message: "));
      Serial.println(sendingCounter);
      failedmsg++;
    }
    responding = response_none;
  }

  loadsequencepicture();
}
