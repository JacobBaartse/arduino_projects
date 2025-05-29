/*
 * UNO R4 Wifi, configured in as an Access Point
 */

#include "matrix.h"
#include "networking.h"
#include "screen.h"
#include "webinterface.h"

int status = WL_IDLE_STATUS;

IPAddress IPhere;

char ssid[] = "UNO_R4_AP_Living"; // your network SSID (name)
char pass[] = "TBD PW for UNO_R4_AP_RF"; // your network password

uint8_t WiFichannel = 13; // WiFi channel (1-13), 6 seems default

// void restart_arduino(){
//   Serial.println(F("Restart the arduino UNO board..."));
//   delay(2000);
//   NVIC_SystemReset();
// }

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  delay(1000);
  Serial.println(F("Starting UNO R4 WiFi"));
  Serial.println();
  Serial.print(__FILE__);
  Serial.print(F("\n, creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 
  
  //RTC.begin();
  matrix.begin();

  setupRFnetwork();

  setupScreenControl();

  // print the network name (SSID);
  Serial.print(F("Creating access point named: "));
  Serial.print(ssid);
  Serial.print(F(", password: '"));
  Serial.print(pass);
  Serial.println(F("'"));

  // by default the local IP address will be 192.168.4.1
  // you can override it with the following:
  WiFi.config(IPAddress(192,168,14,4));
  // Create open network. Change this line if you want to create an WEP network:
  status = WiFi.beginAP(ssid, WiFichannel); // no password needed
  //status = WiFi.beginAP(ssid, pass, WiFichannel); // password required
  if (status != WL_AP_LISTENING) {
    Serial.println(F("Creating access point failed"));
    // don't continue
    while (true) delay(1000);
  }

  // wait 10 seconds for connection:
  delay(10000);
  // start the web server on port 80
  server.begin();
  // you're connected now, so print out the status
  IPhere = printWiFiStatus();

  Serial.println(F("Created access point available"));

  startupscrollingtext(String("-->: ") + IPhere.toString());

  Serial.println(F("\n ***************\n"));  
  Serial.flush(); 
}
 
unsigned long currentMillis = 0; // stores the value of millis() in each iteration of loop()
unsigned int receiveaction = 0;
unsigned int transmitaction = 0;
bool screening = false;

void loop() {

  currentMillis = millis();

  network.update();
 
  //===== Receiving =====//
  receiveaction = receiveRFnetwork(currentMillis);
  bool receivedfresh = receiveaction > 0;

  //===== Sending =====//
  transmitaction = transmitRFnetwork(currentMillis, receivedfresh);

  webinterfacing();

  if (screening){
    screening = screenprocessing(currentMillis);
  }

}

