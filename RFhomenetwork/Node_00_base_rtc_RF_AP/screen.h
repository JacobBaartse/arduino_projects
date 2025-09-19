/*
 * connect parallel to the Somfy driver (1 realys for up and 1 relays for down)
 */

#define ScreenUp 3
#define ScreenDown 2
#define MonitorUp 3
#define MonitorDown 2

void setupScreenControl(){
  pinMode(ScreenUp, OUTPUT);
  pinMode(ScreenDown, OUTPUT);

  pinMode(MonitorUp, INPUT);
  pinMode(MonitorDown, INPUT);

  // turn off both relays                   !!!!!!!!!!!! check electrical connections
  digitalWrite(ScreenDown, HIGH);
  digitalWrite(ScreenUp, HIGH);
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
    digitalWrite(ScreenDown, LOW);
    digitalWrite(ScreenUp, LOW);
  }
  return activescreen; 
}
