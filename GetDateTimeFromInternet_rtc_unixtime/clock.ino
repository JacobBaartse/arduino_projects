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
{ 0, 1, 0,  0, 0, 1,  1, 1, 0,  1, 1, 0,  1, 0, 1,  1, 1, 1,  0, 1, 1,  1, 1, 1,  0, 1, 0,  0, 1, 1,   },
{ 1, 0, 1,  0, 1, 1,  0, 0, 1,  0, 0, 1,  1, 0, 1,  1, 0, 0,  1, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 1,   },
{ 1, 0, 1,  0, 0, 1,  0, 1, 1,  1, 1, 0,  1, 1, 1,  1, 1, 0,  1, 1, 0,  0, 0, 1,  0, 1, 0,  0, 1, 1,   },
{ 1, 0, 1,  0, 0, 1,  1, 0, 0,  0, 0, 1,  0, 0, 1,  0, 0, 1,  1, 0, 1,  0, 1, 0,  1, 0, 1,  0, 0, 1,   },
{ 0, 1, 0,  0, 0, 1,  1, 1, 1,  1, 1, 0,  0, 0, 1,  1, 1, 0,  0, 1, 0,  1, 0, 0,  0, 1, 0,  1, 1, 0,   },
};                                   

void displayDigit(int d, int s_x, int s_y){
  for (int i=0;i<3;i++)
    for (int j=0;j<5;j++)
      Time[i+s_x][11-j-s_y] = Digits[j][i+d*3];   
    
  matrix.renderBitmap(Time, 8, 12);
}

byte Seconds_pos [2][30]{
  {0,  0,  0 ,0 ,0 ,0 ,0 ,0 ,0 ,0 ,0, 0, 4,  4,  4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  },
  {11, 10, 9, 8, 7, 6, 5, 4, 3, 2 ,1 ,0, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2 ,1 ,0, }
};

void displaySecond(int second){

}


unsigned long startup_unix_time_rtc;
unsigned long unix_time_rtc;
RTCTime currentTime;
int Minutes = 0;
int Hour = 0;
int Seconds = -1;

void clock_setup() {  
  matrix.begin();
  RTC.begin();
}

void(* restart_uno) (void) = 0; //declare reset function @ address 0

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


int prev_second;
const int hour = 60*60;

void clock_loop_once(){
  RTC.getTime(currentTime);
  if (currentTime.getSeconds() != prev_second){
    //Serial.println("clock loop once");
    prev_second = currentTime.getSeconds();
    unsigned long unix_time = currentTime.getUnixTime();
    unsigned long elapsed_seconds = unix_time - startup_unix_time_rtc;
    elapsed_seconds -= elapsed_seconds/53;
    elapsed_seconds += elapsed_seconds/3400;

    unsigned long corrected_unix_time = startup_unix_time_rtc + elapsed_seconds;
    Minutes = (corrected_unix_time / 60) % 60;
    Hour = (corrected_unix_time / hour) % 12;
    Seconds = corrected_unix_time % 60;
    Serial.print(Hour);
    Serial.print(":");
    Serial.print(Minutes);
    Serial.print(":");
    Serial.println(Seconds);
    Serial.println("---------");

    // update the display
    displayDigit((int)(Hour/10),0,0 );
    displayDigit(Hour%10,4,0 );
    displayDigit((int)(Minutes/10),1,6 );
    displayDigit(Minutes%10,5,6 );
    displaySecond(Seconds);
    matrix.renderBitmap(Time, 8, 12);

    // displayDigit((int)(Minutes/10),0,0 );
    // displayDigit(Minutes%10,4,0 );
    // displayDigit((int)(Seconds/10),1,6 );
    // displayDigit(Seconds%10,5,6 );
    // matrix.renderBitmap(Time, 8, 12);
  
    if (currentTime.getHour()== 4 && (millis()/1000/hour)>6) restart_uno();  //restart every night at 4 a clock once.
  }



}