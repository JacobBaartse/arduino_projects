/*
 *  Experiment with the buttons with LED included.
 *
 *  Press button and the sequence is: flashing interval Y milliseconds for X seconds, ON for X seconds, OFF
 *  Press button again: off directly
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

const int debounceDelay = 50;

bool greendebounce(bool pressedbutton, unsigned long curtime){
  static bool buttonstate = false;
  static unsigned long lastdebouncetime = 0;

  if (pressedbutton != buttonstate) {
    if ((unsigned long)(curtime - lastdebouncetime) > debounceDelay) {
      buttonstate = pressedbutton;
      // lastdebouncetime = curtime;
    }
    lastdebouncetime = curtime;
  }
  return buttonstate;
}

bool reddebounce(bool pressedbutton, unsigned long curtime){
  static bool buttonstate = false;
  static unsigned long lastdebouncetime = 0;

  if (pressedbutton != buttonstate) {
    if ((unsigned long)(curtime - lastdebouncetime) > debounceDelay) {
      buttonstate = pressedbutton;
      // lastdebouncetime = curtime;
    }
    lastdebouncetime = curtime;
  }
  return buttonstate;
}

LEDState greenledprocessing(unsigned long curtime, bool buttonpressed) {
  static LEDState ledstatus = LEDState::Off;
  static unsigned long ledinterval = 500;
  static unsigned long ledstateinterval = 5000;
  static unsigned long ledtime = 0;
  static unsigned long leddurationtime = 0;
  static bool previousbuttonpressed = false;

  if (buttonpressed) {
    if (buttonpressed != previousbuttonpressed) { // only start once
      if (ledstatus == LEDState::Off) {
        pinMode(ledPinGreen, OUTPUT);
        ledstatus = LEDState::Flashing;
        ledtime = (unsigned long)(curtime - 2 * ledinterval); // make sure the blinking start directly
        leddurationtime = curtime;
      }
      else { 
        ledstatus = LEDState::Off;
      }
    }
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
      // case LEDState::Off:
      // case LEDState::On:
      default:
        ledstatus = LEDState::Off;
    }
    leddurationtime = curtime;
  }

  previousbuttonpressed = buttonpressed;
  return ledstatus;
}

LEDState redledprocessing(unsigned long curtime, bool buttonpressed) {
  static LEDState ledstatus = LEDState::Off;
  static int ledinterval = 250;
  static int ledstateinterval = 10000;
  static unsigned long ledtime = 0;
  static unsigned long leddurationtime = 0;
  static bool previousbuttonpressed = false;

  if (buttonpressed) {
    if (buttonpressed != previousbuttonpressed) { // only start once
      if (ledstatus == LEDState::Off) {
        pinMode(ledPinRed, OUTPUT);
        ledstatus = LEDState::Flashing;
        ledtime = (unsigned long)(curtime - 2 * ledinterval); // make sure the blinking start directly
        leddurationtime = curtime;
      }
      else { 
        ledstatus = LEDState::Off;
      }
    }
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
      // case LEDState::Off:
      // case LEDState::On:
      default:
        ledstatus = LEDState::Off;
    }
    leddurationtime = curtime;
  }

  previousbuttonpressed = buttonpressed;
  return ledstatus;
}

void setup() {
  //Serial.begin(115200);

  pinMode(buttonPinGreen, INPUT_PULLUP);
  pinMode(buttonPinRed, INPUT_PULLUP);

  pinMode(ledPinGreen, INPUT_PULLUP);
  pinMode(ledPinRed, INPUT_PULLUP);
  //pinMode(ledPinGreen, OUTPUT);
  //pinMode(ledPinRed, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
bool buttonGreenPressed = false;
bool buttonRedPressed = false;
LEDState GreenIndication = LEDState::Off;
LEDState RedIndication = LEDState::Off;

void loop() {

  currentMillis = millis();   // capture the value of millis() only once in the loop

  GreenIndication = greenledprocessing(currentMillis, buttonGreenPressed);
  RedIndication = redledprocessing(currentMillis, buttonRedPressed);

  if ((RedIndication == LEDState::Off)&&(GreenIndication == LEDState::Off)){
    digitalWrite(LED_BUILTIN, LOW);
    //Serial.println(F("Builtin LED OFF"));
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
    //Serial.println(F("Builtin LED ON"));
  }

  buttonGreenPressed = greendebounce((digitalRead(buttonPinGreen) == LOW), currentMillis);
  buttonRedPressed = reddebounce((digitalRead(buttonPinRed) == LOW), currentMillis);

}
