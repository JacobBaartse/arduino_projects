/* Dev by Artron Shop Co.,Ltd. */

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h> 

Adafruit_BMP280 bmp;
Adafruit_AHTX0 myAHT20(AHT10_ADDRESS_0X38, AHT20_SENSOR);

void setup() {
  Serial.begin(115200);
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush();

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
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
  Serial.print("Pressure: "); Serial.print(bmp.readPressure()); Serial.println(" hPa");

  delay(1000);

  Serial.print("Temperature : ");
  Serial.print(myAHT20.readTemperature());
  Serial.println(" *C");
  
  Serial.print("Humidity : ");
  Serial.print(myAHT20.readHumidity());
  Serial.println(" RH");
  
  Serial.print("Pressure : ");
  Serial.print( bmp.readPressure());
  Serial.println(" hPa");
  Serial.println(" ");
  
  delay(1000);
}
