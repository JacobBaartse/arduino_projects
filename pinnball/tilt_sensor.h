

const int mpu_addr=0x68;  // I2C address of the MPU-6050

void tilt_setup()
{
  Wire.begin();
  // Wire.setClock(400000UL); // Set I2C frequency to 400kHz
  Wire.beginTransmission(mpu_addr);
  Wire.write(0x6B);
  Wire.write(0); // wake up the mpu6050
  Wire.endTransmission(true);
  Serial.begin(115200);
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

bool read_status_ready(){
  Wire.beginTransmission(mpu_addr);
  Wire.write(0x3A);  // INT_STATUS
  Wire.endTransmission(false);
  Wire.requestFrom(mpu_addr,1);
  int Status=Wire.read();
  Wire.endTransmission(true);
  if (Status & 0x01) return true;
  else return false;
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

bool tilt() {        
  
  if (read_status_ready())
  {
    int movement =  Gyro_X();
    if (movement > 8){
      return true;
    }
  }
  return false;
}
