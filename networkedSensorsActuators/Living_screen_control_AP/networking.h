/*
 * 
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
  Serial.print("SSID: ");
  Serial.print(WiFi.SSID());

  // print your AP IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(", IP address: ");
  Serial.print(ip);

  // print where to go in a browser:
  Serial.print(", browse to http://");
  Serial.println(ip);
  
  return ip;
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
  Serial.print("number of available networks: ");
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

#define radio_channel 104

/**** Configure the nrf24l01 CE and CSN pins ****/
// for the UNO/NANO with external RF24 module:
RF24 radio(7, 8); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t base_node = 00;   // Address of this node in Octal format (04, 031, etc.)
const uint16_t kitchen_node = 01;

void setupRFnetwork(){
  SPI.begin();

  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }  
  //radio.setPALevel(RF24_PA_MIN, false); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
  radio.setDataRate(RF24_1MBPS); // (RF24_2MBPS);

  network.begin(radio_channel, base_node); // (channel, node address)
}

// Payload for kitchen
struct kitchen_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t light; // 0 - no change, 100 - ON, 200 - OFF
};


//===== Receiving =====//
unsigned int receiveRFnetwork(unsigned long currentmilli){
  unsigned int reaction = 0;

  // Check for incoming data from the sensors
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      case 'K': // Message received from HomeController for RemoteNode
        Serial.print(F("Message received from Base: "));
        kitchen_payload kpayload;
        network.read(header, &kpayload, sizeof(kpayload));
      break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.print(header.type);
    }
    Serial.println(currentmilli);
  } // end of while network.available

  return reaction;
}

//===== Sending =====//
unsigned int transmitRFnetwork(unsigned long currentmilli, bool fresh){
  static unsigned long sendingTimer = 0;
  unsigned int traction = 0;
  bool ok = false;

  // Every x seconds...
  if((fresh)||(currentmilli - sendingTimer > 5000)){
    sendingTimer = currentmilli;
    kitchen_payload kpayload;
    kpayload.keyword = 0;
    kpayload.count = 0;
    kpayload.light = 0;
    kpayload.timing = currentmilli;
    RF24NetworkHeader headerK(kitchen_node, 'K'); // Address where the data is going
    ok = network.write(headerK, &kpayload, sizeof(kpayload)); // send the data
    if (!ok) {
      //Serial.print(F("Retry sending message: "));
      //Serial.println(sendingCounter);      
      ok = network.write(headerK, &kpayload, sizeof(kpayload)); // retry once
    }

    Serial.print(currentmilli);
    Serial.print(F(" send message "));
    if (ok) {
      Serial.println(F("OK "));
    }
    else{
      Serial.println(F("Failed "));
    }
    //Serial.println(sendingCounter);
  }
  return traction;
}
