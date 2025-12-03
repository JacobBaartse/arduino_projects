#include <FastLED.h>
#include "my_led_strip.h"


void setup() {
  ledstrip_setup();

}

int effect;
long start_millis;
long duration;

long prev = 0;
void loop() {


  // for (int effect=1; effect<MAX_EFFECT; )
  int effect = LIGHT_GOES_UP;
  {
    long now = millis();
    if (now > prev + 8000){
      start_millis = millis();
      duration = 6000;
      prev = now;
      // effect++;
    }
    pattern_on_ledstrip(effect, start_millis, duration);
    FastLED.show();
  } 

}
