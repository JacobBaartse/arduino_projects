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
{ 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0 },
{ 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 1 },
{ 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1 },
{ 1, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1 },
{ 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0 },
};                                   


int currentSecond;
boolean secondsON_OFF = 1;
int hours, minutes, seconds, year, dayofMon;
String dayofWeek, month;

void displayDigit(int d, int s_x, int s_y){
  for (int i=0;i<3;i++)
    for (int j=0;j<5;j++)
      Time[i+s_x][11-j-s_y] = Digits[j][i+d*3];   
    
  matrix.renderBitmap(Time, 8, 12);
}


DayOfWeek convertDOW(String dow){
  if (dow == String("1")) return DayOfWeek::MONDAY;
  if (dow == String("2")) return DayOfWeek::TUESDAY;
  if (dow == String("3")) return DayOfWeek::WEDNESDAY;
  if (dow == String("4")) return DayOfWeek::THURSDAY;
  if (dow == String("5")) return DayOfWeek::FRIDAY;
  if (dow == String("6")) return DayOfWeek::SATURDAY;
  if (dow == String("7")) return DayOfWeek::SUNDAY;
}

Month convertMonth(String m){
  if (m == String("01")) return Month::JANUARY;
  if (m == String("02")) return Month::FEBRUARY;
  if (m == String("03")) return Month::MARCH;
  if (m == String("04")) return Month::APRIL;
  if (m == String("05")) return Month::MAY;
  if (m == String("06")) return Month::JUNE;
  if (m == String("07")) return Month::JULY;
  if (m == String("08")) return Month::AUGUST;
  if (m == String("09")) return Month::SEPTEMBER;
  if (m == String("10")) return Month::OCTOBER;
  if (m == String("11")) return Month::NOVEMBER;
  if (m == String("12")) return Month::DECEMBER;
}

void getCurTime(String timeSTR,String* d_w,int* d_mn, String* mn,int* h,int* m,int* s,int* y){
  // Serial.println(timeSTR);
  *d_w = timeSTR.substring(0,1);
  *mn = timeSTR.substring(7,9);
  *d_mn = timeSTR.substring(10,12).toInt();
  *h = timeSTR.substring(13,15).toInt();
  *m = timeSTR.substring(16,18).toInt();
  *s = timeSTR.substring(19,21).toInt();
  *y = timeSTR.substring(2,6).toInt();
  // Serial.println(*d_w);
  // Serial.println(*mn);
  // Serial.println(*d_mn);
  // Serial.println(*h);
  // Serial.println(*m);
  // Serial.println(*s);
  // Serial.println(*y);
}


void clock_setup(String timeStamp) {  
  matrix.begin();
  RTC.begin();    
  getCurTime(timeStamp,&dayofWeek,&dayofMon,&month,&hours,&minutes,&seconds,&year);
  RTCTime startTime(dayofMon, convertMonth(month) , year, hours, minutes, seconds, 
                    convertDOW(dayofWeek), SaveLight::SAVING_TIME_ACTIVE); 
  RTC.setTime(startTime);
}

void clock_loop_once(){
  RTCTime currentTime;
  RTC.getTime(currentTime);
  if (currentTime.getSeconds()!=currentSecond){
    secondsON_OFF ?  secondsON_OFF = 0 : secondsON_OFF = 1;
    displayDigit((int)(currentTime.getHour()/10),0,0 );
    displayDigit(currentTime.getHour()%10,4,0 );
    displayDigit((int)(currentTime.getMinutes()/10),1,6 );
    displayDigit(currentTime.getMinutes()%10,5,6 );
    // Time[0][2]=secondsON_OFF;
    // Time[0][4]=secondsON_OFF;
    currentSecond=currentTime.getSeconds();
    matrix.renderBitmap(Time, 8, 12);
    if (debug){
      Serial.println(secondsON_OFF);
    }
  }
}