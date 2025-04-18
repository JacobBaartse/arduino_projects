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
  show_leds_rainbow();
}

bool left1hit = false;
bool left2hit = false;
bool left3hit = false;

void reset_lefthit(){
    left1hit = false;
    left2hit = false;
    left3hit = false;
}

void loop()
{
  int switch_nr = io_extender_check_switches();
  if (switch_nr == 1){
    show_leds_reandom(1, false);
    cute_buzzer(1);
    score_counter += 10;
    show_leds_reandom(10, true);
  }
  if (switch_nr == 2){
    show_leds_rainbow();
    cute_buzzer(2);
    score_counter += 100;
    delay(300);
    show_leds_reandom(0, true);
  }
  if (switch_nr == 3){
    show_leds_reandom(1, false);
    cute_buzzer(3);
    score_counter += 100;
    show_leds_reandom(10, true);
  }  
  if (switch_nr == 10){  //red button
    reset_lefthit();
    cute_buzzer(4);
    score_counter = 0;
  }  
  if (switch_nr == 9){  //green button
    cute_buzzer(5);
    //score_counter = 0;
  }
  if (switch_nr == 11){  //left side button 1
    left1hit = true;
    score_onleds(left1hit, left2hit, left3hit);
    cute_buzzer(6);
    blink_leds(left1hit, false, false);
    score_counter += 5;
  }
  if (switch_nr == 12){  //left side button 2
    left2hit = true;
    score_onleds(left1hit, left2hit, left3hit);
    cute_buzzer(7);
    blink_leds(false, left2hit, false);
    score_counter += 5;
  }
  if (switch_nr == 13){  //left side button 3
    left3hit = true;
    score_onleds(left1hit, left2hit, left3hit);
    cute_buzzer(8);
    blink_leds(false, false, left3hit);
    score_counter += 5;
  }

  if (left1hit & left2hit & left3hit){
    score_onleds(left1hit, left2hit, left3hit);
    score_counter += 100;
    blink_leds(left1hit, left2hit, left3hit);
    cute_buzzer(9);
    reset_lefthit();
  }

  if (switch_nr > 0)
  {
    Serial.println(switch_nr);
    lcd.print(score_counter, 0);
    score_onleds(left1hit, left2hit, left3hit);
    if (switch_nr == 10) show_leds_rainbow();
  }
  delay(2);

}

