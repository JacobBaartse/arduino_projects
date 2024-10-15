
#include "Arduino_LED_Matrix.h"
#include "display_3xdigit.h"

#include "display_3xdigit.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMono9pt7b.h>


bool debug=true;

int sensorPin = A0;   // select the input pin to be measured
// const int pullup1k = A1;
const int pullup1k = A2;
float sensorValue = 0;  // variable to store the value coming from the sensor
const int minus_sign = 15;
const int exponent_offset = 10;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  analogReadResolution(14);
  display_digit_setup();
  pinMode(pullup1k,OUTPUT);
  digitalWrite(pullup1k, HIGH);
  delay(100);
}

float voltage_gemeten=0;
float i_ref_resistor=0;
float unknown_resistor=0;
float high_correction_factor=1;

const int num_measurements = 100;  // num of measurements should result in a multiple of  about 0,060 ms ( 3 times 50 hz duration)

void loop() {
  // read the value from the sensor:
  int results [num_measurements];
  sensorValue = 0;
  for  (int i=0; i< num_measurements; i++){
      //sensorValue += analogRead(sensorPin);
      delay(1);
      results[i]=analogRead(sensorPin);
  }
  for  (int i=0; i< num_measurements; i++){
    Serial.println(results[i]);
    sensorValue += results[i];
  }
  Serial.println("results done");
  
  sensorValue /= num_measurements;
  if (debug) Serial.println(sensorValue, 6);

  //voltage_gemeten = ((float)5)*sensorValue/16313.5;  // 230k ipv 220k
  voltage_gemeten = ((float)5)*sensorValue/16316;  // 220k ipv 220k   correction for high resistor values
  //voltage_gemeten = ((float)5)*sensorValue/16320;  // 213k ipv 220k
  
  if (debug) Serial.println(voltage_gemeten, 6);

  i_ref_resistor = (5-voltage_gemeten)/1000;
  
  if (debug) Serial.println(i_ref_resistor, 6);
  if (i_ref_resistor < 0.00000001) i_ref_resistor=0.00000001;  // prevent divede by zero
  unknown_resistor = voltage_gemeten/i_ref_resistor - 0.65;   // correction for low resisteor values

  // correct for high values
  high_correction_factor = 1 + (unknown_resistor / 11000000);
  unknown_resistor *= high_correction_factor;
  

   if (debug) Serial.println(unknown_resistor, 6);
   if (debug) Serial.println("------");
   show_value(unknown_resistor);
}
