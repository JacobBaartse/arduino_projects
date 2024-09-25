
int sensorPin = A0;   // select the input pin to be measured
int sensorValue = 0;  // variable to store the value coming from the sensor

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  //Serial.println(sensorValue);
  if (sensorValue < 200){
digitalWrite(LED_BUILTIN, HIGH);
  }
  else {
digitalWrite(LED_BUILTIN, LOW);
  }
  delay(100);
}
