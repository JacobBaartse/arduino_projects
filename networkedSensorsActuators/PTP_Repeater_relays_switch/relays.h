/*
 * https://eletechsup.com/products/dc-12v-10a-latching-keep-relay-module-zero-power-hold-switch-bistable-self-locking-flip-flop-latch-board
 * 
 */

#define ClearPin 3;
#define SetPin 2;
#define FeedbackPin 4;

const uint16_t pulse_time = 2100;  // pulse low time 

enum RelayState {
    R_None = 0,
    R_Off = 1,
    R_GoOn = 2,
    R_On = 3,
    R_GoOff = 4
};

void setuprelays(){
  pinMode(ClearPin, OUTPUT);
  pinMode(SetPin, OUTPUT);
  pinMode(FeedbackPin, INPUT_PULLUP);

  digitalWrite(ClearPin, HIGH);
  digitalWrite(SetPin, HIGH);
}

unsigned int handleRelays(unsigned long currentmilli, RelayState Raction){
  static RelayState RelayState = RelayState::R_Off;
  //static unsigned long sendingTimer = 0;
  static unsigned long pulseTimer = 0;

  switch(Raction){
    case RelayState::R_Off:
      break;
    case RelayState::R_GoOn:
      break;
    case RelayState::R_On:
      break;
    case RelayState::R_GoOff:
      break;
    //case LEDState::R_None:
    default:
  }



}
