#include "clock.h"

ArduinoLEDMatrix matrix;
bool debug = false;


byte Time[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte  Digits  [5][30]{                                                                 
{ 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1 },
{ 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1 },
{ 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1 },
{ 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1 },
{ 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0 },
};                                   

void displayDigit(int d, int s_x, int s_y){
  for (int i=0;i<3;i++)
    for (int j=0;j<5;j++)
      Time[i+s_x][11-j-s_y] = Digits[j][i+d*3];   
    
  matrix.renderBitmap(Time, 8, 12);
}


unsigned long startup_unix_time_rtc;
unsigned long unix_time_rtc;
RTCTime currentTime;
int Minutes = 0;
int Hour = 0;

void clock_setup() {  
  matrix.begin();
  RTC.begin();
}

void set_clock(unsigned long unix_time){
  startup_unix_time_rtc = unix_time;
  Serial.println("set_clock");
  Serial.println(unix_time);
  currentTime.setUnixTime(unix_time);
  RTC.setTime(currentTime);
  RTC.getTime(currentTime);
  Serial.println(currentTime);
  Serial.println("----------");
}

String get_clock(){
  return String(Hour) + ":" + String(Minutes);
}

int prev_minuut;
int prev_second;
const int hour = 60*60;

void clock_loop_once(){
  RTC.getTime(currentTime);
  if (currentTime.getSeconds() != prev_second){
    Serial.println("clock loop once");
    prev_second = currentTime.getSeconds();
    // correct timing
    unsigned long unix_time = currentTime.getUnixTime();
    Serial.println(unix_time);
    long elapsed_seconds = unix_time - startup_unix_time_rtc;
    Serial.println("elapsed_seconds");
    Serial.println(elapsed_seconds);
    elapsed_seconds += elapsed_seconds/60;
    Serial.println(elapsed_seconds); 
    unsigned long corrected_unix_time = startup_unix_time_rtc + elapsed_seconds;
    Serial.println("corrected_unix_time");
    Serial.println(corrected_unix_time);
    Minutes = (corrected_unix_time / 60) % 60;
    Serial.println(Minutes);
    Hour = (corrected_unix_time / hour) % 12;
    Serial.println(Hour);
    Serial.println("---------");
  }
  if (prev_minuut != Minutes)
  {
    prev_minuut = Minutes;
    // update the display
    displayDigit((int)(Hour/10),0,0 );
    displayDigit(Hour%10,4,0 );
    displayDigit((int)(Minutes/10),1,6 );
    displayDigit(Minutes%10,5,6 );
    matrix.renderBitmap(Time, 8, 12);
  }
}