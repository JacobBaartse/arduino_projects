/*
 * RF-Nano, base node for demo purposes
 */

#include <RF24Network.h>
#include "RF24.h"
#include <SPI.h>

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// https://learn.adafruit.com/16-channel-pwm-servo-driver

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN   90 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  490 // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

#define radioChannel 104

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node00 = 00; // Address of the home/host/controller node in Octal format

unsigned long const keywordvalM = 0xfeedbeef; 
unsigned long const keywordvalS = 0xbeeffeed; 
unsigned long const keywordvalJ = 0xbcdffeda;

struct joystick_payload{
  uint32_t keyword;
  uint32_t timing;
  uint16_t xvalue;
  uint16_t yvalue;
  uint8_t count;
  uint8_t bvalue;
  uint8_t sw1value;
  uint8_t sw2value;
};

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


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_1MBPS);
  network.begin(radioChannel, node00);
}

unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;

//===== Receiving =====//
void receiveRFnetwork(){

  // Check for incoming data details
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      // Display the incoming millis() values from sensor nodes
      case 'J': 
        joystick_payload payload;
        network.read(header, &payload, sizeof(payload));
        Serial.print(F("Received from Joystick nodeId: "));
        Serial.print(header.from_node);
        Serial.print(F(", timing: "));
        Serial.println(payload.timing);
        if (payload.keyword == keywordvalJ) {
          // message received from joystick 




          // end of joystick message processing      
        }
        else{
          Serial.println(F("Wrong Joystick keyword")); 
        }
        break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.println(header.type);
    }
  }
}

//===== Sending =====//
bool transmitRFnetwork(bool fresh){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  unsigned long currentRFmilli = millis();
  if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
    sendingTimer = currentRFmilli;

    // joystick_payload Txdata;
    // Txdata.keyword = keywordvalM;
    // Txdata.timing = currentRFmilli;
    // Txdata.count = counter++;
    // Txdata.xvalue = xValue;
    // Txdata.yvalue = yValue;
    // Txdata.bvalue = bValue;
    // Txdata.sw1value = sw1Value;
    // Txdata.sw2value = sw2Value;

    // Serial.print(F("Message: "));
    // Serial.print(Txdata.count);
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

    // RF24NetworkHeader header0(node00, 'J'); // address where the data is going
    // w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    // if (!w_ok){ // retry
    //   failcount++;
    //   delay(50);
    //   w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    // }
    // Serial.print(F("Message send ")); 
    // if (w_ok){
    //   bValue = 0; 
    //   sw1Value = 0;
    //   sw2Value = 0;
    //   fresh = false;
    //   failcount = 0;
    // }    
    // else{
    //   Serial.print(F("failed "));
    //   failcount++;
    // }
    // Serial.print(Txdata.count);
    // Serial.print(F(", "));
    // Serial.println(currentRFmilli);

    // if (failcount > 10){
    //   fresh = false; // do not send a lot of messages continously
    // }

    // if(!fresh){ // clear buttons status always after 5 seconds
    //   bValue = 0; 
    //   sw1Value = 0;
    //   sw2Value = 0;
    // }

  }

  return fresh;
}

bool driveServo(uint8_t servonum, uint16_t pulselen){

  pwm.setPWM(servonum, 0, pulselen);

}


bool newdata = false;

void loop() {


  network.update();

  currentmilli = millis();

  receiveRFnetwork();

  //************************ sensors/actuators ****************//

  // depending on received data, driveServo() (one or more commands)


  //************************ sensors/actuators ****************//

  newdata = transmitRFnetwork(newdata);

}
