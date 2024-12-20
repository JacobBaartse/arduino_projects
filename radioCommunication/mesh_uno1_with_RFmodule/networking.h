/*
 * 
 */

#include <RF24Network.h>
#include <RF24.h>
#include "WiFiS3.h"
#include "networkdata.h"

int initWiFi(char* pssid, char* ppass, int timeout=10000) {
  long endTime = millis() + timeout;
  int WiFistatus = WL_IDLE_STATUS;
  WiFi.begin(pssid, ppass);
  // Serial.print("Connecting to WiFi...");
  while ((WiFistatus != WL_CONNECTED) && (millis() < endTime)) {
    // Serial.print('.');
    delay(1000);
    WiFistatus = WiFi.status();
  }
  // Serial.println(" !");
  return WiFistatus;
} 

unsigned int connection = 0;

IPAddress printWifiStatus(int connect) {
  Serial.println();
  Serial.println();
  // print your board's IP address:
  Serial.print("IP Address: ");
  IPAddress here = WiFi.localIP();
  Serial.print(here);

  // print the received signal strength:
  Serial.print(", signal strength (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(String(" dBm, con: ") + connect);
  Serial.flush();
  return here;
}

void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      Serial.println("WEP");
      break;
    case ENC_TYPE_WPA:
      Serial.println("WPA");
      break;
    case ENC_TYPE_WPA2:
      Serial.println("WPA2");
      break;
    case ENC_TYPE_WPA3:
      Serial.print("WPA3");
      break;   
    case ENC_TYPE_NONE:
      Serial.println("None");
      break;
    case ENC_TYPE_AUTO:
      Serial.println("Auto");
      break;
    case ENC_TYPE_UNKNOWN:
    default:
      Serial.println("Unknown");
      break;
  }
}

String findNetwork() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("Couldn't get WiFi information");
    while (true);
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print(" Signal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.println(" dBm");
    // Serial.print(" Encryption: ");
    // printEncryptionType(WiFi.encryptionType(thisNet));
    // Serial.println(" ");
  }

  String availSSID = "";
  String foundSSID = "";
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    availSSID = WiFi.SSID(thisNet);
    for (int storage = 0; knownnetworks[storage][0] != "EOR"; storage++){
      if (availSSID == knownnetworks[storage][0]){
        //if (foundSSID == "") { // collect only the first matching networkID (the strongest???)
        foundSSID = availSSID;
        break;
        //}
      }
    }
  }
  Serial.print("foundSSID: ");
  Serial.println(foundSSID);
  return foundSSID;
}

String getNetworkPassword(String SSID) {
  String foundPWD = "";
  for (int storage = 0; knownnetworks[storage][0] != "EOR"; storage++){
    if (SSID == knownnetworks[storage][0]){
      foundPWD = knownnetworks[storage][1];
      break;
    }
  }
  return foundPWD;
}

String getNetworkLocation(String SSID){
  String foundLocation = "";
  for (int storage = 0; knownnetworks[storage][0] != "EOR"; storage++){
    if (SSID == knownnetworks[storage][0]){
      foundLocation = knownnetworks[storage][2];
      break;
    }
  }
  return foundLocation;
}

int WifiConnect(){
  String SSIDfound = findNetwork();
  String SSIDpwd = getNetworkPassword(SSIDfound);
  String SSIDlocation = getNetworkLocation(SSIDfound);
  
  Serial.print("Wifi network: ");
  Serial.print(SSIDfound);
  Serial.print(", ");
  Serial.println(SSIDlocation);

  // attempt to connect to WiFi network:
  int stat = WL_IDLE_STATUS;

  unsigned int ssid_len = SSIDfound.length() + 1;
  unsigned int pass_len = SSIDpwd.length() + 1;
  char ssid[ssid_len];
  char pass[pass_len];
  SSIDfound.toCharArray(ssid, ssid_len);
  SSIDpwd.toCharArray(pass, pass_len);

  while (stat != WL_CONNECTED){
    stat = initWiFi(ssid, pass, 7000);
    connection++;
  }  
  return stat;
}

//===== Radio =====//

const uint16_t this_node = 00;   // Address of this node in Octal format (04, 031, etc.)
const uint16_t node01 = 01;      // Address of the other node in Octal format

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(8, 7);                // nRF24L01 (CE, CSN)
RF24Network network(radio);      // Include the radio in the network

const uint16_t wrappingcounter = 255;

unsigned long const keywordval= 0xabcdfedc; 
unsigned long const command_none = 0x00; 
unsigned long const command_clear_counters = 0x01; 
unsigned long const command_status = 0x02; 
//unsigned long const command_reboot = 0x04; 
//unsigned long const command_unixtime = 0x08; 
unsigned long const response_none = 0x00; 
unsigned long const response_button = 0x01; 
unsigned long const response_ack = 0x02; 

// max 32 bytes can be in the FIFO of the nRF24L01
// that means maximum 8 data items of unsigned long
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

unsigned long receivedmsg = 0;
unsigned long sendmsg = 0;
unsigned long droppedmsg = 0;
unsigned long failedmsg = 0;

unsigned long sendingTimer = 0;
unsigned long sendingCounter = 0;
unsigned long receiveCounter = 0;
unsigned long rcvmsgcount = 2 * wrappingcounter;

unsigned long commanding = command_none;
unsigned long responding = response_none;
unsigned long data1 = response_none;
unsigned long data2 = response_none;
unsigned long data3 = response_none;
unsigned long responsefromremote = response_none;

unsigned long updatecounter(unsigned long countval, unsigned long wrapping=wrappingcounter) {
  // countval++;
  // if (countval > wrapping) countval = 1;
  countval >= wrapping ? countval=1 : countval++;
  return countval;
}

//===== Receiving =====//
unsigned int receiveRFnetwork(){
  unsigned int reaction = 0;

  while (network.available()) { // Is there any incoming data?
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
        commanding = command_clear_counters;
      }
      else { // check received message value
        if (rcvmsgcount != receiveCounter) {
          if (receivedmsg > 1) {
             droppedmsg++; // this could be multiple as well
            Serial.print(F("Missed network message(s): "));
            Serial.print(F("received id: "));
            Serial.print(receiveCounter);
            Serial.print(F(", expected id: "));
            Serial.println(rcvmsgcount);             
          }
          rcvmsgcount = receiveCounter; // re-synchronize
        }
      }
      rcvmsgcount = updatecounter(rcvmsgcount); // calculate next expected message 
    }
    else{
      Serial.println(F("Keyword failure"));
    }

    if (responsefromremote > response_none) {
      // Serial.print(F("responsefromremote: "));
      // Serial.println(responsefromremote, HEX);
      unsigned long fails = incomingData.data2 & 0xffff;
      unsigned long drops = (incomingData.data2 >> 16) & 0xffff;
      unsigned long rsend = incomingData.data1;
      unsigned long rcoll = incomingData.response;
      Serial.print(F("Remote network messages "));
      Serial.print(F("received: "));
      Serial.print(rcoll);
      Serial.print(F(", send: "));
      Serial.print(rsend);
      Serial.print(F(", missed: "));
      Serial.print(drops);
      Serial.print(F(", failed: "));
      Serial.println(fails);
      Serial.println(F("-"));  
      responsefromremote = response_none;
    }
  }
  return reaction;
}

//===== Sending =====//
unsigned int transmitRFnetwork(unsigned long commandtx){
  static unsigned long sendingTimer = 0;
  unsigned int traction = 0;

  // Every x seconds...
  unsigned long currentmilli = millis();
  if(currentmilli - sendingTimer > 5000) {
    sendingTimer = currentmilli;
    sendingCounter = updatecounter(sendingCounter); 
    RF24NetworkHeader header1(node01); // (Address where the data is going)
    network_payload outgoing = {keywordval, sendingCounter, currentmilli, commandtx, responding, data1, data2, data3};
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
  return traction;
}
