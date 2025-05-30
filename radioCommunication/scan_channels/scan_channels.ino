/*
 * Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 * Updated 2020 TMRh20
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * https://nrf24.github.io/RF24/examples_2scanner_2scanner_8ino-example.html
 */

/*
 * How to read the output:
 * - The header is a list of supported channels in decimal written vertically.
 * - Each column corresponding to the vertical header is a hexadecimal count of
 *   detected signals (max is 15 or 'f').
 *
 * The following example
 *    000
 *    111
 *    789
 *    ~~~   <- just a divider between the channel's vertical labels and signal counts
 *    1-2
 * can be interpreted as
 * - 1 signal detected on channel 17
 * - 0 signals (denoted as '-') detected on channel 18
 * - 2 signals detected on channel 19
 *
 * Each line of signal counts represent 100 passes of the supported spectrum.
 */

#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// external NRF24 module
// #define CE_PIN 7
// #define CSN_PIN 8

// RF Nano
// #define CE_PIN 10
// #define CSN_PIN 9

// RF Nano, microUSB
#define CE_PIN 9
#define CSN_PIN 10

// SMD projects:
// #define CE_PIN 9
// #define CSN_PIN 10

// instantiate an object for the nRF24L01 transceiver
RF24 radio(CE_PIN, CSN_PIN);

//
// Channel info
//

const uint8_t num_channels = 126;  // 0-125 are supported
uint8_t values[num_channels];      // the array to store summary of signal counts per channel

// To detect noise, we'll use the worst addresses possible (a reverse engineering tactic).
// These addresses are designed to confuse the radio into thinking
// that the RF signal's preamble is part of the packet/payload.
const uint8_t noiseAddress[][2] = { { 0x55, 0x55 }, { 0xAA, 0xAA }, { 0xA0, 0xAA }, { 0xAB, 0xAA }, { 0xAC, 0xAA }, { 0xAD, 0xAA } };

const int num_reps = 100;   // number of passes for each scan of the entire spectrum
bool constCarrierMode = false;  // this flag controls example behavior (scan mode is default)

void printHeader();  // prototype function for printing the channels' header

void setup(void) {
  // Print preamble
  Serial.begin(115200);
  //Serial.begin(230400); // for the 2 micro USB NANO boards
  //Serial.begin(230400); // actual baudrate in IDE: 57600 (RF-NANO, micro USB), there is somewhere a mismatch in clock factor of 4

  while (!Serial) {
    // some boards need this to wait for Serial connection
  }
  Serial.println(F("RF24/examples/scanner/"));
  Serial.print(__FILE__);
  Serial.print(F(",\ncreation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush();

  Serial.print(F("CE_PIN: "));
  Serial.print(CE_PIN);
  Serial.print(F(", CSN_PIN: "));
  Serial.println(CSN_PIN);

  // Setup and configure RF radio
  if (!radio.begin()) {
    Serial.println(F("Radio hardware not responding!"));
    while (true) {
      delay(1000);
      // hold in an infinite loop
    }
  }
  radio.stopConstCarrier();  // in case MCU was reset while radio was emitting carrier wave
  radio.setAutoAck(false);   // Don't acknowledge arbitrary signals
  radio.disableCRC();        // Accept any signal we find
  radio.setAddressWidth(2);  // A reverse engineering tactic (not typically recommended)
  for (uint8_t i = 0; i < 6; ++i) {
    radio.openReadingPipe(i, noiseAddress[i]);
  }

  // set the data rate
  uint8_t dataRate = 0; // set some default

  // Serial.print(F("Select your Data Rate. "));
  // Serial.print(F("Enter '1' for 1 Mbps, '2' for 2 Mbps, '3' for 250 kbps. "));
  // Serial.println(F("Defaults to 1Mbps."));
  // while (!Serial.available()) {
  //   // wait for user input
  // }
  // dataRate = Serial.parseInt();
  if (dataRate == 50) {
    Serial.println(F("Using 2 Mbps."));
    radio.setDataRate(RF24_2MBPS);
  } else if (dataRate == 51) {
    Serial.println(F("Using 250 kbps."));
    radio.setDataRate(RF24_250KBPS);
  } else {
    Serial.println(F("Using 1 Mbps."));
    radio.setDataRate(RF24_1MBPS);
  }
  // Serial.println(F("***Enter a channel number to emit a constant carrier wave."));
  // Serial.println(F("***Enter a negative number to switch back to scanner mode."));

  // Get into standby mode
  radio.startListening();
  radio.stopListening();
  radio.flush_rx();

  // printf_begin();
  // radio.printPrettyDetails();
  // delay(1000);

  // Print out vertical header
  printHeader();
}

void loop(void) {

  /****************************************/
  // Send a number over Serial to begin Constant Carrier Wave output
  // Configure the power amplitude level below
  if (Serial.available()) {
    int8_t c = Serial.parseInt();
    if (c >= 0) {
      c = min((int8_t)125, c);  // clamp channel to supported range
      constCarrierMode = true;
      radio.stopListening();
      delay(2);
      Serial.print(F("\nStarting Carrier Wave Output on channel "));
      Serial.println(c);
      // for non-plus models, startConstCarrier() changes address on pipe 0 and sets address width to 5
      radio.startConstCarrier(RF24_PA_LOW, c);
    } else {
      constCarrierMode = false;
      radio.stopConstCarrier();
      radio.setAddressWidth(2);                   // reset address width
      radio.openReadingPipe(0, noiseAddress[0]);  // ensure address is looking for noise
      Serial.println(F("\nStopping Carrier Wave Output"));
      printHeader();
    }

    // discard any CR and LF sent
    while (Serial.peek() != -1) {
      if (Serial.peek() == '\r' || Serial.peek() == '\n') {
        Serial.read();
      } else {  // got a charater that isn't a line feed
        break;  // handle it on next loop() iteration
      }
    }
  }

  /****************************************/

  if (!constCarrierMode) {
    // Clear measurement values
    memset(values, 0, sizeof(values));

    // Scan all channels num_reps times
    int rep_counter = num_reps;
    while (rep_counter--) {
      int i = num_channels;
      while (i--) {
        // Select this channel
        radio.setChannel(i);

        // Listen for a little
        radio.startListening();
        delayMicroseconds(128);
        bool foundSignal = radio.testRPD();
        radio.stopListening();

        // Did we get a signal?
        if (foundSignal || radio.testRPD() || radio.available()) {
          ++values[i];
          radio.flush_rx();  // discard packets of noise
        }
      }
    }

    // Print out channel measurements, clamped to a single hex digit
    for (int i = 0; i < num_channels; ++i) {
      if (values[i])
        Serial.print(min((uint8_t)0xf, values[i]), HEX);
      else
        Serial.print(F("-"));
    }
    Serial.println();

  }  // if !constCarrierMode
  else {
    // show some output to prove that the program isn't bricked
    Serial.print(F("."));
    delay(1000);  // delay a second to keep output readable
  }
}  // end loop()

void printHeader() {
  // Print the hundreds digits
  for (uint8_t i = 0; i < num_channels; ++i)
    Serial.print(i / 100);
  Serial.println();

  // Print the tens digits
  for (uint8_t i = 0; i < num_channels; ++i)
    Serial.print((i % 100) / 10);
  Serial.println();

  // Print the singles digits
  for (uint8_t i = 0; i < num_channels; ++i)
    Serial.print(i % 10);
  Serial.println();

  // Print the header's divider
  for (uint8_t i = 0; i < num_channels; ++i)
    Serial.print(F("~"));
  Serial.println();
}
