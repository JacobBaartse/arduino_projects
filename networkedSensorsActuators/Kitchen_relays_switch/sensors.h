/*
 * PIR sensor and distance sensor as well as on and off buttons
 */

#include "Wire.h"

#define PIR_PIN 6
#define ON_PIN 8
#define OFF_PIN 4
#define SR04_I2CADDR 0x57

#define distanceThreshold 100 // in cm
#define off_delay 120000 // in ms

byte ds[3] = {0, 0, 0};

void setupsensors(){

  pinMode(PIR_PIN, INPUT);
  pinMode(ON_PIN, INPUT_PULLUP);
  pinMode(OFF_PIN, INPUT_PULLUP);

  // Start I2C
  Wire.begin();
}

unsigned long measure_distance(){
  int i = 0;

  Wire.beginTransmission(SR04_I2CADDR);
  Wire.write(1);          // 1 = cmd to start meansurement
  Wire.endTransmission();
  delay(120);             // 1 cycle approx. 100mS. 
  Wire.requestFrom(SR04_I2CADDR, 3); //read distance       
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

unsigned int checkSensors(unsigned long currentmilli, unsigned int tracksensors)
{
  static unsigned int sensorstate = 0;
  static unsigned long sensortime = 0;
  unsigned long mdistance = 0;

  // check PIR detector
  if (digitalRead(PIR_PIN) == LOW)
  {
    // check distance sensor
    mdistance = measure_distance();
    if (mdistance < distanceThreshold){ // if something between sensor and wall/floor
      sensorstate = 100;
      sensortime = currentmilli + off_delay; 
    }
  }
  // check on button
  if (digitalRead(ON_PIN) == LOW)
  {
    sensorstate = 100;
    Serial.println(F("ON"));
  }

  if (sensorstate == 0){
    if (currentmilli > sensortime){ // turn off after set time
      sensorstate = 200;
      Serial.println(F("Timeout (going OFF)"));
    }
    else {
      if (tracksensors > 0){
        Serial.print((sensortime - currentmilli) / 1000);
        Serial.println(F(" seconds to go (for Timeout)"));
        sensorstate = tracksensors;
      }
    }
  }
  // check off button, this overrides the detectors and on button
  if (digitalRead(OFF_PIN) == LOW)
  {
    sensorstate = 200;
    Serial.println(F("OFF"));
  }

  return sensorstate;
}
