/*
never exceed the measured input voltage of +/- 30 volt this could harm the arduino input.
always use a powerbank that supplies a good 5V to the arduino for this project.
*/

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
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
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


const int minus = 11;
const int space_digit = 10;
byte  Digits  [5][36]{                // 0 1 2 3 4 5 6 7 8 9   -                                               
{ 0, 1, 0,  0, 0, 1,  1, 1, 0,  1, 1, 0,  1, 0, 1,  1, 1, 1,  0, 1, 1,  1, 1, 1,  0, 1, 0,  0, 1, 1,    0, 0, 0,    0, 0, 0},
{ 1, 0, 1,  0, 1, 1,  0, 0, 1,  0, 0, 1,  1, 0, 1,  1, 0, 0,  1, 0, 0,  0, 0, 1,  1, 0, 1,  1, 0, 1,    0, 0, 0,    0, 0, 0},
{ 1, 0, 1,  0, 0, 1,  0, 1, 1,  1, 1, 0,  1, 1, 1,  1, 1, 0,  1, 1, 0,  0, 0, 1,  0, 1, 0,  0, 1, 1,    0, 0, 0,    1, 1, 1},
{ 1, 0, 1,  0, 0, 1,  1, 0, 0,  0, 0, 1,  0, 0, 1,  0, 0, 1,  1, 0, 1,  0, 1, 0,  1, 0, 1,  0, 0, 1,    0, 0, 0,    0, 0, 0},
{ 0, 1, 0,  0, 0, 1,  1, 1, 1,  1, 1, 0,  0, 0, 1,  1, 1, 0,  0, 1, 0,  1, 0, 0,  0, 1, 0,  1, 1, 0,    0, 0, 0,    0, 0, 0},
};      

const int k_ohm = 0;
const int M_ohm = 1;
const int space_ohm = 2;
const int V_plus = 3;
const int V_min = 4;

byte Letters [3][60]{  // k_ohm  m_ohm  ohm  +v  -v
  { 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0,  0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0,  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0,  0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1,  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, },
  { 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0,  0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0,  0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0,  0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1,  0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, },
  { 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1,  0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1,  0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1,  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,},
};


void displayDigit(int d, int s_x, int s_y){
  for (int x=0;x<3;x++)
    for (int y=0;y<5;y++){
      theMatrix[y+s_y][x+s_x] = Digits[y][x+ d*3];     
    }
}

void displayLetter(int d, int s_x, int s_y){
  for (int x=0;x<12;x++)
    for (int y=0;y<3;y++){
      theMatrix[y+s_y][x+s_x] = Letters[y][x+ d*12];     
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
void oled_display(String resistance, int letter, float voltage){ 
    if (display_found) {
      display.clearDisplay();
      display.setCursor(0,12);
      display.print(resistance +  multiplier[letter] + " Ohm");
      display.setCursor(0,27);
      display.print((String)voltage + " Volt");
      display.display();
    }
}

int letter = 0;

float range_to_meg_kilo(float resistance){
  if (resistance > 5000000) resistance = 2000000000;  // above 5M is not possible to measure so show Max ohm  ---
  if (resistance<0) resistance=0;
  letter = space_ohm;
  int dot_pos = 3;
  if (resistance > 999.5){
    letter = k_ohm;
    resistance /= 1000;
    }
  if (resistance > 999.5){
    letter = M_ohm;
    resistance /= 1000;
  }
  return resistance;
}

void show_values_oled(float resistance, int letter, float voltage){
    if (resistance >= 1000) oled_display("Max", space_ohm, voltage);
    else                    oled_display(String(resistance), letter, voltage);
}

void out_of_range(){
  displayDigit(minus,0,0);
  displayDigit(minus,4,0);
  displayDigit(minus,8,0);
  displayLetter(space_ohm, 0, 5);
  matrix.renderBitmap(theMatrix, 8, 12);
  delay(200);
}

void blink_dot(int dot_pos){
  for (int i=0; i<4; i++){
    theMatrix[4][dot_pos*4 -1]=1;
    matrix.renderBitmap(theMatrix, 8, 12);
    delay(150);
    theMatrix[4][dot_pos*4 -1]=0;
    matrix.renderBitmap(theMatrix, 8, 12);
    delay(150);
  }
}

void show_voltage_matrix(float voltage){
  int dot_pos = 2;
  if (voltage > 0) displayLetter(V_plus, 0, 5);
  else displayLetter(V_min, 0, 5);      
  if (voltage < 10){
    voltage *= 10;
    dot_pos = 1;
  }
  voltage = abs(voltage);
  displayDigit((int)voltage/10 % 10,0,0 );
  displayDigit((int)voltage/1 % 10,4,0 );
  displayDigit((int)(voltage*10) % 10,8,0 );
  blink_dot(dot_pos);
}

void show_resistance_matrix(float resistance, int letter){
  int dot_pos = 3;
  if (resistance < 100){
    resistance *= 10;
    dot_pos -=1;
  }
  if(resistance < 100){
    resistance *= 10;
    dot_pos -=1;
  }
  displayDigit((int)resistance/100 % 10,0,0 );
  displayDigit((int)resistance/10 % 10,4,0 );
  displayDigit((int)resistance % 10,8,0 );
  displayLetter(letter, 0, 5);
  blink_dot(dot_pos);
}

void show_values_matrix(float resistance, int letter, float voltage){
  if (resistance >= 1000){
    if (abs(voltage) < 0.1){
      out_of_range();
      return;
    }
    show_voltage_matrix(voltage); 
  }
  else{
    show_resistance_matrix(resistance, letter);
  }
}

void show_value(float resistance, float voltage){
  resistance = range_to_meg_kilo(resistance);
  if (display_found) show_values_oled(resistance, letter, voltage);
  else show_values_matrix(resistance, letter, voltage);
}

