/*
 * NANO with ultrasound receiver connected to interrupt pin 2 and pin 4, buzzer and builtin LED
 */

#define DETECT_PIN2 2
#define DETECT_PIN3 3
#define TRIG_PIN 4
#define BUZZER_PIN 6

bool trigger = false;
unsigned long starttime = 0;
unsigned long stoptime = 0;

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

  pinMode(DETECT_PIN2, INPUT);
  pinMode(DETECT_PIN3, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(TRIG_PIN, LOW); 

  attachInterrupt(digitalPinToInterrupt(DETECT_PIN2), EchoStart, RISING); // trigger when echo signal starts
  attachInterrupt(digitalPinToInterrupt(DETECT_PIN3), EchoStop, FALLING); // trigger when echo signal stops

  Serial.println();  
  Serial.println(F(" ***************"));  
  Serial.println(); 
  Serial.flush(); 
}

unsigned long timeduraton(){
  unsigned long duration = 0;
  if (stoptime > 0){ // calculate difference
    if (stoptime > starttime){ 
      duration = (unsigned long)(stoptime - starttime);
    }

    Serial.print(F("stoptime: "));
    Serial.print(stoptime);  
    Serial.print(F(", starttime: "));
    Serial.print(starttime);
    Serial.print(F(", duration: "));
    Serial.print(duration);
    Serial.println(F(" ms."));

    starttime = 0;
    stoptime = 0;
  }
  return duration;
}

void trigmodule(bool triggering){
  static unsigned long triggertiming = 0;

  if (triggering){ // trigger the ultrasone module
    digitalWrite(TRIG_PIN, LOW);
    //delayMicroseconds(2);
    delay(9);
    triggertiming = millis();
    starttime = triggertiming;
    stoptime = 0;
    digitalWrite(TRIG_PIN, HIGH); 
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW); 

    // Serial.print(F("Trigger: "));
    // Serial.println(triggertiming);
  }
  else{
    //if (stoptime > 0) {
      if ((unsigned long)(millis() - triggertiming) > 2000){ // for debugging
        starttime = 0;
        stoptime = 0;
      }
    //}
  }
}

const uint8_t pitchcount = 5;
const uint16_t pitchlist[pitchcount] = { 2500, 1500, 3000, 2000, 1000}; //, 0 };

uint16_t getTone(){
  static uint16_t tonepitch = 0;
  //uint16_t toneval; // = 1000;

  uint16_t toneval = pitchlist[tonepitch++];
  if (tonepitch >= pitchcount){
    tonepitch = 0;
  }
  return toneval;
}

bool processdetection(){
  static bool alarming = false;
  static unsigned long detecttiming = 0;
  uint16_t tonefreq = 1000;
  bool ready = false;

  if (alarming){ // maximum 1 second
    if ((unsigned long)(millis() - detecttiming) > 1000){ // stop alarm
      noTone(BUZZER_PIN);
      alarming = false;
      digitalWrite(LED_BUILTIN, LOW);
      // Serial.print(F("Stop alarm: "));
      // Serial.println(timingmoment);
    }
  }
  else{
    unsigned long detectduration = timeduraton();
    if ((detectduration > 0)&&(detectduration < 35)){ // smaller 
      alarming = true;
      detecttiming = millis(); 
      digitalWrite(LED_BUILTIN, HIGH);
      tonefreq = getTone();
      tone(BUZZER_PIN, tonefreq); // Send sound signal...
      Serial.print(F("Pulse: "));
      Serial.print(detectduration);      
      Serial.print(F(", start alarm: "));
      Serial.print(detecttiming);
      Serial.print(F(", tonefreq: "));
      Serial.println(tonefreq);
    }
    else{
      ready = starttime == 0;
    }
  }
  return ready;
}

void loop() {

  trigger = processdetection();

  trigmodule(trigger);

}

void EchoStart(){
  starttime = millis();
}

void EchoStop(){
  stoptime = millis();
}
