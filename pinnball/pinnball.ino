
#include <FastLED.h>
#include "my_led_strip.h"

#include "simple_matrix.h"
#include "display8x8matrixes.h"

#include "PCF8575.h"
#include "my_io_extender.h"


#include <Arduino.h>
#include <JQ6500_Serial.h>
#include "mp3player.h"

#include<Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "my_servo.h"

#include "SPI.h"
#include "Adafruit_GFX.h"
#include <Fonts/FreeMono18pt7b.h>
#include "Adafruit_ILI9341.h"
#include "touch.h"
#include "OnScreenKeyboard.h"

#include <EEPROM.h>
#include "menu.h"

#define NR_BALLS 4
#define TILT_PIN 3
#define SKIP_CONTACT_BOUNCE 100

#define DEBUG 0

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

bool reset_servo = false;
bool reset_right_hits_done = false;
String menu_text = "_";

uint32_t score_counter = 0;
uint32_t prev_score_counter = 0;
int nr_balls_left = NR_BALLS;
String current_player_name = "";
bool tilt = false;


void next_player(){
  current_player_name = get_player(false);
  nr_balls_left = NR_BALLS;
  do_servo(0, 0);
  score_counter = 0;
  prev_score_counter = 0;
  reset_left_hit();
  reset_right_hit();
  showScore();
  show_text_on_screen(get_top_scores());
}

void save_tilt_state()
{
  tilt = true;
}


long cannon_millis;
int effect;
long start_millis;
long duration;
bool intro_melody=true;

void setup(){
  Serial.begin(115200);
  setup_8x8matrix();
  onScreen_keyboard_setup();
  setup_io_extender();
  setup_servo();
  ledstrip_setup();
  setup_mp3_player();
  Play_mp3_file(INTRO_MELODY);
  effect = SCROLING_RAINBOW;
  start_millis = millis();
  duration = 20000;
  show_text_on_screen(get_top_scores());
  pinMode(TILT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TILT_PIN), save_tilt_state, FALLING);
  next_player();
}

long keep_speed_millis = 0;
String speed_text = "";

void showScore(){

    String screen_text = current_player_name;
    while (screen_text.length() < 10) screen_text += " ";
    screen_text += String(score_counter);
    while (screen_text.length() < 16) screen_text += " ";
  if (millis() > keep_speed_millis){
    screen_text += String("   Balls: ") + String(nr_balls_left) + ".";
    disp_8x8_matrix.print(screen_text.c_str());
  }
  else{
    screen_text += speed_text;
    disp_8x8_matrix.print(screen_text.c_str());  
  }
}

void loop(){
  int switch_nr = io_extender_check_switches();
  if (switch_nr > 0){
    if (intro_melody){
      duration = 0;
      reset_mp3();
      intro_melody = false;
    } 
  }
  if (switch_nr == 15){  // BALL in the top left corner
    delay(SKIP_CONTACT_BOUNCE);
    long speed = millis() - cannon_millis;
    debug("speed : ");
    debugln(speed);
    Play_mp3_file(KOEKOEK_KLOK);
    effect = CANNON_SHOT_LEDS;
    start_millis = millis();
    duration = 4000;
    
    if (speed < 600){
      score_counter += (600 - speed);
      speed_text = String("speed: ")+ String((float)972 / speed) + String("Km/h");
      keep_speed_millis = millis() + 10000;
    }
    score_counter += 50;
    delay(200); // pevent contact ossilation.
  }
  if (switch_nr == 2){  // high ramp
    Play_mp3_file(DO_RE_MI);
    score_counter += 250;
    effect = SCROLING_RAINBOW;
    start_millis = millis();
    duration = 4000;
  }
  if (switch_nr == 3){  // CANNON 
    cannon_millis = millis();
    Play_mp3_file(CANNON_SHOT);
    do_servo(0, 60);
    score_counter += 100;
  }  
  if (switch_nr == 4){
    if (score_counter > 0){
      if (score_counter > prev_score_counter){
        nr_balls_left --;
      }
    }
    showScore();
    prev_score_counter = score_counter;

    if (nr_balls_left == 0){
      Play_mp3_file(HIGH_ANOTHERONE);  //all BALLs OUT
      Play_mp3_file(LOW_ANOTHERONE);
      Play_mp3_file(HIGH_ANOTHERONE);
      Play_mp3_file(LOW_ANOTHERONE);
      
      store_score(score_counter);
      blink_all_leds(5000);
      next_player();
    }
    else{
      Play_mp3_file(JAMMER);  // BALL OUT
      blink_all_leds_blue_red(5000);
    }
  }  
  if (switch_nr == 10){  //red button
    Play_mp3_file(GUN_SHOT);
    reset_left_hit();
    reset_right_hit();
    do_servo(0, 0);
    score_counter = 0;
    prev_score_counter = 0;
    nr_balls_left = NR_BALLS;
    show_text_on_screen(get_top_scores());
  }  
  if (switch_nr == 9){  //green button
    next_player();
    // Play_mp3_file(KLIK_KLAK);
  }
  if (switch_nr == 11){  //left side button 1
    left1hit = true;
    left1blink_until = millis() + 4000;  // delays also the reset of the servo
    Play_mp3_file(PRRRR);
    score_counter += 5;
    effect = SPARKLING;
    start_millis = millis();
    duration = 4000;
  }
  if (switch_nr == 12){  //left side button 2
    left2hit = true;
    Play_mp3_file(HIGH_PING);
    left2blink_until = millis() + 4000;  // delays also the reset of the servo
    score_counter += 5;
    effect = FADE_IN_OUT;
    start_millis = millis();
    duration = 4000;
  }
  if (switch_nr == 13){  //left side button 3
    left3hit = true;
    Play_mp3_file(OLD_TELEPHONE_RING);
    left3blink_until = millis() + 4000;  // delays also the reset of the servo
    score_counter += 5;
    effect = SCROLING_RAINBOW;
    start_millis = millis();
    duration = 4000;
  }
   if (switch_nr == 14){  // top ball 
    Play_mp3_file(KIP);
    score_counter += 25;
    effect = SPARKLING_COLORS;
    start_millis = millis();
    duration = 4000;
  }
  
  if (switch_nr == 7){  //right side button Joker
    right4hit = true;
    Play_mp3_file(TADI_TADIADI);
    // nr_balls_left++;
    right4blink_until = millis() + 2000;
    score_counter += 50;
    effect = FADE_IN_OUT;
    start_millis = millis();
    duration = 4000;
  }

  if (switch_nr == 8){   //right side button klaver 10
    right3hit = true;
    right3blink_until = millis() + 2000;
    Play_mp3_file(PAARD);
    score_counter += 5;
    effect = SPARKLING;
    start_millis = millis();
    duration = 4000;
  }
  if (switch_nr == 5){  //right side button klaver 9
    right2hit = true;
    Play_mp3_file(KREKEL);
    right2blink_until = millis() + 2000;
    score_counter += 5;
    effect = SCROLING_RAINBOW;
    start_millis = millis();
    duration = 4000;
  }
  if (switch_nr == 6){  //right side button klaver 8
    right1hit = true;
    Play_mp3_file(KLONGNGNG);
    right1blink_until = millis() + 2000;
    score_counter += 5;
    effect = SCROLING_RAINBOW;
    start_millis = millis();
    duration = 4000;
  }
  

  if (switch_nr == 1){ // ROTARY SENSOR
    Play_mp3_file(Y1_KORT_PR);
    effect = RUN_AROUND;
    start_millis = millis();
    duration = 1000;
    score_counter += 4;
  }  

  if (left1hit & left2hit & left3hit){
    if (!reset_servo) { // prevent double counting of points
      score_counter += 100;
      nr_balls_left++;
      Play_mp3_file(SUPER_GOOD);
    }
    reset_servo =  true;// prevent double counting of points
  }
  else{
    if (reset_servo & (!left1hit | !left2hit | !left3hit)){
      do_servo(0, 0);
      reset_servo = false;
    }
  }

  if (right1hit & right2hit & right3hit & right4hit){
    if (!reset_right_hits_done) { // prevent double counting of points
      score_counter += 100;
      nr_balls_left++;
      Play_mp3_file(SUPER_GOOD);
    }
    reset_right_hits_done =  true;// prevent double counting of points
  }
  else{
    if (reset_right_hits_done & (!right1hit |!right2hit |!right3hit |!right4hit)){
      reset_right_hits_done = false;
    }
  }


  if (tilt){ // tilt contact
    delay(SKIP_CONTACT_BOUNCE);
    Play_mp3_file(HIGH_ANOTHERONE);
    Play_mp3_file(LOW_ANOTHERONE);
    Play_mp3_file(HIGH_ANOTHERONE);
    Play_mp3_file(LOW_ANOTHERONE);
    disp_8x8_matrix.print(tilt_text);
    blink_all_leds(5000);
    next_player();
    tilt = false;
  }

  if (switch_nr > 0){
    debug("switch number: ");
    debugln(switch_nr);
    showScore();

  }

  if (switch_nr == 17)
  {
    do_menu();
    show_text_on_screen(get_top_scores());
  }

  pattern_on_ledstrip(effect, start_millis, duration);
  score_onleds();
  FastLED.show();

  TS_Point p = get_touch(false); // non blocking
  static byte prev_volume_level = 15;
  if (p.x>0)
  {
    byte volume_level = (byte) (p.x/115);
    if (volume_level>30) volume_level = 30;
      if (volume_level != prev_volume_level){
        mp3.setVolume(volume_level);
        delay(10);
        Play_mp3_file(Y1_KORT_PR);
        prev_volume_level = volume_level;
      }
  }
  delay(1);
    
}

#undef debug
#undef debugln