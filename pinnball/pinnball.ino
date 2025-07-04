#include "PCF8575.h"

#include <Servo.h>
#include "my_servo.h"

#include "my_io_extender.h"

#include <HT1621.h> // 7 segment display
#include "my_display.h"


#include <FastLED.h>
#include "my_led_strip.h"

#include <Arduino.h>
#include <JQ6500_Serial.h>
#include "mp3player.h"

#include<Wire.h>
// #include "tilt_sensor.h"

#include "ps2_keyboard.h"
#include <Adafruit_SH110X.h>  //Adafruit SH110X by Adafruit
#include <Adafruit_GFX.h>
#include "font_16pix_high.h"  //https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
#include "sh1106_display.h"

#include <EEPROM.h>
#include "menu.h"

#define NR_BALLS 4
#define TILT_PIN 3

#define DEBUG 0

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

String oled_screen_text = "_";

uint32_t score_counter = 0;
int nr_balls_left = NR_BALLS;
String current_player_name = "";
bool tilt = false;

void show_user_and_balls(){
  display_oled(true, 0,16, current_player_name+ String("\nBalls: ") + String(nr_balls_left), true);
}

void next_player(){
  current_player_name = get_player();
  nr_balls_left = NR_BALLS;
  do_servo(9, 0);
  score_counter = 0;
  reset_lefthit();
  showScore();
  show_user_and_balls();
}

void save_tilt_state()
{
  tilt = true;
}

void setup(){
  Serial.begin(115200);
  setup_io_extender();
  setup_servo();
  display_setup();
  lcd.print(score_counter, 0);
  ledstrip_setup();
  setup_mp3_player();
  Play_mp3_file(INTRO_MELODY);
  light_show(20000);
  show_leds_rainbow();
  setup_ps2_keyboard();
  setup_oled_display();
  display_oled(true, 0,16, get_top_scores(), true);
  pinMode(TILT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TILT_PIN), save_tilt_state, FALLING);
}

bool left1hit = false;
bool left2hit = false;
bool left3hit = false;

void reset_lefthit(){
    left1hit = false;
    left2hit = false;
    left3hit = false;
}

void showScore(){
  lcd.print(score_counter, 0);
  score_onleds(left1hit, left2hit, left3hit);
}

long cannon_micros;

void loop(){
  int switch_nr = io_extender_check_switches();
  if (switch_nr == 15){  // BALL ON DECK
    long speed = millis() - cannon_micros;
    debug("speed : ");
    debugln(speed);
    Play_mp3_file(KOEKOEK_KLOK);
    if (speed < 400) score_counter += (400 - speed);
    display_oled(true, 0,16, String("speed: ")+ String((float)900 / speed) + String("Km/h") , true);
    light_show(2825);
  }
  if (switch_nr == 2){  // UNUSED FOR NOW
    Play_mp3_file(DO_RE_MI);
    score_counter += 100;
    light_show(1600);
  }
  if (switch_nr == 3){  // CANNON 
    cannon_micros = millis();
    Play_mp3_file(CANNON_SHOT);
    do_servo(9, 60);
    score_counter += 100;
  }  
  if (switch_nr == 4){
    Play_mp3_file(JAMMER);  // BALL OUT
    nr_balls_left --;
    if (nr_balls_left == 0){
      store_score(score_counter);
      display_oled(true, 0,16, String(score_counter) + String("\nBalls: 0"), true);
      blink_all_leds(5000);
      next_player();
    }
    else{
      show_user_and_balls();
      light_show(4000);
    }
  }  
  if (switch_nr == 10){  //red button
    Play_mp3_file(GUN_SHOT);
    reset_lefthit();
    do_servo(9, 0);
    score_counter = 0;
    display_oled(true, 0,16, get_top_scores(), true);
  }  
  if (switch_nr == 9){  //green button
    next_player();
    // Play_mp3_file(KLIK_KLAK);
  }
  if (switch_nr == 11){  //left side button 1
    left1hit = true;
    Play_mp3_file(PRRRR);
    score_onleds(left1hit, left2hit, left3hit);
    blink_leds(left1hit, false, false, 2000);
    score_counter += 5;
  }
  if (switch_nr == 12){  //left side button 2
    left2hit = true;
    Play_mp3_file(HIGH_PING);
    score_onleds(left1hit, left2hit, left3hit);
    blink_leds(false, left2hit, false, 1400);
    score_counter += 5;
  }
  if (switch_nr == 13){  //left side button 3
    left3hit = true;
    Play_mp3_file(OLD_TELEPHONE_RING);
    score_onleds(left1hit, left2hit, left3hit);
    blink_leds(false, false, left3hit, 5000);
    score_counter += 5;
  }
  if (switch_nr == 1){ // ROTARY SENSOR
    Play_mp3_file(Y1_KORT_PR);
    light_show(100);
    score_counter += 9;
  }  

  if (left1hit & left2hit & left3hit){
    Play_mp3_file(SUPER_GOOD);
    score_onleds(left1hit, left2hit, left3hit);
    score_counter += 100;
    blink_leds(left1hit, left2hit, left3hit, 2000);
    reset_lefthit();
    do_servo(9, 0);
  }

  if (tilt){ // tilt contact
    Play_mp3_file(TOE_TOKKK);
    display_oled(true, 0,16, String("TILT...\nNext player"), true);
    blink_all_leds(10000);
    next_player();
    tilt = false;
  }

  if (switch_nr > 0){
    debug("switch number: ");
    debugln(switch_nr);
    showScore();
    if (switch_nr == 10) show_leds_rainbow();
  }

  int keyboard_char = get_keyboard_char();
  if ((keyboard_char != 0)  && (keyboard_char != 0xAA))
  {
    oled_screen_text = menu_process_key(keyboard_char);
    display_oled(true, 0,16, oled_screen_text, true);
    if (oled_screen_text == "") show_user_and_balls();
  }
  delay(2);

}

#undef debug
#undef debugln