
//#define BUILD_IN_LED 13
int pin_7 = 17; // pir connected

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(pin_7, INPUT);
  //pinMode(BUILD_IN_LED, OUTPUT);
  Serial.println(F("Starting detections"));
}

unsigned long val = 0;

void loop() {
  Serial.print(++val);

  // put your main code here, to run repeatedly:
  if (digitalRead(pin_7)==LOW){
    //digitalWrite(BUILD_IN_LED, LOW);
    Serial.println(" L");
  }
  else{
    //digitalWrite(BUILD_IN_LED, HIGH);
    Serial.println(" H");
  }
  delay(1000);
}
