
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  90 // This is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  490 // This is the 'maximum' pulse length count (out of 4096)
#define SERVO_FREQ 50 // Analog servos run at ~50 Hz updates

// our servo # 
uint8_t servonum = 0;   // just test servo on output 0

void setup() {
  Serial.begin(115200);

  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);  // Analog servos run at ~50 Hz updates

  delay(10);
}


void loop() {
  // Drive each servo one at a time using setPWM()
  Serial.println(servonum);

  uint16_t pulselen = 90; //82;  all my servos work with 90 some can go down to 82
  pwm.setPWM(servonum, 0, pulselen);
  delay(2000);

  pulselen = 490; //505;  al my servos work with 490 some can go up to 505
  pwm.setPWM(servonum, 0, pulselen);
  delay(2000);
}
