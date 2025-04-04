#include "PCF8575.h"

// #include <Servo.h>
// #include "my_servo.h"

#include <CuteBuzzerSounds.h>
#include "my_buzzer.h"

#include "my_io_extender.h"

#include <HT1621.h> // 7 segment display
#include "my_display.h"


#include <FastLED.h>
#include "my_led_strip.h"

uint32_t score_counter = 0;

void setup()
{
  Serial.begin(115200);
  setup_io_extender();
  // setup_servo();
  buzzer_setup();
  display_setup();
  lcd.print(score_counter, 0);
  ledstrip_setup();
}


void loop()
{
  int switch_nr = io_extender_check_switches();
  if (switch_nr == 1){
    cute_buzzer(1);
    score_counter += 10;
    show_leds_reandom();
  }

  if (switch_nr > 0)
  {
    lcd.print(score_counter, 0);
  }

}

