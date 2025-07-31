
#include "simple_matrix.h"
#include "display8x8matrixes.h"

#include "PCF8575.h"

#include "my_io_extender.h"

#include <FastLED.h>
#include "my_led_strip.h"

#include <Arduino.h>
#include <JQ6500_Serial.h>
#include "mp3player.h"

#include<Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "my_servo.h"

// #include "ps2_keyboard.h"
// #include <Adafruit_SH110X.h>  //Adafruit SH110X by Adafruit
#include "SPI.h"
#include "Adafruit_GFX.h"
#include <Fonts/FreeMono18pt7b.h>
#include "Adafruit_ILI9341.h"
#include "touch.h"
// #include "font_16pix_high.h"  //https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
// #include "sh1106_display.h"

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


void next_player(){
  current_player_name = get_player();
  nr_balls_left = NR_BALLS;
  do_servo(0, 0);
  score_counter = 0;
  reset_lefthit();
  showScore();
}

void save_tilt_state()
{
  tilt = true;
}

void setup(){
  setup_8x8matrix();
  Serial.begin(115200);
  setup_io_extender();
  setup_servo();
  // display_setup();
  // lcd.print(score_counter, 0);
  ledstrip_setup();
  setup_mp3_player();
  // Play_mp3_file(INTRO_MELODY);
  // light_show(20000);
  show_leds_rainbow();
  // setup_ps2_keyboard();
  // setup_oled_display();
  // display_oled(true, 0,16, get_top_scores(), true);
  pinMode(TILT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TILT_PIN), save_tilt_state, FALLING);
  next_player();
}

bool left1hit = false;
bool left2hit = false;
bool left3hit = false;

void reset_lefthit(){
    left1hit = false;
    left2hit = false;
    left3hit = false;
}

long keep_matrix_millies = 0;

void showScore(){
  // lcd.print(score_counter, 0);
  score_onleds(left1hit, left2hit, left3hit);

  if (millis() > keep_matrix_millies){
    String screen_text = current_player_name;
    while (screen_text.length() < 10) screen_text += " ";
    screen_text += String(score_counter);
    while (screen_text.length() < 16) screen_text += " ";
    screen_text += String("   Balls: ") + String(nr_balls_left);
    disp_8x8_matrix.print(screen_text.c_str());
  }

}

long cannon_millies;

void loop(){
  int switch_nr = io_extender_check_switches();
  if (switch_nr == 15){  // BALL ON DECK
    long speed = millis() - cannon_millies;
    debug("speed : ");
    debugln(speed);
    Play_mp3_file(KOEKOEK_KLOK);
    if (speed < 400) score_counter += (400 - speed);

    String speed_text = String("speed: ")+ String((float)972 / speed) + String("Km/h");
    disp_8x8_matrix.print(speed_text.c_str());
    keep_matrix_millies = millis() + 10000;

    light_show(2825);
  }
  if (switch_nr == 2){  // ramp down
    Play_mp3_file(DO_RE_MI);
    score_counter += 100;
    light_show(1600);
  }
  if (switch_nr == 3){  // CANNON 
    cannon_millies = millis();
    Play_mp3_file(CANNON_SHOT);
    do_servo(0, 60);
    score_counter += 100;
  }  
  if (switch_nr == 4){
    nr_balls_left --;
    showScore();
    Play_mp3_file(JAMMER);  // BALL OUT
    if (nr_balls_left == 0){
      store_score(score_counter);
      blink_all_leds(5000);
      next_player();
    }
    else{
      light_show(4000);
    }
  }  
  if (switch_nr == 10){  //red button
    Play_mp3_file(GUN_SHOT);
    reset_lefthit();
    do_servo(0, 0);
    score_counter = 0;
    // display_oled(true, 0,16, get_top_scores(), true);
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
    do_servo(0, 0);
  }

  if (tilt){ // tilt contact
    Play_mp3_file(TOE_TOKKK);
    // display_oled(true, 0,16, String("TILT...\nNext player"), true);
    disp_8x8_matrix.print(tilt_text);
    blink_all_leds(5000);
    next_player();
    tilt = false;
  }

  if (switch_nr > 0){
    debug("switch number: ");
    debugln(switch_nr);
    showScore();
    if (switch_nr == 10) show_leds_rainbow();
  }

  // int keyboard_char = get_keyboard_char();
  // if ((keyboard_char != 0)  && (keyboard_char != 0xAA))
  // {
  //   oled_screen_text = menu_process_key(keyboard_char);
  //   display_oled(true, 0,16, oled_screen_text, true);
  // }
  delay(50);

}

#undef debug
#undef debugln