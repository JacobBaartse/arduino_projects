#include<Wire.h>

const int mpu_addr=0x68;  // I2C address of the MPU-6050

void setup()
{
  Wire.begin();
  // Wire.setClock(400000UL); // Set I2C frequency to 400kHz
  Wire.beginTransmission(mpu_addr);
  Wire.write(0x6B);
  Wire.write(0); // wake up the mpu6050
  Wire.endTransmission(true);
  Serial.begin(115200);
}


void loop() {        
  int movement_x =  Gyro_X();
  int movement_y =  Gyro_Y();
  int movement_z =  Gyro_Z();
  int movement = movement_x + movement_y + movement_z;


if (movement > 5){
    Serial.print(millis());
    Serial.print("\t");
    Serial.print(movement);
    Serial.print("\t");
    Serial.print(movement_x);
    Serial.print("\t");
    Serial.print(movement_y);
    Serial.print("\t");
    Serial.println(movement_z);
  } 
//  Serial.print();
//  Serial.print("\t");
//  delay(50);
//  Serial.print(Gyro_Y()-1);
//  Serial.print("\t");
//  delay(50);
//  Serial.print(Gyro_Z());
//  Serial.println("\t");
//  delay(50);
//  Serial.print(Acc_X());
//  Serial.print("\t");
//  delay(50);
//  Serial.print(Acc_Y());
//  Serial.print("\t");
//  delay(50);
//  Serial.println(Acc_Z());
 delay(100);
}


int read_val(int command_val)
{
  Wire.beginTransmission(mpu_addr);
  Wire.write(command_val);
  Wire.endTransmission(false);
  Wire.requestFrom(mpu_addr,1);
  int Gy=Wire.read(); //<<8|Wire.read();
  if (Gy > 0x80) Gy -= 0xFF;
  Wire.endTransmission(true);
  return abs(Gy); // / 250;
}

int Gyro_X()
{
  return read_val(0x43);
}

int Gyro_Y()
{
  return read_val(0x45);
}

int Gyro_Z()
{
  return read_val(0x47);
}

int Acc_X()
{
  return read_val(0x3B);
}

int Acc_Y()
{
  return read_val(0x3D);
}

int Acc_Z()
{
  return read_val(0x3F);
}