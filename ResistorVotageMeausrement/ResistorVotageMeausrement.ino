
#include "Arduino_LED_Matrix.h"
#include "display_3xdigit.h"

#include "display_3xdigit.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMono9pt7b.h>


bool debug=false;

const int sensorPinR = A0;
const int pullup1k = A1;
const int pullup27k = A2;

const int sensorPinV = A3; 

const int range_1k = 1;
const int range_100k = 2;
int cur_range = range_1k;

float sensorValue = 0;  // variable to store the value coming from the sensor
const int minus_sign = 15;
const int exponent_offset = 10;

void set_range(int range){
  switch (range){
    case range_1k:
      pinMode(pullup1k,OUTPUT);
      digitalWrite(pullup1k, HIGH);
      pinMode(pullup27k,INPUT);
      analogReference(AR_DEFAULT);
      delay(50);
    break;
    case range_100k:
      pinMode(pullup27k,OUTPUT);
      digitalWrite(pullup27k, HIGH);
      pinMode(pullup1k,INPUT);
      analogReference(AR_DEFAULT);  
      delay(50);
    break;
  }
}


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  analogReadResolution(14);
  display_digit_setup();

  set_range(cur_range);
}

void get_resistor_measurement(int num_measurements){
  sensorValue = 0;
  for  (int i=0; i< num_measurements; i++){
    sensorValue += analogRead(sensorPinR);
    delay(1);
  }
  sensorValue /= num_measurements;
  
  if (debug) Serial.println(sensorValue, 6);
}

float measured_resistor=0;

void calculate_resistor(){
  float voltage_gemeten=0;
  float i_ref_resistor=0;
  float ref_voltage = 5;
  float pullup_resistor = 1000;
  if (cur_range == range_100k) pullup_resistor = 270000;

  voltage_gemeten = ((float)ref_voltage)*sensorValue/16384;
  if (debug) Serial.println(voltage_gemeten, 6);

  i_ref_resistor = (5-voltage_gemeten)/pullup_resistor;
  if (debug) Serial.println(i_ref_resistor, 6);

  if (i_ref_resistor < 0.00000001) i_ref_resistor=0.00000001;  // prevent divide by zero
  measured_resistor = voltage_gemeten/i_ref_resistor;
  if (debug) Serial.print("cur_range ");
  if (debug) Serial.println(cur_range);
  if (debug) Serial.print("measured_resistor before correction");
  if (debug) Serial.println(measured_resistor, 6);

  // value corrections
  if (cur_range == range_100k){  // no correction needed for 100k range
    if (measured_resistor>800000 ){
      measured_resistor = measured_resistor * 1.02;  
    }
  }
  if (cur_range == range_1k){
    measured_resistor = (measured_resistor- 0.30)*1.03;
    if (debug) Serial.print("correction for 1k range");
  }
  if (debug) Serial.print("measured_resistor after correction");
  if (debug) Serial.println(measured_resistor, 6);

  if (debug) Serial.println("------");
}

float voltage = 0;
void   measure_voltage(){
  const int num_measurements = 1000; 
  sensorValue = 0;
  for  (int i=0; i< num_measurements; i++){
    sensorValue += analogRead(sensorPinV);
    delay(1);
  }
  voltage =  (float) sensorValue /(float) num_measurements;
  if (debug) Serial.println(voltage);
  voltage -= 8184;
  if (debug) Serial.println(voltage);
  voltage /= 240.0;
  if (debug) Serial.println(voltage);
}

void loop() {
  set_range(cur_range);
  get_resistor_measurement(50);
  calculate_resistor();
  if (measured_resistor > 22000) cur_range = range_100k;
  if (measured_resistor < 15000) cur_range = range_1k;
  set_range(cur_range);
  get_resistor_measurement(200);
  calculate_resistor(); 
  measure_voltage();

  show_value(measured_resistor, voltage);
}
