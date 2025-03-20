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

void setup() {
  Serial.begin(115200);
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush();

  Serial.println(F("AHT20+BMP280 test"));

  if (!aht.begin()) {
    Serial.println("Could not find AHT20 sensor: Check wiring!");
    while(1) delay(10);
  }
  Serial.println("AHT20 found");

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor: check wiring!"));
    while(1) delay(10);
  }
  Serial.println("BMP280 found");

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */


}

void loop() {

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); 
  Serial.print(temp.temperature); 
  Serial.println(" degrees C");

  Serial.print("Humidity: "); 
  Serial.print(humidity.relative_humidity); 
  Serial.println("% rH");

  Serial.print(F("Temperature: "));
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print(F("Pressure: "));
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");

  Serial.println("----------------");

  delay(10000);

}
