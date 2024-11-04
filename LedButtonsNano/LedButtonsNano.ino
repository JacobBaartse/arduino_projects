/*
 *  Experiment with the buttons with LED included.
 *  Press button and the sequence is: flashing 10 seconds, on 10 seconds, off
 *  
 */

#define buttonPinGreen 7  
#define buttonPinRed 8  
#define ledPinGreen 4  
#define ledPinRed 6  

enum LEDState {
    Off = 0,
    Flashing = 1,
    On = 2,
};

LEDState greenledprocessing(unsigned long curtime, bool buttonpressed) {
  static LEDState ledstatus = LEDState::Off;
  static int ledinterval = 500;
  static int ledstateinterval = 5000;
  static unsigned long ledtime = 0;
  static unsigned long leddurationtime = 0;

  if (buttonpressed) {
    pinMode(ledPinGreen, OUTPUT);
    ledstatus = LEDState::Flashing;
    ledtime = curtime - 2 * ledinterval; // make sure the blinking start directly
    leddurationtime = curtime;
  }

  switch(ledstatus) {
    case LEDState::Flashing:
      if ((unsigned long)(curtime - ledtime) > ledinterval) {
        digitalWrite(ledPinGreen, !digitalRead(ledPinGreen));
        ledtime = curtime;
      }
      break;
    case LEDState::On:
      digitalWrite(ledPinGreen, HIGH);
      break;
    //case LEDState::Off:
    default:
      digitalWrite(ledPinGreen, LOW);
      pinMode(ledPinGreen, INPUT_PULLUP);
  }

  if ((unsigned long)(curtime - leddurationtime) > ledstateinterval) {
    switch(ledstatus) {
      case LEDState::Flashing:
        ledstatus = LEDState::On;
      break;
      // case LEDState::On:
      default:
        ledstatus = LEDState::Off;
    }
    leddurationtime = curtime;
  }
  return ledstatus;
}

LEDState redledprocessing(unsigned long curtime, bool buttonpressed) {
  static LEDState ledstatus = LEDState::Off;
  static int ledinterval = 250;
  static int ledstateinterval = 10000;
  static unsigned long ledtime = 0;
  static unsigned long leddurationtime = 0;

  if (buttonpressed) {
    pinMode(ledPinRed, OUTPUT);
    ledstatus = LEDState::Flashing;
    ledtime = curtime - 2 * ledinterval; // make sure the blinking start directly
    leddurationtime = curtime;
  }

  switch(ledstatus) {
    case LEDState::Flashing:
      if ((unsigned long)(curtime - ledtime) > ledinterval) {
        digitalWrite(ledPinRed, !digitalRead(ledPinRed));
        ledtime = curtime;
      }
      break;
    case LEDState::On:
      digitalWrite(ledPinRed, HIGH);
      break;
    //case LEDState::Off:
    default:
      digitalWrite(ledPinRed, LOW);
      pinMode(ledPinRed, INPUT_PULLUP);
  }

  if ((unsigned long)(curtime - leddurationtime) > ledstateinterval) {
    switch(ledstatus) {
      case LEDState::Flashing:
        ledstatus = LEDState::On;
      break;
      case LEDState::On:
        ledstatus = LEDState::Off;
    }
    leddurationtime = curtime;
  }
  return ledstatus;
}

void setup() {
  // Serial.begin(115200);

  pinMode(buttonPinGreen, INPUT_PULLUP);
  pinMode(buttonPinRed, INPUT_PULLUP);

  pinMode(ledPinGreen, INPUT_PULLUP);
  pinMode(ledPinRed, INPUT_PULLUP);
  //pinMode(ledPinGreen, OUTPUT);
  //pinMode(ledPinRed, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(3000);
  // digitalWrite(LED_BUILTIN, LOW);
}

unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
bool buttonGreenPressed = false;
bool buttonRedPressed = false;
LEDState GreenIndication = LEDState::Off;
LEDState RedIndication = LEDState::Off;

void loop() {
  currentMillis = millis();   // capture the value of millis() only once in teh loop

  GreenIndication = greenledprocessing(currentMillis, buttonGreenPressed);
  RedIndication = redledprocessing(currentMillis, buttonRedPressed);

  if ((RedIndication == LEDState::Off)&&(GreenIndication == LEDState::Off)){
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  buttonGreenPressed = digitalRead(buttonPinGreen) == LOW;
  buttonRedPressed = digitalRead(buttonPinRed) == LOW;

}
