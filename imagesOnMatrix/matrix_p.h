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

const int seqheart[][2] = {
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

int definedSequences[] = { SEC_NONE, SEC_HEART };

bool sequenceBusy = false;
bool runsequence(int sequence_id, int timelength=3000){
  static long runtim = 0;
  static int pointer = 0;
  static int loopcount = 0;
  int next_time = 0;
  if(millis()<runtim) {
    return true;
  }
  else {
    //Serial.print("- ");
    //Serial.print(pointer);
    //Serial.print(" p, l: ");
    //Serial.println(loopcount);
    if (pointer == 0){
      loopcount++;
    }
    //Serial.print("sequence_id: ");
    //Serial.println(sequence_id);
    switch (sequence_id) {
    case SEC_HEART:
      loadstaticpicture(seqheart[pointer][0]);
      next_time = seqheart[pointer][1];
      pointer++;
      pointer = pointer % 2; // sizeof(seqheart);  // length of the array
      break;
    case SEC_NONE:
    default:
      next_time = timelength;
      //sequenceBusy = false;
    }
    runtim = millis() + next_time;
  }

  if (loopcount > 10){
    loopcount = 0;
    sequenceBusy = false;
  }

  // dit moet later weg
  delay(200);

  //Serial.print("sequenceBusy: ");
  //Serial.println(sequenceBusy);
  return sequenceBusy;
}
