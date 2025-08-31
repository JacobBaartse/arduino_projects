/*
 * ultrasone sensor HC_SR04, via I2C interface
 */

#include "Wire.h"

#define SR04_I2CADDR 0x57

byte ds[3];
unsigned long distancemilli = 0;
bool distanceinprogress = false;

void setupDistance() {
  Wire.begin(); // Start I2C
}

void startDistance(unsigned long distMilli) {
  if (!distanceinprogress){
    distanceinprogress = true;
    Wire.beginTransmission(SR04_I2CADDR);
    Wire.write(1);          // 1 = cmd to start meansurement
    Wire.endTransmission();
    // delay(120);          // 1 cycle approx. 100 ms 
    distancemilli = distMilli;
    Serial.print(" start distance ");
    Serial.println(distMilli);
  }
}

// return distance in cm
uint16_t readDistance(unsigned long distMilli) {
  static uint8_t loopcount = 0;
  uint16_t retDist = 0;

  if (distanceinprogress){
    retDist = 0xffff;
    loopcount++;
    if (((unsigned long)(distMilli - distancemilli) > 120)){
      uint8_t i = 0;
      ds[0] = 0;
      ds[1] = 0;
      ds[2] = 0;
      Wire.requestFrom(SR04_I2CADDR, 3); // read distance       
      while (Wire.available())
      {
        ds[i++] = Wire.read();
      }             
      distanceinprogress = false;
      unsigned long distance = (unsigned long)(ds[0]) * 65536;
      distance = distance + (unsigned long)(ds[1]) * 256;
      distance = (distance + (unsigned long)(ds[2])) / 10000;
      if (distance > 0) {
        Serial.print(distance);
        Serial.print(" cm ");
        Serial.println(loopcount);
        retDist = distance;
        loopcount = 0;
      }
    }
  }
  return retDist;
}
