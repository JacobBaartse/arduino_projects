#define GREEN_LED_PIN 6
#define RED_LED_PIN 4
#define RED_BUTTON_PIN 5
#define GREEN_BUTTON_PIN 7
#define PUSHED LOW

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(GREEN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(GREEN_BUTTON_PIN)==PUSHED) digitalWrite(GREEN_LED_PIN, HIGH);  
  else digitalWrite(GREEN_LED_PIN, LOW);  
  if (digitalRead(RED_BUTTON_PIN)==PUSHED) digitalWrite(RED_LED_PIN, HIGH);  
  else digitalWrite(RED_LED_PIN, LOW);  
  delay(100);                      // wait for some time
}
