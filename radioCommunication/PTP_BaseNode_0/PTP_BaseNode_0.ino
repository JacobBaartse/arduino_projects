/*
  Arduino Wireless Network - Multiple NRF24L01 Tutorial
          == Base/Master Node 00 ==
  by Dejan, www.HowToMechatronics.com
  Libraries:
  nRF24/RF24, https://github.com/nRF24/RF24
  nRF24/RF24Network, https://github.com/nRF24/RF24Network


  https://howtomechatronics.com/tutorials/arduino/how-to-build-an-arduino-wireless-network-with-multiple-nrf24l01-modules/#h-base-00-source-code

Target: UNO R4 Wifi, with RF24 module Long Range
nRF24L01 (CE,CSN) connected to pin 8, 7
location SO148
*/

#include "matrix.h";
#include "networking.h";
// #include "WiFiS3.h"; // already included in networking.h
#include "RTC.h";
#include <NTPClient.h>;
#include <SPI.h>;

WiFiServer server(80);
IPAddress IPhere;

WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
NTPClient timeClient(Udp);

void setup() {
  Serial.begin(115200);
  RTC.begin();
  matrix.begin();
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MIN, 0); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  network.begin(70, this_node); // (channel, node address)
  radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);

  Serial.print(F("Starting up UNO R4 WiFi"));
  Serial.flush();

  String timestamp = __TIMESTAMP__;
  Serial.print(F("Creation/build time: "));
  Serial.println(timestamp);
  Serial.flush(); 

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION){
    Serial.println("Please upgrade the firmware for the WiFi module");
  }

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

  Serial.println(F("\nStarting connection to NTP server..."));
  timeClient.begin();
  unsigned long unixTime = 0;
  const unsigned long unixmintime = 1729331495; // 19 October 2024
  const unsigned long unixmaxtime = 2076393095; // 19 October 2035
  while ((unixTime < unixmintime)||(unixTime > unixmaxtime)) {
    delay(1000);
    Serial.println(F("\nRetrieving UTC time"));
    timeClient.update();
    // Get the current date and time from an NTP server
    unixTime = timeClient.getEpochTime();
  }
  // convert it to UTC +2 by passing the time zone offset in hours.
  // You may change the time zone offset to your local one.  auto timeZoneOffsetHours = 2;
  //unixTime = unixTime + (timeZoneOffsetHours * 3600);
  Serial.print(F("Unix time = "));
  Serial.println(unixTime);
  RTCTime timeToSet = RTCTime(unixTime);
  RTC.setTime(timeToSet);
  // Retrieve the date and time from the RTC and print them
  RTCTime currentTime;
  RTC.getTime(currentTime); 
  Serial.println(F("The RTC was just set to: "));
  Serial.println(currentTime);

  Serial.println(F(" "));  
  Serial.println(F(" *************** "));  
  Serial.println(F(" "));  
  Serial.flush(); 
}

// void restart_arduino(){
//   Serial.println("Restart the Arduino UNO board...");
//   delay(2000);
//   NVIC_SystemReset();
// }

bool messageStatus(unsigned long interval)
{
  static unsigned long statustime = 0;
  static unsigned long statussequence = 0;
  if (millis() < statustime) return false;
  statustime = millis() + interval;
  statussequence++;
  Serial.print(F("Item: "));
  Serial.print(statussequence);
  Serial.print(F(", network messages "));
  Serial.print(F("received: "));
  Serial.print(receivedmsg);
  Serial.print(F(", send: "));
  Serial.print(sendmsg);
  Serial.print(F(", missed: "));
  Serial.print(droppedmsg);
  Serial.print(F(", failed: "));
  Serial.print(failedmsg);
  Serial.println(F(" "));  
  // if (statustime > 0xf0000000){
  //   restart_arduino();
  // }
  return true;
}

bool alarming = true; // should become: false;
unsigned int receiveaction = 0;
unsigned int transmitaction = 0;

void loop() {
  network.update();

  if (messageStatus(60000)) { // request remote status when local status is printed
    commanding = command_status;
  }

  //===== Receiving =====//
  receiveaction = receiveRFnetwork();

  //===== Sending =====//
  transmitaction = transmitRFnetwork(commanding);

  // show something on the LED matrix 
  if (alarming) {
    alarming = alarmingsequence();
  }
  else {
    loadsequencepicture();
  }
}
