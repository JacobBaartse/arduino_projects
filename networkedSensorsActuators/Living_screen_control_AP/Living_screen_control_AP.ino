/*
 * UNO R4 Wifi, configured in as an Access Point
 */

#include "matrix.h"
#include "networking.h"
#include "screen.h"
#include "sdisplay.h"
#include "webinterface.h"
#include "temppress.h"

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

  temppress_setup();

  sdisplay_setup();

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

  startupscrollingtext(String(F("-->: ")) + IPhere.toString());

  Serial.println(F("\n ***************\n"));  
  Serial.flush(); 

  display_oled(true, 0, yline1, Line1);
  display_oled(false, 0, yline3, IPhere.toString());  
}
 
unsigned long currentMillis = 0; // stores the value of millis() in each iteration of loop()
unsigned int receiveaction = 0;
unsigned int transmitaction = 0;
bool screening = false;
int wcommand = 0;
String wcommandtext = " ";
bool receivedfresh = false;

void loop() {

  currentMillis = millis();

  network.update();
 
  //===== Receiving =====//
  receiveaction = receiveRFnetwork(currentMillis);
  if (receiveaction > 0){
    receivedfresh = true; // send a response directly
  }

  //===== Sending =====//
  transmitaction = transmitRFnetwork(currentMillis, receivedfresh);
  receivedfresh = false;

  wcommand = webinterfacing();
  if (wcommand > 0){
    receivedfresh = true; // send a command directly
    switch(wcommand){
    case 1:{
      wcommandtext = "Keuken aan";        
    } 
    break;
    case 2:{      
      wcommandtext = "Keuken uit";              
    } 
    break;
    case 4:{
      wcommandtext = "Schuur aan";              
    } 
    break;
    case 8:{
      wcommandtext = "Schuur uit";              
    } 
    break;  
    case 0:
    default: 
      wcommandtext = " ERROR ";        
    }
    display_oled(true, 0, yline2, wcommandtext); 
    Serial.print(F("WebCommand: "));  
    Serial.print(wcommand);  
    Serial.print(F(" "));  
    Serial.println(wcommandtext);  
    // wcommand = 0;
  }

  if (screening){
    screening = screenprocessing(currentMillis);
  }

  tempress_values(currentMillis);

}

