/*
 * RF-Nano, headers, transmitter for PIR/distance sensor/button
 */

// #include "RF24.h"
// #include <RF24Network.h>
// #include <SPI.h>

// #define radioChannel 111
// #define CE_PIN 10
// #define CSN_PIN 9

// one button or more buttons can be connected to trigger human presence manually
#define BUTTON_PIN 2
#define BUZZER_PIN 4
#define PIR_PIN 6

// /**** Configure the nrf24l01 CE and CSN pins ****/
// RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)
// RF24Network network(radio); // Include the radio in the network

// const uint16_t buzzernode = 02; // Address of this node in Octal format (04, 031, etc.)
// const uint16_t basenode = 00; // Address of the home/host/controller node in Octal format
// const uint8_t radiolevel = RF24_PA_LOW; // RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 

// const unsigned long keywordvalB = 0xbeeffeed; 
// const unsigned long keywordvalD = 0xdeeffeeb; 

// struct base_payload{
//   uint32_t keyword;
//   uint32_t timing;
//   uint8_t count;
//   uint8_t bvalue;
// };

// struct detector_payload{
//   uint32_t keyword;
//   uint32_t timing;
//   uint8_t count;
//   uint8_t dvalue;
//   uint8_t cvalue;
// };

bool newdata = false;
unsigned long currentmilli = 0;
// uint8_t detectionValue = 0;
// uint8_t commandValue = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { // some boards need this because of native USB capability
    delay(10);
  }

  // setup for sensors
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  // SPI.begin();
  // if (!radio.begin()){
  //   Serial.println(F("Radio HW error."));
  //   while (true) delay(1000);
  // }
  // radio.setPALevel(radiolevel, 0);
  // radio.setDataRate(RF24_1MBPS);
  // network.begin(radioChannel, detectornode);
  // Serial.print(F("radioChannel: "));
  // Serial.print(radioChannel);
  // Serial.print(F(", level: "));
  // Serial.println(radiolevel);

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPress, FALLING); // trigger when button is pressed

  Serial.println(F(" ----"));
}

bool activePIR = false;
bool activeBUTTON = false;

// //===== Receiving =====//
// bool receiveRFnetwork(){
//   // unsigned long currentRFmilli = millis();
//   bool mreceived = false;

//   if (network.available()){ // Is there any incoming data?
//     RF24NetworkHeader header;
//     network.peek(header);
//     if ((header.from_node != basenode)||(header.type != 'B')) {
//       Serial.print(F("Received unexpected message, from_node: "));
//       Serial.print(header.from_node);
//       Serial.print(F(", type: "));
//       Serial.print(header.type);
//       Serial.println(char(header.type));
//     }
//     else{
//       base_payload Rxdata;
//       network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
//       if (Rxdata.keyword == keywordvalB){
//         Serial.print(F("Data received from base/collector "));
//         Serial.println(millis());
//         mreceived = true;
//         // in case a message is received, with specific data, the detector could be 'reset' (for example)


//       }
//       else{
//         Serial.println(F("Keyword failure"));
//       }
//     }
//   }

//   return mreceived;
// }

// //===== Sending =====//
// bool transmitRFnetwork(bool pfresh){
//   static unsigned long sendingTimer = 0;
//   static uint8_t counter = 0;
//   static uint8_t failcount = 0;
//   bool fresh = pfresh;
//   unsigned long currentRFmilli = millis();
//   bool w_ok;

//   // Every x seconds, or on new data
//   if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 60000)){
//     sendingTimer = currentRFmilli;

//     detector_payload Txdata;
//     Txdata.keyword = keywordvalD;
//     Txdata.timing = currentRFmilli;
//     Txdata.count = counter++;
//     Txdata.dvalue = detectionValue;
//     Txdata.cvalue = commandValue;

//     Serial.print(F("Message dvalue: "));
//     Serial.print(Txdata.dvalue);
//     Serial.print(F(", cvalue: "));
//     Serial.print(Txdata.cvalue);        

//     RF24NetworkHeader header0(basenode, 'K'); // address where the data is going
//     w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
//     Serial.print(F(", send ")); 
//     if (w_ok){
//       fresh = false;
//       failcount = 0;
//     }    
//     else{
//       Serial.print(F("failed "));
//       failcount++;
//     }
//     Serial.print(F("m # "));
//     Serial.print(Txdata.count);
//     Serial.print(F(", "));
//     Serial.println(currentRFmilli);

//     if (failcount > 4){
//       fresh = false; // do not send a lot of messages continously
//     }
//   }

//   return fresh;
// }

bool pressBUTTON = false;
uint8_t remPIR1 = 3;
uint8_t curPIR1 = 3;
unsigned long difPIR = 3;
unsigned long difPIRtime1 = 0;
unsigned long buttontime = 0;

void loop() {

  //network.update();

  currentmilli = millis();

  //newdata = receiveRFnetwork(currentmilli);

  //************************ sensors ****************//
  newdata = false;

  curPIR1 = digitalRead(PIR_PIN);
  if (curPIR1 != remPIR1){
    difPIR = (unsigned long)(currentmilli - difPIRtime1);
    Serial.print(currentmilli);
    Serial.print(F(" PIR 1 change "));
    Serial.print(difPIR);
    Serial.print(F(" to "));
    Serial.println(curPIR1);
    remPIR1 = curPIR1;
    difPIRtime1 = currentmilli;

    // if change in PIR, refresh globals
    activePIR = curPIR1 == HIGH;
  }

  if (pressBUTTON){
    pressBUTTON = false;
    newdata = true;
    buttontime = currentmilli;
    activeBUTTON = true;
  } 

  if (((unsigned long)(currentmilli - buttontime) > 10000)){
    activeBUTTON = false;
    buttontime = currentmilli;
  }

  //************************ sensors ****************//

  //newdata = trackSensors(newdata, currentmilli);
  
  //transmitRFnetwork(newdata);

}

void buttonPress(){
  if (!activeBUTTON){
    pressBUTTON = true;
    Serial.print(F("Button press: "));
    Serial.println(millis());
  }
  else {
    Serial.println(F("Button already activated! "));
  }
}
