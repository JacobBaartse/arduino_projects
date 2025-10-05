/*
 * https://eletechsup.com/products/dc-12v-10a-latching-keep-relay-module-zero-power-hold-switch-bistable-self-locking-flip-flop-latch-board
 * 
 */

#define FeedbackPin 4
#define ClearPin 5
#define SetPin 6

#define pulse_time 2100  // pulse low time, max. 2 seconds needed

enum RelayState {
    R_None = 0,
    R_Off = 1,
    R_GoOn = 2,
    R_On = 3,
    R_GoOff = 4
};

RelayState commandaction = RelayState::R_None;

RelayState handleRelay(unsigned long currentmilli, RelayState Raction){
  static RelayState RelayStatus = RelayState::R_None;
  static unsigned long pulseTimer = 0;

  switch(RelayStatus){
    case RelayState::R_Off:
      if (Raction == RelayState::R_On){
        digitalWrite(SetPin, LOW);
        pulseTimer = (unsigned long) currentmilli + pulse_time;
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
        pulseTimer = (unsigned long) currentmilli + pulse_time;
        RelayStatus = RelayState::R_GoOff;
      }
      break;
    case RelayState::R_GoOff:
      if (currentmilli > pulseTimer){
        digitalWrite(ClearPin, HIGH);
        RelayStatus = RelayState::R_Off;
      }
      break;
    case RelayState::R_None: // this is the case of initial setup
      RelayStatus = Raction;
    default:
      Serial.print(currentmilli);
      Serial.println(F(" handleRelay"));
  }

  return RelayStatus;
}

uint8_t relaysdetectstate = HIGH;

void setuprelays(){
  pinMode(FeedbackPin, INPUT_PULLUP);
  pinMode(ClearPin, OUTPUT);
  pinMode(SetPin, OUTPUT);

  digitalWrite(ClearPin, HIGH);
  digitalWrite(SetPin, HIGH);

  delay(2500); // make sure a possible pulse is completed

  relaysdetectstate = digitalRead(FeedbackPin);
  if (relaysdetectstate == HIGH){
    handleRelay(0, RelayState::R_Off);
  }
  else{
    handleRelay(0, RelayState::R_On);
  }
}

uint8_t relaytracking(bool fresh){
  unsigned long currentmilli = millis();
  uint8_t trackval = 0;

  // check state agains actual monitoring
  RelayState RelStat = handleRelay(currentmilli, RelayState::R_None);
  if ((RelStat == RelayState::R_On)||(RelStat == RelayState::R_Off)){
    relaysdetectstate = digitalRead(FeedbackPin);
    if (RelStat == RelayState::R_On){
      if (relaysdetectstate != LOW){
        // Serial.print(currentmilli);
        // Serial.println(F(" relaytrackig 1")); 
        trackval = 1;     
      }
    }
    else{ // (RelStat == RelayState::R_Off)
      if (relaysdetectstate != HIGH){
        // Serial.print(currentmilli);
        // Serial.println(F(" relaytrackig 2"));    
        trackval = 2;  
      }
    }
  }
  else{ // action to go on or go off is still in progress
    trackval = 3; // fresh command cannot be handled yet
  }

  if ((fresh)&&(commandaction != RelayState::R_None)){
    RelStat = handleRelay(currentmilli, commandaction); // start the action
    trackval = 4; // new command
    commandaction = RelayState::R_None;
  }

  return trackval;
}


// // Define pin connections
// const int relaySetPin = 7;  // Connect to the S pin of the LR12A01 module
// const int relayResetPin = 8; // Connect to the R pin of the LR12A01 module

// void setup() {
//   // Initialize the relay pins as outputs
//   pinMode(relaySetPin, OUTPUT);
//   pinMode(relayResetPin, OUTPUT);

//   // Start with the relay in a known state (e.g., released)
//   digitalWrite(relayResetPin, LOW); // Send a low pulse to reset
//   delay(1); // Short delay to ensure the pulse is registered
//   digitalWrite(relayResetPin, HIGH); // Set back to high to end the pulse
// }

// void loop() {
//   // --- Example: Toggle relay on and off with button presses ---
//   // For a real application, you'd use a button or sensor to trigger these actions.
//   // This is just a demonstration of the pulse logic.

//   // To "Set" the relay (pull it to one state)
//   Serial.println("Setting relay...");
//   digitalWrite(relaySetPin, LOW); // Send a low pulse to SET the relay
//   delay(200); // Hold the pulse for a short duration (e.g., 200ms)
//   digitalWrite(relaySetPin, HIGH); // Set back to high to end the pulse
//   Serial.println("Relay set.");
//   delay(2000); // Wait for 2 seconds

//   // To "Reset" the relay (pull it to the other state)
//   Serial.println("Resetting relay...");
//   digitalWrite(relayResetPin, LOW); // Send a low pulse to RESET the relay
//   delay(200); // Hold the pulse for a short duration (e.g., 200ms)
//   digitalWrite(relayResetPin, HIGH); // Set back to high to end the pulse
//   Serial.println("Relay reset.");
//   delay(2000); // Wait for 2 seconds

//   // Note: The duration of the pulse should be brief, 0.1-2 seconds is recommended [2].
// }