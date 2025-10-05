/*
 * RF-Nano, headers, receiver for bi-stable relays control
 */

#include "RF24.h"
#include <RF24Network.h>
#include <SPI.h>

#include "relays.h"

#define radioChannel 110
#define CE_PIN 10
#define CSN_PIN 9

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t basenode = 00; // Address of the home/host/controller node in Octal format
const uint16_t detectornode = 01; // Address of the detector node in the kitchen
const uint8_t radiolevel = RF24_PA_LOW; // RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 

const unsigned long keywordvalB = 0xbeeffeed; 
const unsigned long keywordvalD = 0xdeeffeeb; 

struct base_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t bvalue;
};

struct detector_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t count;
  uint8_t dvalue;
  uint8_t cvalue;
};

bool newdata = false;
unsigned long currentmilli = 0;
uint8_t detectionValue = 0;
uint8_t commandValue = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) { // some boards need this because of native USB capability
    delay(10);
  }

  // setup for sensors
  setuprelays();

  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.print(F("Basenode: "));
  Serial.println(basenode);
  Serial.flush(); 

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio HW error."));
    while (true) delay(1000);
  }
  radio.setPALevel(radiolevel, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, basenode);
  Serial.print(F("radioChannel: "));
  Serial.print(radioChannel);
  Serial.print(F(", level: "));
  Serial.println(radiolevel);

  Serial.println(F(" ----"));
}

//===== Receiving =====//
bool receiveRFnetwork(unsigned long currentRFmilli){
  static unsigned long receivalTime = 0;
  bool mreceived = false;

  if (network.available()){ // Is there any incoming data?
    receivalTime = currentRFmilli;
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type){
      // Display the incoming millis() values from sensor nodes
      case 'Y': 

        //mreceived = true;
        break;
      // Display the incoming millis() values from sensor nodes
      case 'Z': 
    
        //mreceived = true;
        break;

      case 'K': 
        detector_payload RxData;
        network.read(header, &RxData, sizeof(RxData)); // Read the incoming data
        //if ((header.from_node != detectornode)||(header.type != 'K')) {
        if (header.from_node != detectornode) {
          Serial.print(F("received unexpected message, from_node: "));
          Serial.print(header.from_node);
          Serial.print(F(", type: "));
          Serial.println(header.type);
        }
        if (RxData.keyword == keywordvalD){
          Serial.print(F("Data received from detector/sensors "));
          Serial.print(currentRFmilli);
          mreceived = true;
          // in case a message is received, with specific data, the detector could be 'reset' (for example)
          detectionValue = RxData.dvalue;
          commandValue = RxData.cvalue;
          Serial.print(F(", m # "));
          Serial.print(RxData.count);
          Serial.print(F(", dvalue: "));
          Serial.print(detectionValue);
          Serial.print(F(", cvalue: "));
          Serial.println(commandValue); 

          if (detectionValue == 0xff){
            Serial.print(F("Command received: "));
            switch(commandValue){
              case 85:
                commandaction = RelayState::R_Off;
                Serial.println(F("Kitchen light OFF (B)"));
              break;
              case 90:
                commandaction = RelayState::R_On;
                Serial.println(F("Kitchen light ON (B)"));
              break;
              case 95:
                commandaction = RelayState::R_On;
                Serial.println(F("Kitchen light ON (D)"));
              break;
              default:
                Serial.print(F("Unknown: "));
                Serial.println(commandValue);
            }
          }
        }
        else{
          Serial.println(F("Keyword failure"));
        }
        break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("unknown header.type: "));
        Serial.print(header.type);
        Serial.println(char(header.type));
    }
  }

  // turn off if too long no message received from detector
  if ((unsigned long)(currentRFmilli - receivalTime) > 300000){ // 5 minutes
    receivalTime = currentRFmilli; // just 1 action every 5 minutes
    Serial.print(F("Missing data from detector/sensors "));
    Serial.println(currentRFmilli);
    commandaction = RelayState::R_Off;
    Serial.println(F("Kitchen light OFF (T)"));
    mreceived = true; // method to get data handled as 'fresh'
  }

  return mreceived;
}

//===== Sending =====//
void transmitRFnetwork(bool pfresh){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool fresh = pfresh;
  unsigned long currentRFmilli = millis();
  bool w_ok;

  // Every x seconds, or on new data
  //if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
  //   sendingTimer = currentRFmilli;

  //   base_payload TxData;
  //   TxData.keyword = keywordvalB;
  //   TxData.timing = currentRFmilli;
  //   TxData.count = counter++;
  //   TxData.bvalue = 0xfd;

  //   RF24NetworkHeader header0(detectornode, 'B'); // address where the data is going
  //   w_ok = network.write(header0, &TxData, sizeof(TxData)); // Send the data
  //   Serial.print(F("Message send ")); 
  //   if (w_ok){
  //     failcount = 0;
  //   }    
  //   else{
  //     Serial.print(F("failed "));
  //     failcount++;
  //   }
  //   Serial.print(Txdata.count);
  //   Serial.print(F(", "));
  //   Serial.println(currentRFmilli);

  // }
}

uint8_t relaysstatus = 0;
uint8_t remrelaysstatus = 0;

void loop() {

  network.update();

  currentmilli = millis();

  newdata = receiveRFnetwork(currentmilli);

  //************************ sensors/actuators ****************//

  relaysstatus = relaytracking(newdata);

  //************************ sensors/actuators ****************//

  if (relaysstatus != remrelaysstatus){
    remrelaysstatus = relaysstatus;
    Serial.print(F("Relays @ "));
    Serial.print(millis());
    Serial.print(F(", status: "));
    Serial.println(relaysstatus);
  }

  //transmitRFnetwork(ack);

}
