int pin_7 = 11; // pir connected
#define BUILD_IN_LED 13

void setup() {
  // put your setup code here, to run once:
    pinMode(pin_7, INPUT);
    pinMode(BUILD_IN_LED, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
    if (digitalRead(pin_7)==LOW){
      digitalWrite(BUILD_IN_LED, LOW);
    }
    else{
      digitalWrite(BUILD_IN_LED, HIGH);
    }
      delay(100);
}
