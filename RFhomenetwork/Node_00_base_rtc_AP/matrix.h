/*
 *
 */

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

const PROGMEM uint32_t full_on[] = {
    0xffffffff,
    0xffffffff,
    0xffffffff
};

const PROGMEM uint32_t full_off[] = {
    0x00,
    0x00,
    0x00
};

#define PICT_HAPPY 1
#define PICT_HEART_S 2
#define PICT_HEART_B 3
#define PICT_LIKE 4

void loadstaticpicture(int item_id){
  //Serial.print("loadstaticpicture: ");
  //Serial.println(item_id);  
  switch (item_id) {
  case PICT_HAPPY:
    matrix.loadFrame(LEDMATRIX_EMOJI_HAPPY);
    break;
  case PICT_HEART_S:
    matrix.loadFrame(LEDMATRIX_HEART_SMALL);
    break;
  case PICT_HEART_B:
    matrix.loadFrame(LEDMATRIX_HEART_BIG);
    break;
  case PICT_LIKE:
  default:
    matrix.loadFrame(LEDMATRIX_LIKE);
  }
}

const PROGMEM int sequencing[][2] = {
  {
    PICT_HEART_S,
    600
  },
  {
    PICT_HEART_B,
    200
  },
  {
    PICT_HEART_S,
    600
  },
  {
    PICT_HEART_B,
    200
  },
  {
    PICT_HEART_S,
    600
  },
  {
    PICT_HEART_B,
    200
  },
  {
    PICT_HEART_S,
    600
  },
  {
    PICT_HEART_B,
    200
  },
    {
    PICT_HEART_S,
    600
  },
  {
    PICT_HEART_B,
    200
  },
  {
    PICT_HAPPY,
    1000
  },
  {
    PICT_LIKE,
    800
  }
};

void loadsequencepicture(unsigned long ftime){
  static unsigned long runningtime = 0;
  static int pointer = 0;
  if (ftime < runningtime) return;
  loadstaticpicture(sequencing[pointer][0]);
  runningtime = (unsigned long) ftime + sequencing[pointer][1];
  pointer = (pointer + 1) % 12;
}

void startupscrollingtext(String starttext){
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(100);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(starttext);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
}

void displaystatictext(String statictext){
  matrix.beginDraw();

  matrix.textFont(Font_4x6);
  matrix.beginText(1, 1, 0xFFFFFF);
  matrix.println(statictext);
  matrix.endText();

  matrix.endDraw();
}
 
void displayFull(bool on=true){
  if (on) matrix.loadFrame(full_on);
  else matrix.loadFrame(full_off);
}

bool alarmingsequence(unsigned long ftime){
  static unsigned long alarmtime = 0;
  static int alarmcount = 0;
  static bool fullred = false;
  if (ftime < alarmtime) return true;
  fullred = !fullred;
  displayFull(fullred);
  alarmcount++;
  alarmtime = (unsigned long) ftime + 500;
  return alarmcount < 20; // flash maximum 10 times
}
