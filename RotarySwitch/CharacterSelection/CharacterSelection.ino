#include <Adafruit_SSD1306.h>  //Adafruit SSD 1306 by Adafruit
#include <Adafruit_GFX.h>
#include "font_16pix_high.h"
#include "display_32x128.h"

#include <RotaryEncoder.h>

#define PIN_IN1 3
#define PIN_IN2 4

RotaryEncoder encoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR0);


void setup() {
  setup_display();
  // put your setup code here, to run once:
  pinMode(4, INPUT);
  Serial.begin(115200);
  display.clearDisplay();
}

// Read the current position of the encoder and print out when changed.
void loop()
{
  static int pos = 0;
  encoder.tick();

  long newPos = encoder.getPosition();
  if (pos != newPos) {
    Serial.print("pos:");
    Serial.print(newPos);
    Serial.print(" dir:");
    Serial.println((int)(encoder.getDirection()));
    int charval = ( ((abs(newPos) % 95) + 0x21) % 95) + 0x20;

    char string_to_send[2];
    string_to_send[0] = charval;
    string_to_send[1] = '\0'; // nul to terminate the string.
    display.clearDisplay();
    display.setCursor(0,16);
    display.write( string_to_send );
    display.display();
    pos = newPos;
  } // if
} // loop ()
