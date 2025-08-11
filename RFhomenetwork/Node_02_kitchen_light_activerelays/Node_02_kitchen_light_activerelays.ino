/*
 * RF-Nano, PIR sensor(s) and distance sensor as well as off button, 2 active relays
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#define pinPIR1 7        // PIR pin connection
#define pinPIR2 8        // PIR pin connection
#define pinPressButton 2 // light off button
#define pinLight1 4      // relays driver pin
#define pinLight2 5      // relays driver pin

#define radioChannel 104

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t kitchen_node = 02; // Address of this node in the kitchen
const uint16_t base_node = 00;    // Address of the home/host/controller node in Octal format

unsigned long const keywordvalM = 0xfeedbeef; 

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  pinMode(pinPIR1, INPUT);
  pinMode(pinPIR2, INPUT);
  pinMode(pinPressButton, INPUT_PULLUP);
  pinMode(pinLight1, OUTPUT);
  pinMode(pinLight2, OUTPUT);

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, kitchen_node);

  attachInterrupt(digitalPinToInterrupt(pinPressButton), buttonPress, FALLING); // trigger when button is pressed
}

//===== Receiving =====//
bool receiveRFnetwork(unsigned long currentRFmilli){
  bool mreceived = false;

  // Check for incoming data details
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      // Display the incoming millis() values from sensor nodes
      case 'Y': 
        //joystick_payload payload;

        mreceived = true;
        break;
      // Display the incoming millis() values from sensor nodes
      case 'Z': 
        //keypad_payload kpayload;
    
          mreceived = true;

        break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.println(header.type);
    }
  }
  return mreceived;
}

//===== Sending =====//
void transmitRFnetwork(bool fresh, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  //if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
  // if (fresh)
  //   if ((unsigned long)(currentRFmilli - sendingTimer) > 5000){

  //   sendingTimer = currentRFmilli;

  //   network_payload Txdata;
  //   Txdata.keyword = keywordvalM;
  //   Txdata.timing = currentRFmilli;
  //   Txdata.counter = counter++;

  //   RF24NetworkHeader header0(joynode, 'M'); // address where the data is going
  //   w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
  //   if (!w_ok){ // retry
  //     failcount++;
  //     delay(50);
  //     w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
  //   }
  //   Serial.print(F("Message send ")); 
  //   if (w_ok){
  //     failcount = 0;
  //   }    
  //   else{
  //     Serial.print(F("failed "));
  //     failcount++;
  //   }
  //   Serial.print(Txdata.counter);
  //   Serial.print(F(", "));
  //   Serial.println(currentRFmilli);

  // }
}

unsigned long currentmilli = 0;

bool newdata = false;
bool ack = false;
bool activePIR = false;
bool ButtonPressed = false;
bool ButtonActive = false;

void loop() {

  network.update();

  currentmilli = millis();

  newdata = receiveRFnetwork(currentmilli);

  //************************ sensors/actuators ****************//

  if (!activePIR){
    if (digitalRead(pinPIR1) == LOW){
      activePIR = true;
      newdata = true;
    }
  }
  if (!activePIR){
    if (digitalRead(pinPIR2) == LOW){
      activePIR = true;
      newdata = true;
    }
  }
  if (ButtonPressed){
    ButtonActive = true;
    newdata = true;
    ButtonPressed = false;
  } 

  //************************ sensors/actuators ****************//

  if (ButtonActive){ // turn off relays/light(s)
    if (!activePIR){ // no detections
      // turn off the relays

      ButtonActive = false;
    }
  }

  transmitRFnetwork(ack, currentmilli);

}

void buttonPress(){
  if (!ButtonActive){
    ButtonPressed = true;
    Serial.print(F("Button press: "));
    Serial.println(millis());
  }
}
