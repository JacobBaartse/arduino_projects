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
#define POS_MIN   0 
#define POS_HALF 90 
#define POS_MAX 180 

#define radioChannel 102

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(10, 9); // nRF24L01 (CE, CSN)
RF24Network network(radio); // Include the radio in the network

const uint16_t node00 = 00; // Address of the home/host/controller node in Octal format
uint16_t joynode = 00; // address for the receiving message

unsigned long const keywordvalM = 0xfeedbeef; 
unsigned long const keywordvalJ = 0xbcdffeda;

struct joystick_payload{
  uint32_t keyword;
  uint32_t timing;
  uint8_t xmvalue;
  uint8_t xpvalue;
  uint8_t ymvalue;
  uint8_t ypvalue;
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
unsigned long object1time = 0;
unsigned long object2time = 0;

// data for servo's (pulse length)
uint16_t x1value = 0;
uint16_t y1value = 0;
uint16_t x2value = 0;
uint16_t y2value = 0;

// data from remote control (joystick)
uint8_t xmvalue;
uint8_t xpvalue;
uint8_t ymvalue;
uint8_t ypvalue;
uint8_t mcount;
uint8_t jbvalue;
uint8_t sw1value;
uint8_t sw2value;

//

// /*
//  * angleToPulse(int ang)
//  * gets angle in degree and returns the pulse width
//  * also prints the value on seial monitor
//  * written by Ahmad Nejrabi for Robojax, Robojax.com
//  */
// int angleToPulse(int ang){
//    int pulse = map(ang,0, 180, SERVOMIN,SERVOMAX);// map angle of 0 to 180 to Servo min and Servo max 
//    Serial.print("Angle: ");Serial.print(ang);
//    Serial.print(" pulse: ");Serial.println(pulse);
//    return pulse;
// }

void driveServo(uint8_t servonum, uint8_t pos){
  uint16_t pulselen = map(pos, 0, 180, SERVOMIN, SERVOMAX);
  // uint16_t pulselen2 = angleToPulse(pos);
  // Serial.print(F("Pulse: "));
  // Serial.print(pulselen);
  // Serial.print(F(", "));
  // Serial.println(pulselen2);
  pwm.setPWM(servonum, 0, pulselen);
}

void pauseServo(uint8_t servonum){ // check what this exactly does, does it turn the power off?
  pwm.setPWM(servonum, 0, 0);
}

void driveobject(uint8_t itemnumber){
  uint8_t posx = POS_HALF;
  uint8_t posy = POS_HALF;
  // translate x and y values from network to pos values for servo's
  uint8_t posx1 = map(xmvalue, 0, 255, POS_HALF, POS_MIN);
  uint8_t posx2 = map(xpvalue, 0, 255, POS_HALF, POS_MAX);
  uint8_t posy1 = map(ymvalue, 0, 255, POS_HALF, POS_MIN);
  uint8_t posy2 = map(ypvalue, 0, 255, POS_HALF, POS_MAX);
  
  posx = posx1/2 + posx2/2;
  posy = posy1/2 + posy2/2;

  if (itemnumber == 1){
    driveServo(0, posx); // x coordination
    driveServo(1, posy); // y coordination
  }
  if (itemnumber == 2){
    driveServo(2, posx); // x coordination
    driveServo(3, posy); // y coordination
  } 
}

void checkobjectdrive(unsigned long curtime){
  if (curtime - object1time > 5000){
    pauseServo(0);
    pauseServo(1);
  }
  if (curtime - object2time > 5000){
    pauseServo(2);
    pauseServo(3);
  }
}

void interpretdata(bool fresh, unsigned long curtime){
  // remember the data
  static unsigned long receivedtime = 0;
  static uint8_t itemtomove = 0;

  if (fresh){ // new data received
    if (sw1value > 10)
      itemtomove  = 1;
    if (sw2value > 10)
      itemtomove  = 2;
    if (itemtomove == 1){
      object1time = curtime; 
    }
    if (itemtomove == 2){
      object2time = curtime; 
    }
  }
  if (itemtomove > 0){
    driveobject(itemtomove);
  }
}

//===== Receiving =====//
bool receiveRFnetwork(){
  bool mreceived = false;

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
        joynode = header.from_node;
        Serial.print(header.from_node);
        Serial.print(F(", timing: "));
        Serial.println(payload.timing);
        if (payload.keyword == keywordvalJ) {
          // message received from joystick 
          mcount = payload.count;

          xmvalue = payload.xmvalue;
          ymvalue = payload.ymvalue;
          xpvalue = payload.xpvalue;
          ypvalue = payload.ypvalue;
          jbvalue = payload.bvalue;
          sw1value = payload.sw1value;
          sw2value = payload.sw2value;

          // end of joystick message collection      
          mreceived = true;
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
  return mreceived;
}

//===== Sending =====//
void transmitRFnetwork(bool fresh){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;

  // Every 5 seconds, or on new data
  unsigned long currentRFmilli = millis();
  //if ((fresh)||((unsigned long)(currentRFmilli - sendingTimer) > 5000)){
  if (fresh){
    sendingTimer = currentRFmilli;

    network_payload Txdata;
    Txdata.keyword = keywordvalM;
    Txdata.timing = currentRFmilli;
    Txdata.counter = counter++;

    RF24NetworkHeader header0(joynode, 'M'); // address where the data is going
    w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    if (!w_ok){ // retry
      failcount++;
      delay(50);
      w_ok = network.write(header0, &Txdata, sizeof(Txdata)); // Send the data
    }
    Serial.print(F("Message send ")); 
    if (w_ok){
      failcount = 0;
    }    
    else{
      Serial.print(F("failed "));
      failcount++;
    }
    Serial.print(Txdata.counter);
    Serial.print(F(", "));
    Serial.println(currentRFmilli);

  }
}

bool newdata = false;
bool ack = false;

void loop() {

  network.update();

  currentmilli = millis();

  newdata = receiveRFnetwork();

  //************************ sensors/actuators ****************//

  // depending on received data, driveServo() (one or more commands)
  interpretdata(newdata, currentmilli);

  //************************ sensors/actuators ****************//

  checkobjectdrive(currentmilli);

  transmitRFnetwork(ack);

}
