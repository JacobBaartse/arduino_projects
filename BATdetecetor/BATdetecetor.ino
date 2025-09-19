/*
 * NANO with ultrasound receiver coneceted to interrupt pin, buzzer and builtin LED
 */

#define DETECT_PIN 2
#define TRIG_PIN 4
#define BUZZER_PIN 6

bool activeDetect = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
    delay(10);
  }
  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  pinMode(DETECT_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(TRIG_PIN, LOW); 

  //attachInterrupt(digitalPinToInterrupt(DETECT_PIN), batDetect, RISING); // trigger when ultrasound detected
  attachInterrupt(digitalPinToInterrupt(DETECT_PIN), batDetect, FALLING); // trigger when ultrasound detected

  Serial.println();  
  Serial.println(F(" ***************"));  
  Serial.println(); 
  Serial.flush(); 
}

void trigmodule(unsigned long timingmoment){
  static unsigned long trigtiming = 0;

  if ((unsigned long)(timingmoment - trigtiming) > 300){ // trig
    trigtiming = timingmoment; 
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG_PIN, HIGH); 
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW); 
  }
}

const uint16_t PROGMEM pitchlist[] = { 2500, 1500, 3000, 2000, 1000, 0 };

void processdetection(bool detect, unsigned long timingmoment){
  static bool alarming = false;
  static uint16_t tonepitch = 0;
  static unsigned long detecttiming = 0;

  if (alarming){ // maximum 1 second
    if ((unsigned long)(timingmoment - detecttiming) > 1000){ // stop alarm
      noTone(BUZZER_PIN);
      alarming = false;
      digitalWrite(LED_BUILTIN, LOW);
      activeDetect = false;
      Serial.print(F("Stop alarm: "));
      Serial.println(timingmoment);
    }
  }
  else{
    if (detect){
      alarming = true;
      detecttiming = timingmoment; 
      digitalWrite(LED_BUILTIN, HIGH);
      tone(BUZZER_PIN, pitchlist[tonepitch++]); // Send sound signal...
      if (pitchlist[tonepitch] == 0){
        tonepitch = 0;
      }
      Serial.print(F("Start alarm: "));
      Serial.println(timingmoment);
    }
    else {
      trigmodule(timingmoment);
    }
  }
}

unsigned long runtiming = 0;
bool detectsound = false;

void loop() {

  runtiming = millis();

  if (activeDetect)
  {
    activeDetect = false;
    detectsound = true;
  }
  processdetection(detectsound, runtiming);

  detectsound = false;

  //delay(10);
}

void batDetect(){
  activeDetect = true;

  // if (!activeDetect){
  //   activeDetect = true;
  //   // Serial.print(F("Detection: "));
  //   // Serial.println(millis());
  // }
}
