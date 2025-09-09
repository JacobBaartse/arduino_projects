/*
 * part of the tool/demo to measure the RF distance with the standardized RF24 hardware, build into the RF-NANO
 */

#define DETECT_PIN 2
#define LED_PIN 6

bool activeDetect = false;
bool activeDetection = false;

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
  attachInterrupt(digitalPinToInterrupt(DETECT_PIN), batDetect, FALLING); // trigger when button is pressed
  
  // pinMode(LED_PIN, OUTPUT);
  // digitalWrite(LED_PIN, LOW);

  Serial.println();  
  Serial.println(F(" ***************"));  
  Serial.println(); 
  Serial.flush(); 
}
 
unsigned long runtiming = 0;

void loop() {

  runtiming = millis();

  if (activeDetection){
    activeDetect = true;
    activeDetection = false;
  }

  delay(100);
}

void batDetect(){
  if (!activeDetect){
    activeDetection = true;
    Serial.print(F("Detection: "));
    Serial.println(millis());
  }
}
