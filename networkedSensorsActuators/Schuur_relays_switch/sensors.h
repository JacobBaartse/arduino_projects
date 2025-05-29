/*
 * PIR sensor and distance sensor as well as on and off buttons
 */

#include "Wire.h"

#define PIR1_PIN 6
#define PIR2_PIN 7
#define IN_PIN 2

#define SR04_I2CADDR 0x57

#define distanceThreshold 100 // in cm
#define off_delay 120000 // in ms

#define mask_button 1
#define mask_no_movement 2
#define mask_remain_on 4
#define mask_auto_on 8
#define mask_auto_off 16

byte ds[3] = {0, 0, 0};

void setupsensors(){

  pinMode(PIR1_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(IN_PIN, INPUT_PULLUP);

  // Start I2C
  Wire.begin();
}

unsigned long measure_distance(){
  int i = 0;

  Wire.beginTransmission(SR04_I2CADDR);
  Wire.write(1);          // 1 = cmd to start meansurement
  Wire.endTransmission();
  delay(120);             // 1 cycle approx. 100ms. 
  Wire.requestFrom(SR04_I2CADDR, 3); //read distance (3 bytes)      
  while (Wire.available())
  {
    ds[i++] = Wire.read();
  }
  unsigned long distance = (unsigned long)(ds[0]) * 65536;
  distance = distance + (unsigned long)(ds[1]) * 256;
  distance = (distance + (unsigned long)(ds[2])) / 10000;
  if (distance > 0) {
    Serial.print(distance);
    Serial.println(F(" cm"));
  }
  return distance;
}

// #define mask_button 1
// #define mask_no_movement 2
// #define mask_remain_on 4
// #define mask_auto_on 8
// #define mask_auto_off 16
unsigned int checkSensors(unsigned long currentmilli, unsigned int tracksensors)
{
  static unsigned int bloop = 0;
  static unsigned long sensortime = 0;
  //static unsigned int sensorstate = 0;
  unsigned int sensorstate = 0;
  unsigned long mdistance = 0;

  bool det1 = digitalRead(PIR1_PIN) == LOW;
  bool det2 = digitalRead(PIR2_PIN) == LOW;
  bool buttonpress = digitalRead(IN_PIN) == LOW;

  // check PIR detectors
  if (det1 && det2)
  {
    sensorstate += mask_remain_on; // remaining on signal
    if (tracksensors > 5){
      sensortime = currentmilli + off_delay; 
    }
    else {
      // check distance sensor
      mdistance = measure_distance();
      if (mdistance < distanceThreshold){ // if something between sensor and wall/floor
        sensortime = currentmilli + off_delay; 
        sensorstate += mask_auto_on; // automatic on signal
      }
    }
  }

  // if none of the detectors, check automatic turn off time
  if (!det1 && !det2)
  {
    sensorstate += mask_no_movement; // no movement signal
    if (currentmilli > sensortime){
      sensorstate += mask_auto_off; // automatic off signal
    }
  }

  // check button
  if (buttonpress)
  {
    if (bloop < 1){
      Serial.println(F("Button press"));
    }
    bloop = 1000;
  }
  if (bloop > 0){ // anti dender software
    bloop -= 1;
    sensorstate += mask_button; // button pressed signal
  }

  return sensorstate;
}
