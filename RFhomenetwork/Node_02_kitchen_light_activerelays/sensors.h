/*
 * PIR sensor and distance sensor as well as on and off buttons
 */

#include "Wire.h"

#define pinPIR1 8        // PIR pin connection
#define pinPIR2 7        // PIR pin connection

#define pinPressButton 2 // light off button

#define SR04_I2CADDR  0x57

byte ds[3] = {0, 0, 0};

void setupsensors(){

  pinMode(pinPIR1, INPUT);
  pinMode(pinPIR2, INPUT);
  pinMode(pinPressButton, INPUT_PULLUP);

  // Start I2C
  Wire.begin();
}

unsigned long measure_distance(){
  unsigned long distance = 0;
  int i = 0;
  bool newdata = false;

  Wire.beginTransmission(SR04_I2CADDR);
  Wire.write(1);          // 1 = cmd to start meansurement
  Wire.endTransmission();
  delay(120);             // 1 cycle approx. 100ms. 
  Wire.requestFrom(SR04_I2CADDR, 3); //read distance (3 bytes)      
  while (Wire.available())
  {
    ds[i++] = Wire.read();
    newdata = true;
  }
  if (newdata){
    distance = (unsigned long)(ds[0]) * 65536;
    distance = distance + (unsigned long)(ds[1]) * 256;
    distance = (distance + (unsigned long)(ds[2])) / 10000;
    if (distance > 0) {
      Serial.print(distance);
      Serial.println(F(" cm"));
    }
  }
  return distance;
}

unsigned int checkSensors(unsigned long currentSensormilli)
{
  static unsigned long sensortime = 0;

  if ((unsigned long)(currentSensormilli - sensortime) > 1000){
    sensortime = currentSensormilli;

    // bool det1 = digitalRead(pinPIR1) == LOW;
    // bool det2 = digitalRead(pinPIR2) == LOW;    
    int det1 = digitalRead(pinPIR1);
    int det2 = digitalRead(pinPIR2);
    unsigned long dist = measure_distance();

    if ((det1 > 0)||(det2 > 0)||(dist > 0)){
      Serial.print(currentSensormilli);
      Serial.print(F(" PIR1 "));
      Serial.print(det1);
      Serial.print(F(" PIR2 "));
      Serial.print(det2);
      Serial.print(F(" DIST "));
      Serial.print(dist);
      Serial.println(F(" cm"));
    }
  }
}
