
#include "Wire.h"


void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start I2C
  Wire.begin();
}

int SR04_I2CADDR = 0x57;


void loop() {
  byte ds[3];
  ds[0]=0;
  ds[1]=0;
  ds[2]=0;
  Wire.beginTransmission(SR04_I2CADDR);
  Wire.write(1);          //1 = cmd to start meansurement
  Wire.endTransmission();
  delay(120);             //1 cycle approx. 100mS. 
  int i = 0;
  Wire.requestFrom(SR04_I2CADDR,3);  //read distance       
  while (Wire.available())
  {
    ds[i++] = Wire.read();
  }        
    
  unsigned long distance = (unsigned long)(ds[0]) * 65536;
  distance = distance + (unsigned long)(ds[1]) * 256;
  distance = (distance + (unsigned long)(ds[2])) / 10000;
  Serial.print(distance);
  Serial.println("cm");
  delay(1000);
}