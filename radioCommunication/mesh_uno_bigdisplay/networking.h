/*
 * 
 */

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
// for the NANO with onboard RF24 module:
// RF24 radio(10, 9); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external RF24 module:
RF24 radio(8, 7); // nRF24L01 (CE, CSN)

const byte slaveAddress[5] = {'R','x','A','B','C'};

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

// struct payload_from_slave {
//   unsigned long keyword;
//   uint32_t timing;
//   uint8_t nodeId;
//   uint8_t detection;
//   uint8_t distance;
// };


// https://forum.arduino.cc/t/simple-nrf24l01-2-4ghz-transceiver-demo/405123/3

char dataToSend[10] = "Message 0";
char txNum = '0';
char ackData[6] = "-----";

void updateMessage() {
  // so you can see that new data is being sent
  txNum += 1;
  if (txNum > '9') {
    txNum = '0';
  }
  dataToSend[8] = txNum;
}

//===== Sending =====//
void transmitRFnetwork(){
  static unsigned long sendingTimer = 0;

  // Every x seconds...
  unsigned long currentmilli = millis();
  if(currentmilli - sendingTimer > 5000){
    sendingTimer = currentmilli;

    // Always use sizeof() as it gives the size as the number of bytes.
    if (radio.write(&dataToSend, sizeof(dataToSend))) {
      if (radio.isAckPayloadAvailable()) {
        radio.read(&ackData, sizeof(ackData));
        //newData = true;
        Serial.print(F("< Data returned: "));
        Serial.println(ackData);
        Serial.print(F("> Data Sent: "));
        Serial.println(dataToSend);
        Serial.println();
      }
      else {
        Serial.println(F("Acknowledge, no data"));
      }
      updateMessage();
    }
    else {
      Serial.println(F("Tx failed"));
    }
  }
}

