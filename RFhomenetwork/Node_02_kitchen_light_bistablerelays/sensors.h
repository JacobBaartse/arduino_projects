/*
 * PIR sensor and distance sensor as well as on and off buttons
 */

#include "Wire.h"

#define pinPIR1 6        // PIR pin connection
#define pinPIR2 7        // PIR pin connection

#define pinPressButton 2 // light off button

void setupsensors(){

  pinMode(pinPIR1, INPUT);
  pinMode(pinPIR2, INPUT);
  pinMode(pinPressButton, INPUT_PULLUP);

}

unsigned int checkSensors(unsigned long currentSensormilli)
{
  static unsigned long sensortime = 0;
  static bool detionval = false;

  if ((unsigned long)(currentSensormilli - sensortime) > 1000){
    sensortime = currentSensormilli;

    bool det1 = digitalRead(pinPIR1) == LOW;
    //bool det2 = digitalRead(pinPIR2) == LOW;    

    if (detionval != det1){
      Serial.print(currentSensormilli);
      Serial.print(F(" PIR1 "));
      Serial.println(det1);
      detionval = det1;
    }

  }
}
