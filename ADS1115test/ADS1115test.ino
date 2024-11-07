// https://how2electronics.com/how-to-use-ads1115-16-bit-adc-module-with-arduino/

  //                                ADS1015  ADS1115
  //                                -------  -------
  //  2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  //  1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  //  2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  //  4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  //  8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  //  16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
//Download: RobTillaart ADS1X15 Library

#include "ADS1X15.h"


#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeMono9pt7b.h>
#include "display128x32.h"

ADS1115 ADS(0x48);

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Hello!");

  display_setup();
  oled_display("Hello", "World");

  ADS.begin();

}

void loop(void)
{
  int gain = 2;
  float supply_voltage = 4.7;
  ADS.setGain(gain);  // 0, 1, 2, 4, 8, 16
 
  long val_differential = 0;
  long absolute_val = 0;
  float absolute_val_power = 0;

  int samplecount = 100;
  for (int i =0; i<samplecount; i++){
    val_differential += ADS.readADC_Differential_2_3();  // to measure low resistance
  }
  delay(10); 
  for (int i =0; i<samplecount; i++){
        absolute_val += ADS.readADC(2);  // to measure high resistance
  }
  delay(10);
  ADS.setGain(0);
  for (int i =0; i<samplecount; i++){
        absolute_val_power += ADS.readADC(3);  // to measure high resistance power supply
  }
  delay(10);
  // Serial.print("absolute_val");
  // Serial.println((float)absolute_val/samplecount);
  absolute_val_power = absolute_val_power / samplecount;
  absolute_val_power = absolute_val_power * 0.0001875;
  float resistor=0;
  float high_resistor_val = (float)absolute_val /  samplecount;
  float low_resistor_val = 0 - ((float)val_differential / samplecount);

  if (high_resistor_val < low_resistor_val){
    high_resistor_val -= 16;  // correct for high resistors open value
    float voltage = (4.096/gain/32768) * high_resistor_val;
    if (voltage > 0.0005){
      float current = voltage / 2200;
      resistor = (supply_voltage - voltage)/current;
    }
    else { resistor = -1; }
  }
  else{
    float voltage_bottom = (4.096/gain/32768) * high_resistor_val;
    float voltage_resitor = (4.096/gain/32768) * low_resistor_val;
    float current = voltage_bottom / 2200;
    resistor = voltage_resitor / current;
  }



  oled_display("r " + String(resistor), " " + String(""));

}

