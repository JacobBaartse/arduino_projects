/*
 *
 */

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

#define PICT_HAPPY 1
#define PICT_HEART_S 2
#define PICT_HEART_B 3
#define PICT_LIKE 4

void loadstaticpicture(int item_id){
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

const uint32_t seqheart[][2] = {
  {
    PICT_HEART_S,
    600
  },
  {
    PICT_HEART_B,
    200
  }
};

#define SEC_NONE 0
#define SEC_HEART 1

bool sequenceBusy = false;
bool runsequence(int sequence_id, int timelength=1000){
  static long runtim = 0;
  if(millis()<runtim) {

  }
  else {
    runtim = millis() + timelength;
  }

  if (sequenceBusy){
    delay(10);
  }
  return ;
}

/*  example timing:
bool BoardLED(int interval){
  static long next_time = 0; // remember the next_time for next action (note static)
  if(millis()<next_time) return false; // if time has not arrived, return
  next_time += interval; // else update next_time by period
  toggleLED(LED_BUILTIN); // and do the action
  return true;
}
