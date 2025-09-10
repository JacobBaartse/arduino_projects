/*
 * NANO with ultrasound receiver coneceted to interrupt pin, buzzer and builtin LED
 */

#define DETECT_PIN 2
#define BUZZER_PIN 8

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

  pinMode(DETECT_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  attachInterrupt(digitalPinToInterrupt(DETECT_PIN), batDetect, RISING); // trigger when ultrasound detected

  Serial.println();  
  Serial.println(F(" ***************"));  
  Serial.println(); 
  Serial.flush(); 
}
 
void processdetection(bool detect, unsigned long timingmoment){
  static bool alarming = false;
  static unsigned long detecttiming = 0;

  if (alarming){ // maximum 1 second
    if ((unsigned long)(timingmoment - detecttiming) > 1000){ // stop alarm
      noTone(BUZZER_PIN);
      alarming = false;
      digitalWrite(LED_BUILTIN, LOW);
      activeDetect = false;
    }
  }
  else{
    if (detect){
      alarming = true;
      detecttiming = timingmoment; 
      digitalWrite(LED_BUILTIN, HIGH);
      tone(BUZZER_PIN, 2500); // Send 1KHz sound signal...
    }
  }
}

unsigned long runtiming = 0;

void loop() {

  runtiming = millis();

  processdetection(activeDetect, runtiming);

  delay(10);
}

void batDetect(){
  if (!activeDetect){
    activeDetect = true;
    Serial.print(F("Detection: "));
    Serial.println(millis());
  }
}
