/*
 * 
 */

#define PowerPin 3
#define DirectionPin 2

#define DirectionUP LOW
#define DirectionDOWN HIGH

void setupScreenControl(){
  pinMode(PowerPin, OUTPUT);
  pinMode(DirectionPin, OUTPUT);

  digitalWrite(PowerPin, LOW);
  digitalWrite(DirectionPin, LOW);
}

bool screenprocessing(unsigned long currentmilli){
  static unsigned long screenTimer = 0;
  static bool activescreen = false;

  return activescreen; 
}
