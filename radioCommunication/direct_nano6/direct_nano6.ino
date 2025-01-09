/*
 * Nano with RF module

 https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/?utm_content=cmp-true

 https://forum.arduino.cc/t/communication-problem-with-nrf24l01-and-2-arduino-nano/666085

 https://arduino.stackexchange.com/questions/79452/nrf24l01-channel-switching

 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Servo.h>

#define pinPIR 7 // PIR pin connection

#define SERVOpin 5 // Servo pin connection

#define radioChannel 111

/**** Configure the nrf24l01 CE and CSN pins ****/
// for the NANO with onboard RF24 module:
// RF24 radio(10, 9); // nRF24L01 (CE, CSN)
// for the UNO/NANO with external RF24 module:
//RF24 radio(8, 7); // nRF24L01 (CE, CSN)

#define CE_PIN 10
#define CSN_PIN 9
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)

Servo myservo;  // create servo object to control a servo

uint32_t sleepTimer = 0;

void setup() {
  Serial.begin(115200);
  Serial.println(F(" ************<<"));  

  //pinMode(LED_BUILTIN, OUTPUT);
  //digitalWrite(LED_BUILTIN, LOW); 

  pinMode(pinPIR, INPUT);

  myservo.attach(SERVOpin);  
  for (int p = 0; p <= 90; p += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(p);              // tell servo to go to position in variable 'pos'
    delay(10);                     // waits 15ms for the servo to reach the position
  }

  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(radioChannel);

  Serial.println(F(" <<*************"));  
  Serial.flush();  
}

bool detectionval = false;
bool remdetectionval = false;
uint8_t detval = 0;
uint8_t distval = 0;
bool sendDirect = false;

unsigned long stopPresenceDetectionTime = 0;
//int posServo = 0; // variable to store the servo position
bool swingServo = false;
uint32_t servoTimer = 0;

void handleServo(){
  static int pos = 90;
  static int rempos = 10;
  static int incremental = 1;
  if ((swingServo)||(pos != 90)){ // calculate next pos
    pos += incremental;
    myservo.write(pos); // tell servo to go to position in variable 'pos'
  }
  if (((pos < 3)&&(incremental<0))||((pos > 177)&&(incremental>0))){ 
    incremental = -incremental; 
  }
  // if (pos != rempos){
  //   Serial.print(F(", "));
  //   Serial.print(pos);  
  //   Serial.print(F(", inc: "));
  //   Serial.println(incremental); 
  //   rempos = pos;
  // } 
}

unsigned long timinginloop = 0;

void loop() {

  timinginloop = millis();

  detectionval = digitalRead(pinPIR) == HIGH;
  detval = 0;
  if (detectionval){
    detval = 0xff;
    if (!swingServo){
      sendDirect = true; // send new detection directly
      //digitalWrite(LED_BUILTIN, HIGH); 
    }
    swingServo = true;
    stopPresenceDetectionTime = timinginloop;
  }
  if (remdetectionval != detectionval){
    remdetectionval = detectionval;
    Serial.print(F("PIR detection: "));
    Serial.println(detectionval);
  }

  //if (timinginloop - stopPresenceDetectionTime > 60000) { // scan time after detection
  if (timinginloop - stopPresenceDetectionTime > 10000) { // scan time after detection
    swingServo = false;
    //digitalWrite(LED_BUILTIN, LOW); 
  }

  if (timinginloop - servoTimer > 25) { // limit rotation speed
    servoTimer = timinginloop;
    handleServo();
  }

}
