#include "display_4xdigit.h"

ArduinoLEDMatrix matrix;
int upLeftDigit, upRightDigit, downLeftDigit, downRightDigit;


byte theMatrix[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte  Digits  [5][48]{                // 0 1 2 3 4 5 6 7 8 9 e0 e1 e2 e3 e4  -                                               
{ 0, 1, 0,  0, 0, 1,  1, 1, 0,  1, 1, 0,  1, 0, 1,  1, 1, 1,  0, 1, 1,  1, 1, 1,  0, 1, 0,  0, 1, 1,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0},
{ 1, 0, 1,  0, 1, 1,  0, 0, 1,  0, 0, 1,  1, 0, 1,  1, 0, 0,  1, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 1,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 1,   0, 0, 0},
{ 1, 0, 1,  0, 0, 1,  0, 1, 1,  1, 1, 0,  1, 1, 1,  1, 1, 0,  1, 1, 0,  0, 0, 1,  0, 1, 0,  0, 1, 1,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 1, 0,   0, 1, 0,   1, 1, 1},
{ 1, 0, 1,  0, 0, 1,  1, 0, 0,  0, 0, 1,  0, 0, 1,  0, 0, 1,  1, 0, 1,  0, 1, 0,  1, 0, 1,  0, 0, 1,   0, 0, 0,   0, 0, 0,   0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 0},
{ 0, 1, 0,  0, 0, 1,  1, 1, 1,  1, 1, 0,  0, 0, 1,  1, 1, 0,  0, 1, 0,  1, 0, 0,  0, 1, 0,  1, 1, 0,   0, 0, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 0, 0},
};                                   

void displayDigit(int d, int s_x, int s_y){
  for (int i=0;i<3;i++)
    for (int j=0;j<5;j++)
      theMatrix[i+s_x][11-j-s_y] = Digits[j][i+d*3];   
}

void display_digit_setup() {  
  matrix.begin();
}

void show_digits(int upleft, int upright, int downleft, int downright){
    displayDigit(upleft,0,0 );
    displayDigit(upright,4,0 );
    displayDigit(downleft,1,6 );
    displayDigit(downright,5,6 );
    matrix.renderBitmap(theMatrix, 8, 12);
}

