/*
   https://www.instructables.com/Dot-Matrix-With-MAX7219-Interfacing-With-Arduino/
*/

/*
  Basic code for using Maxim MAX7219/MAX7221 with Arduino.

  Wire the Arduino and the MAX7219/MAX7221 together as follows:

  | Arduino   | MAX7219/MAX7221 |
  | --------- | --------------- |
  | MOSI (11) | DIN (1)         |
  | SCK (13)  | CLK (13)        |
  | I/O (7)*  | LOAD/CS (12)    |

    * - This should match the LOAD_PIN constant defined below.
  
  For the rest of the wiring follow the wiring diagram found in the datasheet.
  
  Datasheet: http://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf

  Author:  Nicholas Dobie <nick@nickdobie.com>
  Date:    30 December 2013
  License: WTFPL (http://www.wtfpl.net/)
 */
#include <SPI.h>

// What pin on the Arduino connects to the LOAD/CS pin on the MAX7219/MAX7221
#define LOAD_PIN 7

/**
 * Transfers data to a MAX7219/MAX7221 register.
 * 
 * @param address The register to load data into
 * @param value   Value to store in the register
 */
void maxTransfer(uint8_t address, uint8_t value) {

  // Ensure LOAD/CS is LOW
  digitalWrite(LOAD_PIN, LOW);

  // Send the register address
  SPI.transfer(address);

  // Send the value
  SPI.transfer(value);

  // Tell chip to load in data
  digitalWrite(LOAD_PIN, HIGH);
}
  

void setup() {
  
  // Set load pin to output
  pinMode(LOAD_PIN, OUTPUT);

  // Reverse the SPI transfer to send the MSB first  
  SPI.setBitOrder(MSBFIRST);
  
  // Start SPI
  SPI.begin();

  // Run test
  // All LED segments should light up
  maxTransfer(0x0F, 0x01);
  delay(1000);
  maxTransfer(0x0F, 0x00);
  
  // Enable mode B
  maxTransfer(0x09, 0xFF);
  
  // Use lowest intensity
  maxTransfer(0x0A, 0x00);
  
  // Only scan one digit
  maxTransfer(0x0B, 0x00);
  
  // Turn on chip
  maxTransfer(0x0C, 0x01);
}

void loop() {
  
  // Loop through each code
  for (uint8_t i = 0; i < 0x10; ++i)
  {
    maxTransfer(0x01, i);
    delay(500);
  }
  
}
