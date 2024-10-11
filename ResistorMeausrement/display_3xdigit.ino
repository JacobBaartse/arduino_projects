#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET     -1 

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


void print_matrix(){
  for (int i =0; i<8; i++)
  {
    for (int j=0; j<12; j++)
      Serial.print(theMatrix[i][j]);
    Serial.println();
  }
  Serial.println("------------------");
}


const int minus = 15;
const int space_digit = 10;
byte  Digits  [5][48]{                // 0 1 2 3 4 5 6 7 8 9 e0 e1 e2 e3 e4  -                                               
{ 0, 1, 0,  0, 0, 1,  1, 1, 0,  1, 1, 0,  1, 0, 1,  1, 1, 1,  0, 1, 1,  1, 1, 1,  0, 1, 0,  0, 1, 1,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0},
{ 1, 0, 1,  0, 1, 1,  0, 0, 1,  0, 0, 1,  1, 0, 1,  1, 0, 0,  1, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 1,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 1,   0, 0, 0},
{ 1, 0, 1,  0, 0, 1,  0, 1, 1,  1, 1, 0,  1, 1, 1,  1, 1, 0,  1, 1, 0,  0, 0, 1,  0, 1, 0,  0, 1, 1,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 1, 0,   0, 1, 0,   1, 1, 1},
{ 1, 0, 1,  0, 0, 1,  1, 0, 0,  0, 0, 1,  0, 0, 1,  0, 0, 1,  1, 0, 1,  0, 1, 0,  1, 0, 1,  0, 0, 1,   0, 0, 0,   0, 0, 0,   0, 0, 1,   0, 0, 1,   0, 0, 1,   0, 0, 0},
{ 0, 1, 0,  0, 0, 1,  1, 1, 1,  1, 1, 0,  0, 0, 1,  1, 1, 0,  0, 1, 0,  1, 0, 0,  0, 1, 0,  1, 1, 0,   0, 0, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 0, 0},
};      

const int k = 0;
const int M = 1;
const int space_letter = 2;
byte Letters [3][15]{
  { 1, 0, 1, 0, 0,  1, 0, 0, 0, 1,  0, 0, 0, 0, 0, },
  { 1, 1, 0, 0, 0,  1, 1, 0, 1, 1,  0, 0, 0, 0, 0, },
  { 1, 0, 1, 0, 0,  1, 0, 1, 0, 1,  0, 0, 0, 0, 0, },
};


void displayDigit(int d, int s_x, int s_y){
  for (int x=0;x<3;x++)
    for (int y=0;y<5;y++){
      theMatrix[y+s_y][x+s_x] = Digits[y][x+ d*3];     
    }
}

void displayLetter(int d, int s_x, int s_y){
  for (int x=0;x<5;x++)
    for (int y=0;y<3;y++){
      theMatrix[y+s_y][x+s_x] = Letters[y][x+ d*5];     
    }
}

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool display_found = false;

void display_digit_setup() {  

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display_found = false;
  Wire.beginTransmission(SCREEN_ADDRESS);
  byte result = Wire.endTransmission(1);
  if (result == 0) display_found = true;


  if (display_found){
      display.clearDisplay();
      display.setFont(&FreeMono9pt7b);
      display.setTextSize(1);
      display.setTextColor(WHITE);
  }
  matrix.begin();
}

const char multiplier [] = "KM ";
void oled_display(String text, int letter){ 
    if (display_found) {
      display.clearDisplay();
      display.setCursor(0,12);
      display.print(text + " " + multiplier[letter]);
      display.setCursor(0,27);
      display.print("       ohm");
      display.display();
    }
}

void show_value(float value){
  if (value<0) value=0;
  int letter = space_letter;
  int dot_pos = 3;
  if (value > 999.5){
    letter = k;
    value /= 1000;
    }
  if (value > 999.5){
    letter = M;
    value /= 1000;
  }
  if (value > 1000){
    displayDigit(minus,0,0);
    displayDigit(minus,4,0);
    displayDigit(minus,8,0);
    displayLetter(space_letter, 5, 5);
    if (!display_found) matrix.renderBitmap(theMatrix, 8, 12);
    oled_display("Max", space_letter);


    delay(800);
    }
    else{
      oled_display(String(value), letter);
      if (value < 100){
        value *= 10;
        dot_pos -=1;
      }
      if(value < 100){
        value *= 10;
        dot_pos -=1;
      }
      displayDigit((int)value/100 % 10,0,0 );
      displayDigit((int)value/10 % 10,4,0 );
      displayDigit((int)value % 10,8,0 );
      displayLetter(letter, 7, 5);


      for (int i=0; i<4; i++){
        theMatrix[4][dot_pos*4 -1]=1;
        if (!display_found) matrix.renderBitmap(theMatrix, 8, 12);
        delay(250);
        theMatrix[4][dot_pos*4 -1]=0;
        if (!display_found) matrix.renderBitmap(theMatrix, 8, 12);
        delay(250);
      }
    }
}

