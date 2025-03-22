/*
AHT20    0x38
BMP280   0x77

Based on examples from:
https://learn.adafruit.com/adafruit-aht20/arduino

https://embedded-things.blogspot.com/2021/02/test-aht20bmp280-temperature-humidity.html

*/

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

int sensor1_temp = 0;
int sensor1_humi = 1;
int sensor2_temp = 2;
int sensor2_pres = 3;
bool ahtsensor = false;
bool bmpsensor = false;

bool sensors_setup(){

  ahtsensor = aht.begin();
  // if (!aht.begin()) {
  //   Serial.println(F("Could not find AHT20 sensor: Check wiring!"));
  //   while(1) delay(10);
  // }
  if (ahtsensor)
    Serial.println(F("AHT20 found"));

  bmpsensor = bmp.begin();
  // if (!bmp.begin()) {
  //   Serial.println(F("Could not find a valid BMP280 sensor: check wiring!"));
  //   while(1) delay(10);
  // }
  if (bmpsensor) {
    Serial.println(F("BMP280 found"));

    /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }
  return true;
}

bool read_sensors(){
  static unsigned long sensortime = 0xffffffff; // start directly from first read_sensors
  static int humidity_mem = 10;
  static int pressure_mem = 11;
  static int temp1_mem = 12;
  static int temp2_mem = 13;

  unsigned long timing = millis();
  if((sensortime - timing) < 60000) return false;

  sensortime = timing + 60000; // once per minute

  if (ahtsensor){
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data

    sensor1_temp = int(temp.temperature * 10); // make it 1 digit after the .
    sensor1_humi = int(humidity.relative_humidity);
  }
  if (bmpsensor){
    float sensor2_tempe = bmp.readTemperature();
    sensor2_temp = int(sensor2_tempe * 10); // make it 1 digit after the .
    float sensor2_press = bmp.readPressure();
    sensor2_pres = int(sensor2_press / 100); // make it hPa
  }

  bool changedetected = false;
  if (pressure_mem != sensor2_pres){
    pressure_mem = sensor2_pres;
    changedetected = true;
  }
  if (humidity_mem != sensor1_humi){
    humidity_mem = sensor1_humi;
    changedetected = true;
  }
  if (temp1_mem != sensor1_temp){
    temp1_mem = sensor1_temp;
    changedetected = true;
  }  
  if (temp2_mem != sensor2_temp){
    temp2_mem = sensor2_temp;
    changedetected = true;
  }
  if (changedetected){
    Serial.println(int(timing/1000));

    Serial.print(F("Temperature 1: ")); 
    Serial.print(sensor1_temp); 
    Serial.println(F(" *C"));

    Serial.print(F("Humidity: ")); 
    Serial.print(sensor1_humi); 
    Serial.println(F("% rH"));

    Serial.print(F("Temperature 2: "));
    Serial.print(sensor2_temp);
    Serial.println(F(" *C"));

    Serial.print(F("Pressure: "));
    Serial.print(sensor2_pres);
    Serial.println(F(" hPa"));

    Serial.println(F("-------"));
  }
  return changedetected;
}
