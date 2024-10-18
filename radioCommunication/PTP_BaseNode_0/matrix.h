/*
 *
 */

#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

/*
const uint32_t happy[] = {
    0x19819,
    0x80000001,
    0x81f8000
};

const uint32_t heart[] = {
    0x3184a444,
    0x44042081,
    0x100a0040
};
/* */

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

const int sequencing[][2] = {
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

void loadsequencepicture(){
  static long runningtime = 0;
  static int pointer = 0;
  if(millis() < runningtime) return;
  loadstaticpicture(sequencing[pointer][0]);
  runningtime = millis() + sequencing[pointer][1];
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
