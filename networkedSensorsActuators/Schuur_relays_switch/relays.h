/*
 * http://wiki.sunfounder.cc/index.php?title=2_Channel_5V_Relay_Module
 * 
 */

#define ClearPin 3
#define SetPin 2
#define FeedbackPin 4

#define pulse_time 2100  // pulse low time, max 2 seconds needed

enum RelayState {
    R_None = 0,
    R_Off = 1,
    R_GoOn = 2,
    R_On = 3,
    R_GoOff = 4
};


RelayState handleRelay(unsigned long currentmilli, RelayState Raction){
  static RelayState RelayStatus = RelayState::R_None;
  static unsigned long pulseTimer = 0;

  switch(RelayStatus){
    case RelayState::R_Off:
      if (Raction == RelayState::R_On){
        digitalWrite(SetPin, LOW);
        pulseTimer = currentmilli + pulse_time;
        RelayStatus = RelayState::R_GoOn;
      }
      break;
    case RelayState::R_GoOn:
      if (currentmilli > pulseTimer){
        digitalWrite(SetPin, HIGH);
        RelayStatus = RelayState::R_On;
      }
      break;
    case RelayState::R_On:
      if (Raction == RelayState::R_Off){
        digitalWrite(ClearPin, LOW);
        pulseTimer = currentmilli + pulse_time;
        RelayStatus = RelayState::R_GoOff;
      }
      break;
    case RelayState::R_GoOff:
      if (currentmilli > pulseTimer){
        digitalWrite(ClearPin, HIGH);
        RelayStatus = RelayState::R_Off;
      }
      break;
    case RelayState::R_None: // in case of initial setup
      RelayStatus = Raction;
    default:
      Serial.print(currentmilli);
      Serial.println(F(" handleRelay"));
  }

  return RelayStatus;
}

void setuprelays(){
  pinMode(ClearPin, OUTPUT);
  pinMode(SetPin, OUTPUT);
  pinMode(FeedbackPin, INPUT_PULLUP);

  digitalWrite(ClearPin, HIGH);
  digitalWrite(SetPin, HIGH);

  delay(500); // make sure a possible pulse is completed

  int startupstate = digitalRead(FeedbackPin);
  if (startupstate == LOW){
    handleRelay(0, RelayState::R_Off);
  }
  else{
    handleRelay(0, RelayState::R_On);
  }
}
