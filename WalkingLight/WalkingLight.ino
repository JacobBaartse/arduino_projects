// variables for blink led:
// constants won't change. Used here to set a pin number:
const int ledPins [] = {13, 10, 11, 12, 5, 6, 7};
const int nr_leds = (sizeof(ledPins)/sizeof(int));
int ledIndex = 0;


// Generally, you should use "unsigned long" for variables that hold time for 272 years
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated
unsigned long currentMillis = 0;

// constants won't change:
const unsigned long interval = 500;  // interval at which to blink (milliseconds)

void setup_blink(){
  // set the digital pin as output:
  for (int i=0; i<nr_leds; i+=1){
      pinMode(ledPins[i], OUTPUT);
  }
}

void blink(){
  // from example code blink without delay
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {

    previousMillis = currentMillis;

    digitalWrite(ledPins[ledIndex], LOW);
    ledIndex += 1;
    ledIndex =ledIndex % nr_leds;
    digitalWrite(ledPins[ledIndex], HIGH);

  }
}


void setup() {
  setup_blink();
}




void loop() {
  // here is where you'd put code that needs to be running all the time.
  blink();

}
