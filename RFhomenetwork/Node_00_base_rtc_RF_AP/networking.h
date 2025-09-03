/*
 * Wifi and RF24 functions
 */

#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

#include "WiFiS3.h"
#include "networkdata.h"

int initWiFi(char* pssid, char* ppass, int timeout=10000) {
  unsigned long endTime = millis() + timeout;
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

IPAddress printWiFiStatus() {
  // print the SSID of the network you're hosting (Access Point mode)
  Serial.print(F("SSID: "));
  Serial.print(WiFi.SSID());

  // print your AP IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F(", IP address: "));
  Serial.print(ip);

  // print where to go in a browser:
  Serial.print(F(", browse to http://"));
  Serial.println(ip);
  
  return ip;
}

unsigned int connection = 0;

IPAddress printWifiStatus(int connect) {
  Serial.println();
  Serial.println();
  // print your board's IP address:
  Serial.print(F("IP Address: "));
  IPAddress here = WiFi.localIP();
  Serial.print(here);

  // print the received signal strength:
  Serial.print(F(", signal strength (RSSI): "));
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
  Serial.println(F("** Scan Networks **"));
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println(F("Couldn't get WiFi information"));
    while (true);
  }

  // print the list of networks seen:
  Serial.print(F("number of available networks: "));
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(F(") "));
    Serial.print(WiFi.SSID(thisNet));
    Serial.print(F(" Signal: "));
    Serial.print(WiFi.RSSI(thisNet));
    Serial.println(F(" dBm"));
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
  Serial.print(F("foundSSID: "));
  Serial.println(foundSSID);
  return foundSSID;
}

String getNetworkPassword(String SSID) {
  String foundPWD = "";
  for (int storage = 0; knownnetworks[storage][0] != F("EOR"); storage++){
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
  
  Serial.print(F("Wifi network: "));
  Serial.print(SSIDfound);
  Serial.print(F(", "));
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

#define radio_channel 104
#define CE_PIN 9
#define CSN_PIN 10

const uint32_t kitchenkeyword = 0x10112003;
const uint32_t shedkeyword = 0xffddeecc;
const uint32_t spoorstrakeyword = 0xfdecba98;
const uint32_t keypadkeyword = 0xabcdef01;


/**** Configure the nrf24l01 CE and CSN pins ****/
// for the UNO/NANO with external RF24 module:
//RF24 radio(7, 8); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external SMD RF24 module:
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t base_node = 00;   // Address of this node in Octal format (04, 031, etc.)
const uint16_t shed_node = 01;
const uint16_t spoorstra_node = 011; // connect via shed_node
const uint16_t kitchen_node = 02;
const uint16_t keypad_node = 012; // connect via kitchen_node

void setupRFnetwork(){
  SPI.begin();

  Serial.print(F("CE_PIN: "));
  Serial.print(CE_PIN);
  Serial.print(F(", CSN_PIN: "));
  Serial.println(CSN_PIN);

  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }  
  //radio.setPALevel(RF24_PA_MIN, false); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
  //radio.setPALevel(RF24_PA_MAX); // when in use
  radio.setDataRate(RF24_1MBPS); // (RF24_2MBPS);

  network.begin(radio_channel, base_node); // (channel, node address)
}

// // Payload for kitchen (from base)
// struct kitchen_payload{
//   uint32_t keyword;
//   uint32_t timing;
//   uint8_t count;
//   uint8_t light; // 0 - no change, 100 - ON, 200 - OFF
//   //uint8_t dummy1; 
//   //uint8_t dummy2; 
// };

// // Payload for shed (from base)
// struct shed_payload{
//   uint32_t keyword;
//   uint32_t timing;
//   uint8_t count;
//   uint8_t light; // 0 - no change, 100 - ON, 200 - OFF
//   //uint8_t dummy1; 
//   //uint8_t dummy2; 
// };

// Payload from base (to shed or kitchen)
struct ks_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t light; // 0 - no change, 100 - ON, 200 - OFF
};

// Payload for base (from shed or kitchen)
struct base_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t light;
  uint8_t pirs;
  uint8_t distance;
};

struct webtoRF{
  uint16_t destination;
  uint8_t light;
};

char nodetokey(uint16_t nodeaddress){
  char found = 'X';
  switch(nodeaddress){
    case shed_node:
      found = 'S';
    break;
    case kitchen_node:
      found = 'K';
    break;
    case spoorstra_node:
      found = 'P';
    break;
    // case keypad_node:
    //   found = 'Z';
    // break;
    default:
      found = 'D';
  }
  return found;
}

//===== Receiving =====//
unsigned int receiveRFnetwork(unsigned long currentmilli){
  static unsigned long kcounter = 0;
  static unsigned long scounter = 0;
  static unsigned long pcounter = 0;
  static unsigned long zcounter = 0;
  unsigned int reaction = 0;

  // Check for incoming data from the sensors
  while (network.available()) {
    RF24NetworkHeader header;
    base_payload rpayload;
    uint32_t expect_keyword = 0;

    network.peek(header);
    Serial.print(currentmilli);
    Serial.print(F(" Message received from "));
    switch(header.type){
      case 'K': // Message received from Keypad for Livingroom
        Serial.print(F("Keypad: "));
        expect_keyword = keypadkeyword;
        Serial.println(++pcounter);
      break;   
      case 'L': // Message received from Kitchen for Livingroom
        Serial.print(F("Kitchen: "));
        expect_keyword = kitchenkeyword;
        Serial.println(++kcounter);
      break;
      case 'B': // Message received from Shed for Livingroom
        Serial.print(F("Shed: "));
        expect_keyword = shedkeyword;
        Serial.println(++scounter);
      break;
      case 'Z': // Message received from Spoorstra for Livingroom
        Serial.print(F("Spoorstra: "));
        expect_keyword = spoorstrakeyword;
        Serial.println(++zcounter);
      break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.println(header.type);
    }
    if (expect_keyword > 0){ // known message type
      network.read(header, &rpayload, sizeof(rpayload));
      if (expect_keyword == rpayload.keyword){ // valid message received
        reaction = 3; // directly send 'fresh' response

        // switch(header.type){
        // }

      }
      else{
        Serial.print(F("Found unexpected keyword, expected: "));
        Serial.print(expect_keyword);
        Serial.print(F(", received keyword: "));
        Serial.println(rpayload.keyword);
      }
    }
  } // end of while network.available

  return reaction;
}

const char mestypes[] = {'K', 'D', 'S', 'D'}; // node list, D means Default, no node, small pause for network
const uint8_t mestypesamount = 4;

//===== Sending =====//
unsigned int transmitRFnetwork(unsigned long currentmilli, bool fresh, uint16_t ldest, uint8_t lstatus){ //}, webtoRF RFdata){
  static unsigned long sendingTimer = 0;
  static uint8_t mcounter = 0;
  static uint8_t messageindex = 0;
  unsigned int traction = 0;
  bool ok = false;
  bool retry = true;
  bool lfresh = fresh;
  uint16_t mnode = 0;

  if (lstatus > 0){
    lfresh = true;
  }

  // Every x seconds...
  if((lfresh)||(currentmilli > sendingTimer)){
    sendingTimer = currentmilli + 10000; // once per 10 seconds, cycle the connected nodes
    ks_payload mpayload;
    char datakey = 'X';

    Serial.print(currentmilli);
    if (lstatus > 0){
      datakey = nodetokey(ldest);
      Serial.print(F(" light: "));
      Serial.print(lstatus);
      //mpayload.light = lstatus;
    }
    else {
      messageindex = (messageindex + 1) % mestypesamount; // cycle the nodes
      datakey = mestypes[messageindex];      
    }
    mpayload.light = lstatus;

    switch(datakey){
      case 'K': {
        mnode = kitchen_node;
        mpayload.keyword = kitchenkeyword;
      }
      break;
      case 'S': {
        mnode = shed_node;
        mpayload.keyword = shedkeyword;
      }
      break;
      case 'P': {
        mnode = spoorstra_node;
        mpayload.keyword = spoorstrakeyword;
      }
      break;
      case 'D': // default, skip a time
      default: {
        retry = false;
        Serial.print(F(" pause "));
      }
    }

    if (retry){ // if message is to be send
      RF24NetworkHeader mheader(mnode, datakey); // Address where the data is going
      mpayload.timing = currentmilli;
      mpayload.count = mcounter++;
      ok = network.write(mheader, &mpayload, sizeof(mpayload)); // send the data
      if (!ok) {
        Serial.print(F(" retry,"));
        ok = network.write(mheader, &mpayload, sizeof(mpayload)); // send the data
      }
      Serial.print(F(" send message "));
      if (ok) {
        Serial.print(F("OK: "));
      }
      else{
        Serial.print(F("Failed: "));
      }
    }
    Serial.println(datakey);
  }

  return traction;
}
