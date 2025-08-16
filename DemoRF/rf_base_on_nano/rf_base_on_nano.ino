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
uint16_t joynode = 00; // address for the receiving message from joystick
uint16_t keynode = 00; // address for the receiving message from keypad
uint16_t tmnode = 00; // address for the receiving message from TM1638

unsigned long const keywordvalM = 0xfeedbeef; 
unsigned long const keywordvalJ = 0xbcdffeda;
unsigned long const keywordvalK = 0xbeeffeed; 
unsigned long const keywordvalT = 0x12345678; 

struct joystick_payload{
  uint32_t keyword;
  uint32_t timing;
  uint16_t xrvalue;
  uint16_t xvalue;
  uint16_t yrvalue;
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

struct keypad_payload{
  uint32_t keyword;
  uint32_t timing;
  uint32_t count;
  char keys[11];
};

struct tm_payload{
  uint32_t keyword;
  uint32_t timing;
  uint32_t counter;
  uint8_t buttons;
  bool SW[8];
};

struct tm_ack_payload{
  uint32_t keyword;
  uint32_t timing;
  uint32_t counter;
  uint8_t leds;
  uint8_t TXT[8];
};

const uint8_t maxkeys = 10;
char keytracking[11]; // 10 characters + room for the null terminator
//uint8_t keyindex = 0;

void clearkeypadcache(){
  for (int i=0;i<=maxkeys;i++){
    keytracking[i] = 0; // place null character
  }
  //keyindex = 0;
}

// TM1638
uint8_t ackbuttonvalue = 0;
uint8_t acktxtvalues[8];


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
uint16_t xvalue;
uint16_t xrvalue;
uint16_t yvalue;
uint16_t yrvalue;
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

uint8_t memPos[][3] = {
  // x, y, x neutral pos (has to be tuned)
  { 90, 90, 90 }, // not used
  { 90, 90, 90 }, // item 1
  { 90, 90, 90 }, // item 2
}; 

void driveobject(uint8_t itemnumber, unsigned long movetime){
  static unsigned long movingTimer = 0;
  uint8_t posx = memPos[itemnumber][0];
  uint8_t posy = memPos[itemnumber][1];
  uint8_t refx = memPos[itemnumber][2];
  bool fresh = false;

  // for x is a 360 rotating servo, speed should be restricted
  // posx = map(xvalue, 0, (xrvalue * 2), 0, 180);
  if (xvalue < xrvalue){ // turning left
    posx = map(xvalue, 0, xrvalue, (refx - 10), refx); // pos range reduced to 10 (from 90)
  }
  else if (xvalue > xrvalue){ // turning right
    posx = map(xvalue, xrvalue, 1023, refx, (refx + 10)); // pos range reduced to 10 (from 90)
  }
  else posx = refx; // neutral, not moving

  // foy, directly map joystick to position
  posy = map(yvalue, 0, (yrvalue * 2), 0, 180);

  if (fresh ||(posx != memPos[itemnumber][0])||(posy != memPos[itemnumber][1])){
    Serial.print(F("Pos X: "));
    Serial.print(posx);
    Serial.print(F(", pos Y: "));
    Serial.print(posy);
    Serial.print(F(", object: "));
    Serial.println(itemnumber);

    memPos[itemnumber][0] = posx;
    memPos[itemnumber][1] = posy;

    if (itemnumber == 1){
      driveServo(0, posx); // x coordination
      driveServo(1, posy); // y coordination
      //Serial.println(F("item 1"));
    }
    if (itemnumber == 2){
      driveServo(2, posx); // x coordination
      driveServo(3, posy); // y coordination
      //Serial.println(F("item 2"));
    } 
  }
}

void interpretdata(bool fresh, unsigned long curtime){
  // remember the data
  static unsigned long receivedtime = 0;
  static uint8_t itemtomove = 0;

  if (fresh){ // new data received
    if (jbvalue > 10)
      itemtomove = 0;
    if (sw1value > 10)
      itemtomove = 1;
    if (sw2value > 10)
      itemtomove = 2;
    if (itemtomove == 1){
      object1time = curtime; 
    }
    if (itemtomove == 2){
      object2time = curtime; 
    }
  }
  if (itemtomove > 0){
    driveobject(itemtomove, curtime);
  }
}

unsigned char RFacktype = '0';

//===== Receiving =====//
bool receiveRFnetwork(unsigned long currentRFmilli){
  uint8_t mreceived = 0;
  RFacktype = '0';

  // Check for incoming data details
  while (network.available()) {
    RF24NetworkHeader header;
    network.peek(header);
  
    switch(header.type) {
      // Handle Joystick input
      case 'J': 
        joystick_payload payload;
        network.read(header, &payload, sizeof(payload));
        // Serial.print(F("Received from Joystick nodeId: "));
        joynode = header.from_node;
        // Serial.print(header.from_node);
        // Serial.print(F(", timing: "));
        // Serial.println(payload.timing);
        if (payload.keyword == keywordvalJ) {
          // message received from joystick 
          mcount = payload.count;

          xvalue = payload.xvalue;
          xrvalue = payload.xrvalue;
          yvalue = payload.yvalue;
          yrvalue = payload.yrvalue;

          jbvalue = payload.bvalue;
          sw1value = payload.sw1value;
          sw2value = payload.sw2value;

          // end of joystick message collection      
          mreceived += 1;
        }
        else{
          Serial.println(F("Wrong Joystick keyword")); 
        }
        break;
      // Handle Keypad input
      // case 'K': 
      //   bool keyfollowup = false;
      //   keypad_payload kpayload;
      //   network.read(header, &kpayload, sizeof(kpayload));
      //   Serial.print(F("Received from Keypad nodeId: "));
      //   keynode = header.from_node;
      //   Serial.print(header.from_node);
      //   Serial.print(F(", timing: "));
      //   Serial.println(kpayload.timing);
      //   if (kpayload.keyword == keywordvalK) {
      //     // message received from keypad
      //     Serial.print(F("Key(s): '"));        
      //     for (int i=0;i<=maxkeys;i++){
      //       keytracking[i] = kpayload.keys[i];
      //       if (keytracking[i] > 0){
      //         Serial.print(keytracking[i]); 
      //         keyfollowup = true;       
      //       }
      //     }
      //     Serial.println(F("'")); 
      //     if (keyfollowup){
      //       // process the received characters, interpret as commands etc.



      //     }       

      //     // end of keypad message collection      
      //     mreceived += 1;
      //   }
      //   else{
      //     Serial.println(F("Wrong Keypad keyword")); 
      //   }
      //   break;
      // Handle TM1638 input
      case 'T': 
        RFacktype = header.type; // send an acknowledge (the received data)
        bool buttonfollowup = false;
        tm_payload tpayload;
        network.read(header, &tpayload, sizeof(tpayload));
        Serial.print(F("Received from TM1638 nodeId: "));
        tmnode = header.from_node;
        Serial.print(header.from_node);
        Serial.print(F(", timing: "));
        Serial.println(tpayload.timing);
        if (tpayload.keyword == keywordvalT) {
          // message received from TM1638
     
          Serial.print(F("Button(s): '"));        
          for (int i=0;i<8;i++){
            if (tpayload.SW[i]){
              Serial.print(i); 
              buttonfollowup = true; 
              acktxtvalues[i] = 0xff;      
            }
            else acktxtvalues[i] = 0x00;      
          }
          Serial.println(F("'")); 
          if (buttonfollowup){
            ackbuttonvalue = tpayload.buttons;
            // process the received button presses, interpret as commands etc.

          }

          // end of keypad message collection      
          mreceived += 1;
        }
        else{
          Serial.println(F("Wrong T keyword")); 
        }
        break;
      default: 
        network.read(header, 0, 0);
        Serial.print(F("TBD header.type: "));
        Serial.println(header.type);
    }
  }

  if (mreceived > 1){
    Serial.print(F("mreceived: "));
    Serial.println(mreceived);
  }
  
  return mreceived > 0;
}

//===== Sending =====//
void transmitRFnetwork(bool pnew, unsigned long currentRFmilli){
  static unsigned long sendingTimer = 0;
  static uint8_t counter = 0;
  static uint8_t failcount = 0;
  bool w_ok;
  uint16_t destnode = 00;
  tm_ack_payload Txdata;

  // When acknowledge requested expected
  if (RFacktype != '0'){
    // sendingTimer = currentRFmilli;

    switch(RFacktype) {
      // ack for TM1638
      case 'T': 
        destnode = tmnode;
        tm_ack_payload Txdata;
        Txdata.keyword = keywordvalT;
        Txdata.leds = ackbuttonvalue;
        for (int i=0;i<8;i++){
          Txdata.TXT[i] = acktxtvalues[i];
          acktxtvalues[i] = 0x00;
        }

        break;
      default: 
        Serial.print(F("No ACK for header.type: "));
        Serial.println(RFacktype);
    }

    if (destnode > 0){
      Txdata.timing = currentRFmilli;
      Txdata.counter = counter++;
      RF24NetworkHeader header0(destnode, RFacktype); // address where the data is going
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
    }
    Serial.print(Txdata.counter);
    Serial.print(F(", "));
    Serial.println(currentRFmilli);
  }
  RFacktype = '0';
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
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

  clearkeypadcache();

  // set servo's in some kind of start position
  driveServo(0, memPos[1][0]); // 1 X
  driveServo(1, memPos[1][1]); // 1 Y
  driveServo(2, memPos[2][0]); // 2 X
  driveServo(3, memPos[2][1]); // 2 Y
}

bool newdata = false;

void loop() {

  network.update();

  currentmilli = millis();

  newdata = receiveRFnetwork(currentmilli);

  //************************ sensors/actuators ****************//

  // depending on received data, driveServo() (one or more commands)
  interpretdata(newdata, currentmilli);

  //************************ sensors/actuators ****************//

  transmitRFnetwork(newdata, currentmilli);

}
