/* Dev by Artron Shop Co.,Ltd. */

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <AHT10.h>

Adafruit_BMP280 bmp;
AHT10 myAHT20(AHT10_ADDRESS_0X38, AHT20_SENSOR);

void setup() {
  Serial.begin(115200);
  Serial.println(F("AHT20+BMP280 test"));

  while (myAHT20.begin() != true) {
    Serial.println(F("AHT20 not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
    delay(5000);
  }
  Serial.println(F("AHT20 OK"));
  
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {
  Serial.printf("Temperature: %.02f *C\n", myAHT20.readTemperature());
  Serial.printf("Humidity: %.02f %RH\n", myAHT20.readHumidity());
  Serial.printf("Pressure: %.02f hPa\n", bmp.readPressure());

  delay(1000);
}
