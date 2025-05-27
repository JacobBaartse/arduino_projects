/*
 * http://wiki.sunfounder.cc/index.php?title=2_Channel_5V_Relay_Module
 * 
 */

#define OnPin1 5
#define OnPin2 6
#define off_delay_time 10000 // 10 seconds delay time before going off

enum RelayState {
    R_Off = 0,
    R_On = 1,
    R_Off_wait = 2,
};


RelayState handleRelay(unsigned long currentmilli, RelayState Raction){
  static RelayState RelayStatus = RelayState::R_Off;
  static unsigned long offTimer = 0;

  switch(RelayStatus){
    case RelayState::R_Off:
      if (Raction == RelayState::R_On){
        digitalWrite(OnPin1, LOW);
        digitalWrite(OnPin2, LOW);
        RelayStatus = RelayState::R_On;
      }
      break;
    case RelayState::R_On:
      if (Raction == RelayState::R_Off_wait){
        offTimer = currentmilli + off_delay_time;
        RelayStatus = RelayState::R_Off_wait;
      }
      break;
    case RelayState::R_Off_wait:
      if (Raction == RelayState::R_On){
        digitalWrite(OnPin1, LOW);
        digitalWrite(OnPin2, LOW);
        RelayStatus = RelayState::R_On;
      }
      else if (currentmilli > offTimer){
        digitalWrite(OnPin1, HIGH);
        digitalWrite(OnPin2, HIGH);
        RelayStatus = RelayState::R_Off;
      }
      break;
    default:
      Serial.print(currentmilli);
      Serial.println(F(" handleRelay"));
  }

  return RelayStatus;
}

void setuprelays(){
  pinMode(OnPin1, OUTPUT);
  pinMode(OnPin2, OUTPUT);

  digitalWrite(OnPin1, HIGH);
  digitalWrite(OnPin2, HIGH);

  handleRelay(0, RelayState::R_Off);
}
