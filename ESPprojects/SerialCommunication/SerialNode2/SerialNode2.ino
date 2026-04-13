#include <SoftwareSerial.h>

// Use D5 (RX) and D6 (TX)
SoftwareSerial mySerial(D5, D6); 

void setup() {
  Serial.begin(115200);   // USB Serial
  mySerial.begin(9600);   // Software Serial to other D1 Mini
  Serial.println("Receiver Initialized");
}

// function to indicate the passing of certain duration
bool timepassing(unsigned long curtime, unsigned long duration){
  static unsigned long rtime = 0;
  if(rtime + duration > curtime) return false;
  rtime = millis(); // get fresh time to base the new interval on
  return true;
}

unsigned long runtiming = 0;
bool action = false;

void loop() {

  runtiming = millis();

  action = timepassing(runtiming, 14000);
  if (action){
    mySerial.println("Hello from Board 2");
    mySerial.println(0xbeefdead, HEX);
    mySerial.println(runtiming, HEX);
    //slen = mySerial.write(pmessage);
    Serial.print("Sent message at: ");
    Serial.println(runtiming);
  }

  if (mySerial.available()) {
    String data = mySerial.readStringUntil('\n');
    Serial.println("Received: " + data);
  }
}
