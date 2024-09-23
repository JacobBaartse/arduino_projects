/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-dht22
 */

#include "DHT.h"

#include "Arduino_LED_Matrix.h"
#include "display_4xdigit.h"

#define DHT22_PIN 2
#define POWER_RELAY 10

DHT dht22(DHT22_PIN, DHT22);

void setup() {
  Serial.begin(9600);
  dht22.begin(); // initialize the DHT22 sensor
  pinMode(POWER_RELAY, OUTPUT);
  delay(2000);  // give the power of the relay some time to startup.
  digitalWrite(POWER_RELAY, HIGH);

  display_digit_setup();
}

int lower_digit = 1;
void loop() {


  // read temperature as Celsius
  float tempC = dht22.readTemperature();


  // check if any reads failed
  if (isnan(tempC)) {
    tempC=0;
    Serial.println("Failed to read from DHT22 sensor!");
  } else {

    Serial.print("Temperature: ");
    Serial.print(tempC);  
    Serial.println("°C");
    if (tempC > 27.00)
    {
          digitalWrite(POWER_RELAY, LOW);
          lower_digit = 0;
    }
    show_digits((int)tempC/10,
            (int)tempC%10,
            (int)(tempC*10)%10,
            lower_digit);
  }
    // wait a few seconds between measurements.
  delay(60000);

}
