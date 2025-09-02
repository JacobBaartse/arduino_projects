/*
AHT20    0x38
BMP280   0x77

Based on examples from:
https://embedded-things.blogspot.com/2021/02/test-aht20bmp280-temperature-humidity.html

*/

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

void temppress_setup() {

  Serial.println(F("Setup AHT20+BMP280"));

  if (!aht.begin()) {
    Serial.println("Could not find AHT20 sensor: Check wiring!");
    while(1) delay(1000);
  }
  Serial.print("AHT20 found, ");

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor: check wiring!"));
    while(1) delay(1000);
  }
  Serial.println("BMP280 found");

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void tempress_data(unsigned long currentmilli) {
  Serial.println(F("----------------"));
  Serial.println(currentmilli);

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
  Serial.print(F("Temperature: ")); 
  Serial.print(temp.temperature); 
  Serial.println(F(" degrees C"));

  Serial.print(F("Humidity: ")); 
  Serial.print(humidity.relative_humidity); 
  Serial.println(F("% rH"));

  Serial.print(F("Temperature: "));
  Serial.print(bmp.readTemperature());
  Serial.println(F(" *C"));

  Serial.print(F("Pressure: "));
  Serial.print(bmp.readPressure());
  Serial.println(F(" Pa"));

  Serial.println(F("----------------"));
}

void tempress_values(unsigned long currentmilli){
  static unsigned long sensorTimer = 0;
  if (currentmilli > sensorTimer){ // todo de andere berekening die wel altijd werkt
    sensorTimer = (unsigned long) currentmilli + 60000; // once per minute
    tempress_data(currentmilli);
  }
}
