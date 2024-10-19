/* 
 *  This servo, SG90 360 degrees seems to be a motor:
 *
 *  position   0 means maximum speed right (clockwise)
 *  position  45 means medium speed right (clockwise)
 *  position  90 means still, not moving, idle position
 *  position 135 means medium speed left (counterclockwise)
 *  position 180 means maximum speed left (counterclockwise)
 */

#include <Servo.h>

Servo myservo;  // create Servo object to control a servo
// twelve Servo objects can be created on most boards

int pos = 0;    // variable to store the servo position


void set_servo(int position, int duration=1000) {
  Serial.print(F("Servo position/speed: ")); 
  Serial.println(position);

  delay(duration);
  myservo.write(position); 

  Serial.print(F("Servo position/speed: ")); 
  Serial.println(position);
  delay(duration);
}

int valinput = 90; // idle for this servo

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  myservo.attach(9); // attaches the servo on pin 9 to the Servo object

  set_servo(valinput);
}

void loop() {
  // for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
  //   // in steps of 1 degree
  //   set_servo(pos);                // tell servo to go to position in variable 'pos'
  // }
  // for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
  //   set_servo(pos);                // tell servo to go to position in variable 'pos'
  // }

  while (Serial.available() > 0) {
    valinput = Serial.parseInt();
    if (valinput > -1) {
      set_servo(valinput);
    }
  }

}
