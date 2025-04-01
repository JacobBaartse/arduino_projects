/*
 * 
 */

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

int connection = 0;

IPAddress printWifiStatus(int connect) {
  // print your board's IP address:
  Serial.print(F("\nIP Address: "));
  IPAddress here = WiFi.localIP();
  Serial.print(here);

  // print the received signal strength:
  Serial.print(F(", signal strength (RSSI): "));
  Serial.print(WiFi.RSSI());
  Serial.print(F(" dBm, con: "));
  Serial.println(connect);
  Serial.flush();
  return here;
}

// void printEncryptionType(int thisType) {
//   // read the encryption type and print out the name:
//   switch (thisType) {
//     case ENC_TYPE_WEP:
//       Serial.println("WEP");
//       break;
//     case ENC_TYPE_WPA:
//       Serial.println("WPA");
//       break;
//     case ENC_TYPE_WPA2:
//       Serial.println("WPA2");
//       break;
//     case ENC_TYPE_WPA3:
//       Serial.print("WPA3");
//       break;   
//     case ENC_TYPE_NONE:
//       Serial.println("None");
//       break;
//     case ENC_TYPE_AUTO:
//       Serial.println("Auto");
//       break;
//     case ENC_TYPE_UNKNOWN:
//     default:
//       Serial.println("Unknown");
//       break;
//   }
// }

String findNetwork() {
  String availSSID = "";
  String foundSSID = "";

  // scan for nearby networks:
  Serial.println(F("** Scan for WiFi **"));
  int numSsid = WiFi.scanNetworks();
  if (numSsid < 1) {
    Serial.println(F("Couldn't get WiFi information"));
    return foundSSID;
  }

  // print the list of networks seen:
  Serial.print(F("Number of available networks: "));
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

  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    availSSID = WiFi.SSID(thisNet);
    for (int storage = 0; knownnetworks[storage][0] != F("EOR"); storage++){
      if (availSSID == knownnetworks[storage][0]){
        if (foundSSID == "") { // collect only the first matching networkID (the strongest???)
          foundSSID = availSSID;
        }
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
    }
  }
  return foundPWD;
}

String getNetworkLocation(String SSID){
  String foundLocation = "";
  for (int storage = 0; knownnetworks[storage][0] != F("EOR"); storage++){
    if (SSID == knownnetworks[storage][0]){
      foundLocation = knownnetworks[storage][2];
    }
  }
  return foundLocation;
}

int WifiConnect(){
  int stat = WL_IDLE_STATUS;

  String SSIDfound = findNetwork();
  if (SSIDfound == ""){
    return stat;
  }
  String SSIDpwd = getNetworkPassword(SSIDfound);
  String SSIDlocation = getNetworkLocation(SSIDfound);
  
  Serial.print(F("Wifi network: "));
  Serial.print(SSIDfound);
  Serial.print(F(", "));
  Serial.println(SSIDlocation);

  // attempt to connect to WiFi network:
  int ssid_len = SSIDfound.length() + 1;
  int pass_len = SSIDpwd.length() + 1;
  char ssid[ssid_len];
  char pass[pass_len];
  SSIDfound.toCharArray(ssid, ssid_len);
  SSIDpwd.toCharArray(pass, pass_len);

  while ((stat != WL_CONNECTED)&&(++connection < 4)){
    stat = initWiFi(ssid, pass, 7000);
  }
  return stat;
}
