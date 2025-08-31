/*
 * ultrasone sensor HC_SR04, via I2C interface
 */

#include "Wire.h"

#define SR04_I2CADDR 0x57

byte ds[3];
unsigned long distancemilli = 0;
//bool distanceinprogress = false;

void setupDistance(){
  Wire.begin(); // Start I2C
}

uint16_t readDistanceCM(){
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
    // Serial.print(distance);
    // Serial.println(F(" cm "));
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
  if (dstate == 0xfffe){ // check if measurement complete
          // Serial.print(" time ");
          // Serial.print(distMilli);
          // Serial.print(" start distance ");
          // Serial.println(startdistancemilli);
    if (((unsigned long)(distMilli - startdistancemilli) > 120)){ // processing time is around 100 ms
      dstate = 0xfffd;
      distcm = readDistanceCM(); 
      // Serial.print(" distcm ");
      // Serial.println(distcm);
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
        dstate = 0xfff0;
      }
      // else {
      //   retval = dstate;
      // }
      break;
    // case 1:
    default: // 1
      if (((unsigned long)(distMilli - startdistancemilli) > 250)){ // 4 measurements per second is enough
        Wire.beginTransmission(SR04_I2CADDR);
        Wire.write(1);          // 1 = cmd to start meansurement
        Wire.endTransmission();
        startdistancemilli = distMilli;
        // Serial.print(" start distance ");
        // Serial.println(distMilli);
        dstate = 0xfffe;
        retval = dstate;
      }
  }

  // switch(dstate){
  //   case 0xfffd:
  //       Serial.print(" dstate ");
  //       Serial.print(dstate, HEX);
  //       Serial.print(" command ");
  //       Serial.println(command);
  //       if (command == 2){
  //         retval = readDistanceCM();
  //         Serial.print(" retval ");
  //         Serial.println(retval);
  //         dstate = 0xfff0;
  //       }
  //     break;
  //   // case 0xfffe:
  //   //   break;
  //   // case 0xffff:
  //   // case 0xfffc:
  //   // case 0xfff0:
  //   default:
  //     if (((unsigned long)(distMilli - startdistancemilli) > 1000)){ // 1 measurement per second is enough
  //       if (command == 1){ // start a measurement
  //         Wire.beginTransmission(SR04_I2CADDR);
  //         Wire.write(1);          // 1 = cmd to start meansurement
  //         Wire.endTransmission();
  //         startdistancemilli = distMilli;
  //         Serial.print(" start distance ");
  //         Serial.println(distMilli);
  //         dstate = 0xfffe;
  //         //retval = dstate;
  //       }
  //     }
  //     retval = dstate;
  // }
  return retval;
}

uint16_t getDistance(unsigned long distMilli) {
  uint16_t sd = sonardistance(3, distMilli);
  return sd;
}

void startDistance(unsigned long distMilli){
  uint16_t sd = sonardistance(1, distMilli);

  // if (((unsigned long)(distMilli - startdistancemilli) > 1000)){ // 1 measurement per second is enough
  //   if (!distanceinprogress){
  //     distanceinprogress = true;
  //     Wire.beginTransmission(SR04_I2CADDR);
  //     Wire.write(1);          // 1 = cmd to start meansurement
  //     Wire.endTransmission();
  //     // delay(120);          // 1 cycle approx. 100 ms 
  //     distancemilli = distMilli;
  //     startdistancemilli = distMilli;
  //     Serial.print(" start distance ");
  //     Serial.println(distMilli);
  //   }
  // }
}

// return distance in cm
uint16_t readDistance(unsigned long distMilli) {
  uint16_t sd = sonardistance(2, distMilli);
  // if (sd < 0xff00) {
  //   Serial.print(sd, HEX);
  //   Serial.println(F(" cm "));
  // }
  return sd;

  // static uint8_t loopcount = 0;
  // uint16_t retDist = 0;

  // if (distanceinprogress){
  //   retDist = 0xffff;
  //   loopcount++;
  //   if (((unsigned long)(distMilli - distancemilli) > 120)){
  //     uint8_t i = 0;
  //     ds[0] = 0;
  //     ds[1] = 0;
  //     ds[2] = 0;
  //     Wire.requestFrom(SR04_I2CADDR, 3); // read distance       
  //     while (Wire.available())
  //     {
  //       ds[i++] = Wire.read();
  //     }             
  //     distanceinprogress = false;
  //     unsigned long distance = (unsigned long)(ds[0]) * 65536;
  //     distance = distance + (unsigned long)(ds[1]) * 256;
  //     distance = (distance + (unsigned long)(ds[2])) / 10000;
  //     if (distance > 0) {
  //       Serial.print(distance);
  //       Serial.print(" cm ");
  //       Serial.println(loopcount);
  //       retDist = distance;
  //       loopcount = 0;
  //     }
  //   }
  // }
  // return retDist;
}

// state can be 0xffff - nothing, 0xfffe - measuring in progress, 0xfffd - measuring complete
uint16_t measureDistance(unsigned long distMilli){
  uint16_t status = getDistance(distMilli);
  if (status < 0xfffc){
    startDistance(distMilli);
  }
  else if (status == 0xfffd){
    status = readDistance(distMilli);
  }
  return status;
}
