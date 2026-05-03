/*
 * ultrasone sensor HC_SR04, via I2C interface
 */

#include "Wire.h"

#define SR04_I2CADDR 0x57

byte ds[3];
unsigned long distancemilli = 0;

void setupDistance(){
  Wire.begin(); // Start I2C
}

uint16_t readDistanceCM(){
  Serial.println(F(" function readDistanceCM "));

  uint16_t retDist = 0xffff;
  uint8_t i = 0;
  ds[0] = 0;
  ds[1] = 0;
  ds[2] = 0;
  Wire.requestFrom(SR04_I2CADDR, 3); // read 3 bytes, the distance       
  while (Wire.available()) // the 3 bytes should be available
  {
    ds[i++] = Wire.read();
  }             
  unsigned long distance = (unsigned long)(ds[0]) * 65536;
  distance = distance + (unsigned long)(ds[1]) * 256;
  distance = (distance + (unsigned long)(ds[2])) / 10000;
  if (distance > 0) { // minimum distance is 4 cm, maximum 400 cm
    Serial.print(distance);
    Serial.println(F(" cm "));
    retDist = distance;
  }
  return retDist;
}

/*
 *    command: 0 nothing
 *             1 start measurement
 *             2 get measurement
 *             - error
 */
uint16_t sonardistance(uint16_t command, unsigned long distMilli){
  static unsigned long startdistancemilli = 0;
  static uint16_t dstate = 0xfff0;
  static uint16_t distcm = 0xfff0;
  uint16_t retval = 0xff01;

  // state can be 0xffff - nothing, 0xfffe - measuring in progress, 0xfffd - measuring complete
  if (dstate == 0xfffe){ // check if measurement is in progress
    Serial.print(" time ");
    Serial.print(distMilli);
    Serial.print(" start distance ");
    Serial.println(startdistancemilli);
    if (((unsigned long)(distMilli - startdistancemilli) > 150)){ // check if measurement complete, processing time is around 100 ms
      dstate = 0xfffd; // measurement is ready
      distcm = readDistanceCM(); 
      Serial.print(" distcm ");
      Serial.println(distcm);
    }
  }

  retval = dstate;
  switch(command){
    case 3:
      // retval = dstate;
      break;
    case 2:
      if (dstate == 0xfffd){
        retval = distcm;
        dstate = 0xfff0; // ready for next measurement
      }
      // else {
      //   retval = dstate;
      // }
      break;
    // case 1:
    default: // 1
      Serial.println(" start I2C ");

      //if (((unsigned long)(distMilli - startdistancemilli) > 250)){ // 4 measurements per second is enough
      //if (true){ // as quickly as possible
        Wire.beginTransmission(SR04_I2CADDR);
        Wire.write(1);          // 1 = cmd to start meansurement
        Wire.endTransmission();
        startdistancemilli = millis();
        Serial.print(" start distance ");
        Serial.println(startdistancemilli);
        dstate = 0xfffe;
        retval = dstate;
      //}
  }

  return retval;
}

// get status of distance measurement
uint16_t getDistance(unsigned long distMilli) {
  return sonardistance(3, distMilli);
}

// start distance measurement
uint16_t startDistance(unsigned long distMilli){
  return sonardistance(1, distMilli);
}

// return distance in cm
uint16_t readDistance(unsigned long distMilli) {
  return sonardistance(2, distMilli);
}

// state can be 0xffff - nothing, 0xfffe - measuring in progress, 0xfffd - measuring complete
uint16_t measureDistance(unsigned long distMilli){
  //static bool measureactive = true;
  static unsigned long measureuntil = 0;

  // Serial.print(F("measureDistance "));
  // Serial.println(distMilli);
  uint16_t status = getDistance(distMilli);

  //if (measureactive){
    Serial.print(F("getDistance "));
    Serial.println(distMilli);
    //Serial.println(status, HEX);
    if (status < 0xfffa){
      status = startDistance(distMilli);
      Serial.print(F(" startDistance "));
      Serial.println(distMilli);
      //Serial.println(status, HEX);
    }
    else if (status == 0xfffd){
      status = readDistance(distMilli);
      Serial.print(F(" readDistance "));
      Serial.println(distMilli);
      //Serial.println(status, HEX);
    }
    Serial.println(status, HEX);
  //}
  return status;
}
