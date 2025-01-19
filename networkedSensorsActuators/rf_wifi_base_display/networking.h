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

const uint16_t base_node = 00; // Address of the this node in Octal format
const uint16_t pir_node = 03; // Address of PIR node in Octal format (04, 031, etc.)

#define radioChannel 96

/**** Configure the nrf24l01 CE and CSN pins ****/
// for the NANO with onboard RF24 module:
// RF24 radio(10, 9); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external RF24 module:
//RF24 radio(8, 7); // nRF24L01 (CE, CSN)

#define CE_PIN 5
#define CSN_PIN 4
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t wrappingcounter = 255;

//unsigned long const keywordval = 0xabcdfedc; 
unsigned long const keywordval_02 = 0xcdabfedc; 
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
    Serial.print(F("received message, from_node: "));
    Serial.println(header.from_node);
    Serial.print(F("Keyword: 0x"));
    Serial.println(incomingData.keyword, HEX);
    
    if (header.from_node == base_node) {
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    receivedmsg++;
    if (incomingData.keyword == keywordval_03){
      Serial.print(F("msg counter: "));
      Serial.print(incomingData.counter);
      Serial.print(F(", timing: "));
      Serial.println(incomingData.timing);

      rdata1 = incomingData.data1;
      rdata2 = incomingData.data2;
      rdata3 = incomingData.data3;
      rdata4 = incomingData.data4;
      rdata5 = incomingData.data5;
      reaction = 5;
    }
    else{
      Serial.print(F("Keyword failure: "));
      Serial.println(incomingData.keyword, HEX);
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
  if((immediate)||(currentmilli - sendingTimer > 10000)){
    sendingTimer = currentmilli;
    sendingCounter = updatecounter(sendingCounter); 
    RF24NetworkHeader headerb(pir_node); // (Address where the data is going)
    network_payload outgoing = {keywordval_02, sendingCounter, currentmilli, tdata1, tdata2, tdata3, tdata4, tdata5};
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
