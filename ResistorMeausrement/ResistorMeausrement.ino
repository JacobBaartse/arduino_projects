
#include "Arduino_LED_Matrix.h"
#include "display_4xdigit.h"

bool debug=false;

int sensorPin = A0;   // select the input pin to be measured
float sensorValue = 0;  // variable to store the value coming from the sensor
const int minus_sign = 15;
const int exponent_offset = 10;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  analogReadResolution(14);
  display_digit_setup();

}

float voltage_gemeten=0;
float i_ref_resistor=0;
float unkonwn_resistor=0;

void loop() {
  // read the value from the sensor:
  sensorValue = 0;
  for  (int i=0; i< 400; i++){
      sensorValue += analogRead(sensorPin);
  }
  sensorValue /= 400;
  if (debug) Serial.println(sensorValue, 6);

  //voltage_gemeten = ((float)5)*sensorValue/16313.5;  // 230k ipv 220k
  voltage_gemeten = ((float)5)*sensorValue/16316;  // 220k ipv 220k   correction for high resistor values
  //voltage_gemeten = ((float)5)*sensorValue/16320;  // 213k ipv 220k
  
  if (debug) Serial.println(voltage_gemeten, 6);

  i_ref_resistor = (5-voltage_gemeten)/1000;
  
   if (debug) Serial.println(i_ref_resistor, 6);
   if (i_ref_resistor < 0.000001) i_ref_resistor=0.000001;
  unkonwn_resistor = voltage_gemeten/i_ref_resistor - 1.3;   // correction for low resisteor values

   if (debug) Serial.println(unkonwn_resistor, 6);
   if (debug) Serial.println("------");


  if (unkonwn_resistor > 4900000){
      show_digits( minus_sign, minus_sign, minus_sign, minus_sign);
  } 
  else{
    int exponent = 0;
    while (unkonwn_resistor > 999.9999){
      exponent ++;
      unkonwn_resistor = unkonwn_resistor / 10;
    }

    show_digits(
        (((int)unkonwn_resistor) /100 ) % 10,
        (((int)unkonwn_resistor) /10) % 10,      
        ((int)unkonwn_resistor) % 10,
        exponent + exponent_offset);
      
  }


  delay(1000);
}
