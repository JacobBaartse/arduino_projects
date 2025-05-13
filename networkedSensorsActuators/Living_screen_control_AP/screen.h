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
  // turn off both relays
  digitalWrite(PowerPin, LOW);
  digitalWrite(DirectionPin, LOW);
}

bool screenprocessing(unsigned long currentmilli){
  static unsigned long screenTimer = 0;
  static bool activescreen = false;

  if (activescreen){

  }
  else {
    // calculate direction (up/down) and duration (in ms)
    // screenTimer = currentmilli;
  }

  if (!activescreen){ // turn off the relays
    digitalWrite(PowerPin, LOW);
    digitalWrite(DirectionPin, LOW);
  }
  return activescreen; 
}
