/*
 * Nano + NRF extender module, USB-C, using RF24network library
 */

#include "RF24.h"
#include <RF24Network.h>
#include <SPI.h>

#define radioChannel 98 // dit wordt mogelijk instelbaar
#define CE_PIN 10
#define CSN_PIN 9

/* one button on the collector to disable the alarm(s) as a local acknowledge that the detection is noticed
 * another button to enable the alarming (when a fresh detection is arriving)
 */
#define BUTTON1_PIN 2
#define BUTTON2_PIN 3

#define BUZZER_PIN 8

enum BuzzerState {
    Off,
    On,
    // Low,
    // High,
    // Pulse,
    NoChange
};

BuzzerState buzzerstatus = BuzzerState::Off;

#define LED1_PIN 7
#define LED2_PIN 8

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

uint16_t detectornode = 00; // Address of this node in Octal format (04, 031, etc.)
const uint16_t basenode = 00; // Address of the home/host/controller node in Octal format
uint8_t radiolevel = RF24_PA_MIN;

unsigned long const keywordvalD = 0xdeedbeeb; 

struct detector_payload{
  uint32_t keyword;
  uint32_t timing;
  uint16_t count;
  uint16_t dvalue;
  uint8_t p1value;
  uint8_t p2value;
  uint8_t sw1value;
  uint8_t sw2value;
};

bool newdata = false;
uint8_t p1Value = 0;
uint8_t p2Value = 0;
uint8_t sw1Value = 0;
uint8_t sw2Value = 0;

void setup() {
  Serial.begin(115200);

  // PINs for user input
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as an output
  pinMode(LED1_PIN, OUTPUT);   
  pinMode(LED2_PIN, OUTPUT); 

  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true){ 
      delay(1500);
      //drivebuzzer(BuzzerState::On); // testing some buzzer things
    }
  }
  // RF24_PA_MIN (0), RF24_PA_LOW (1), RF24_PA_HIGH (2), RF24_PA_MAX (3) 
  radiolevel = RF24_PA_LOW;
  radio.setPALevel(radiolevel, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, basenode);
  Serial.print(F("radioChannel: "));
  Serial.print(radioChannel);
  Serial.print(F(", level: "));
  Serial.println(radiolevel);

  attachInterrupt(digitalPinToInterrupt(BUTTON1_PIN), buttonPress1, FALLING); // trigger when button1 pressed
  attachInterrupt(digitalPinToInterrupt(BUTTON2_PIN), buttonPress2, FALLING); // trigger when button2 pressed
}
 
unsigned long currentmilli = 0;
uint16_t detectorscount = 0;
bool pressBUTTON1 = false;
bool pressBUTTON2 = false;
bool activeBUTTON1 = false;
bool activeBUTTON2 = false;

void ledactivity(uint8_t ledid, uint8_t ledaction, unsigned long ledtime){
  static unsigned long ledflashtime = 0;
  static uint8_t ledAction[3][3] = {
    {0, 0, 0}, // LED PIN, status, ledaction
    {0, 0, 0},
    {0, 0, 0},
  };
  bool toggle = false;

  if (ledid == 0){ // run the time
    for(uint8_t id=1;id<3;id++){
      if (ledAction[id][2] == 2){ // flashing
        if ((unsigned long)(ledtime - ledflashtime) > 750){
          uint8_t curstate = ledAction[id][1];
          curstate = curstate == 1 ? 0 : 1;
          ledAction[id][1] = curstate;
          toggle = true;
        }
      }
    }
  }
  else {
    if (ledaction > 1){
      digitalWrite(ledAction[ledid][0], 1); // flashing starts with on
      ledAction[ledid][1] = 1;
    }
    else {
      digitalWrite(ledAction[ledid][0], ledaction);
      ledAction[ledid][1] = ledaction;
    }
    ledAction[ledid][2] = ledaction;
  }
  if (toggle){
    ledflashtime = ledtime;
  }
}

void drivebuzzer(BuzzerState buzzerstatustoset){
  static BuzzerState status = BuzzerState::Off;
  static uint16_t buzzertone = 1000;

  if (buzzerstatustoset != status){
    switch(buzzerstatustoset){
      // case BuzzerState::Dim:


      //   status = BuzzerState::Dim;
      //   break;
      case BuzzerState::On:
        buzzertone += 300;
        if (buzzertone > 6000){
          buzzertone = 1000;
        }
        status = BuzzerState::On;
        break;
      case BuzzerState::NoChange:
        break;
      //case BuzzerState::Off:
      default:
        buzzertone = 2000;
        status = BuzzerState::Off;
    }
  }

  switch(status){
    case BuzzerState::On:
      tone(BUZZER_PIN, buzzertone);
      break;
    case BuzzerState::Off:
      noTone(BUZZER_PIN); 
  }

}

void driveLED(uint8_t lstat, unsigned long currenttiming){
  static uint8_t status = 0;

  if (status != lstat){
    ledactivity(1, lstat, currenttiming);
    status = lstat;
  }
}


unsigned long reportingTime = 0;

uint8_t trackDetectionsAndButtons(unsigned long currentDetectMillis){
  static unsigned long activationTime = 0;
  //static uint8_t reportingdog = 0;
  static bool alarming = false;
  bool buzzer_off = false;
  uint8_t retval = 0;

  if (!alarming){
    if (detectorscount > 0){
      // activate LED and sound
      drivebuzzer(BuzzerState::On);
      driveLED(1, currentDetectMillis);
      alarming = true;
      Serial.print(F("Alarming: "));
      Serial.println(detectorscount);
      activationTime = currentDetectMillis;
    }
  }
  // else {
  //   // turn off LED and sound
  //   drivebuzzer(false);

  // }

  if (alarming){ // show LED and sound (buzzer)
    if (activeBUTTON1){ // button 1 means alarm acknowledged, do not buzz
      activeBUTTON1 = false;
      buzzer_off = true;
    }
    if (activeBUTTON2){ // button 2 means reset for detections
      driveLED(0, currentDetectMillis);
      activeBUTTON2 = false;
      buzzer_off = true;
    }
    if ((unsigned long)(currentDetectMillis - activationTime) > 2000){ // maximum 2 seconds of buzzing
      buzzer_off = true;
      alarming = false;
      detectorscount = 0;
      retval = 0x55;
    }

    if (buzzer_off){
      drivebuzzer(BuzzerState::Off);
    }
    // activate alarm LED

  }

  // // if nothing happened
  // if (false){
  //   // at least print for debugging something to know the software is still running
  //   if ((unsigned long)(currentDetectMillis - reportingTime) > 60000){
  //     Serial.print(F("Running detection tracking: "));
  //     Serial.println(currentDetectMillis);
  //     reportingTime = currentDetectMillis;
  //     reportingdog += 1;
  //   }
  // }
  // else{
  //   reportingdog = 0;
  // }

  return retval;
}

//bool pingreceived = false;

//===== Receiving =====//
uint16_t receiveRFnetwork(unsigned long currentRFmilli){
  static unsigned long receivingTime = 0;
  unsigned long diffTime = 0;
  uint16_t nodereceived = 00;

  while (network.available()){ // Is there any incoming data?
    RF24NetworkHeader header;
    detector_payload Rxdata;
    network.read(header, &Rxdata, sizeof(Rxdata)); // Read the incoming data
    if (header.from_node == basenode){
      Serial.print(F("received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    if (header.type != 'D'){
      Serial.print(F("received unexpected message type: "));
      Serial.println(header.type);
      break;
    }
    nodereceived = header.from_node;
    if (Rxdata.keyword == keywordvalD){
      diffTime = (unsigned long)((currentRFmilli - receivingTime));
      receivingTime = currentRFmilli;
      reportingTime = currentRFmilli;
      Serial.print(F("new data received, time diff: "));
      Serial.print(diffTime);
      Serial.print(F(", dvalue: "));
      Serial.print(Rxdata.dvalue);
      Serial.print(F(", p1: "));
      Serial.print(Rxdata.p1value);
      Serial.print(F(", p2: "));
      Serial.print(Rxdata.p2value);
      Serial.print(F(", sw1: "));
      Serial.print(Rxdata.sw1value);
      Serial.print(F(", sw2: "));
      Serial.println(Rxdata.sw2value);

      //pingreceived = ((Rxdata.dvalue==0xff)&&(Rxdata.sw1value==0xff)&&(Rxdata.sw2value==0xff));

      // if (pingreceived){
      //   Serial.print(nodereceived);
      //   Serial.print(F(" PING received: "));
      // }
      // else{
        if (detectorscount < 0xff00)
          detectorscount += Rxdata.dvalue;
        Serial.print(F("detectorscount: "));
        Serial.print(detectorscount);
        Serial.print(F(", timing: "));
      //}
      Serial.println(currentRFmilli);
    }
    else{
      Serial.println(F("Keyword failure"));
    }
  }
  return nodereceived;
}

//===== Sending =====//
bool transmitRFnetwork(bool fresh, uint16_t node_id, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint16_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  //if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
  if (fresh){
    sendingTimer = currentRFmilli;

    detector_payload Txdata;
    Txdata.keyword = keywordvalD;
    Txdata.timing = currentRFmilli;
    Txdata.count = counter++;
    Txdata.dvalue = 300; // 300 cm detection threshold
    Txdata.p1value = p1Value;
    Txdata.p2value = p2Value;
    Txdata.sw1value = sw1Value;
    Txdata.sw2value = sw2Value;

    // Serial.print(F("Message: "));
    // Serial.print(F(", xvalue: "));
    // Serial.print(Txdata.xvalue);
    // Serial.print(F(", yvalue: "));
    // Serial.print(Txdata.yvalue);
    // Serial.print(F(", bvalue: "));
    // Serial.print(Txdata.bvalue);
    // Serial.print(F(", sw1value: "));
    // Serial.print(Txdata.sw1value);        
    // Serial.print(F(", sw2value: "));
    // Serial.println(Txdata.sw2value);

    RF24NetworkHeader header0(node_id, 'B'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    Serial.print(F("Message send ")); 
    if (w_ok){
      fresh = false;
      failcount = 0;
    }    
    else{
      Serial.print(F("failed "));
      failcount++;
    }
    Serial.print(Txdata.count);
    Serial.print(F(", "));
    Serial.println(currentRFmilli);

    if (failcount > 2){
      fresh = false; // do not send a lot of messages continuously
    }
  }

  return fresh;
}

uint8_t actionval = 0;

void loop() {

  network.update();

  currentmilli = millis();

  //pingreceived = false;
  detectornode = receiveRFnetwork(currentmilli);
  newdata = detectornode > 0; // received a message from a detector

  //************************ sensors ****************//

  if (pressBUTTON1){
    activeBUTTON1 = true;
    pressBUTTON1 = false;
  }  
  if (pressBUTTON2){
    activeBUTTON2 = true;
    pressBUTTON2 = false;
  }  

  //************************ sensors ****************//

  actionval = trackDetectionsAndButtons(currentmilli);

  ledactivity(0, 0, currentmilli); // run the LED activity on all LEDs, for example flashing

  // if (pingreceived){
  //   newdata = true;
  // }
  // possible to send acknowledge to the detector node
  newdata = transmitRFnetwork(newdata, detectornode, currentmilli);

  if (actionval == 0x55){ // signal reset of detections
    newdata = true;
    sw1Value = 0xff;
    sw2Value = 0xff;
    // loop here all detector nodes, detector nodes should be tracked in an array (if > 1)
    for(uint16_t dt=01;dt<02;dt++){ 
      transmitRFnetwork(newdata, dt, currentmilli);
    }
    sw1Value = 0;
    sw2Value = 0;
  }

}

void buttonPress1(){
  if (!activeBUTTON1){
    pressBUTTON1 = true;
    Serial.print(F("Button press 1: "));
    Serial.println(millis());
  }
}

void buttonPress2(){
  if (!activeBUTTON2){
    pressBUTTON2 = true;
    Serial.print(F("Button press 2: "));
    Serial.println(millis());
  }
}
