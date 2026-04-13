#include <SoftwareSerial.h>

// Use D6 (RX) and D5 (TX)
SoftwareSerial mySerial(D6, D5); 

void setup() {
  Serial.begin(115200);   // USB Serial
  mySerial.begin(9600);   // Software Serial to other D1 Mini
  Serial.println("Sender Initialized");
}


// struct serial_payload{
//   uint32_t keyword;
//   uint32_t timing;
// };
// serial_payload pmessage = { 0xdeadbeef, 0} ;
// uint8_t plen = 8;
// uint8_t slen = 8;

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

  //pmessage.keyword = ;
  runtiming = millis();

  action = timepassing(runtiming, 9000);
  if (action){
    //mySerial.println("Hello from Board 1");
    mySerial.println(0xdeadbeef, HEX);
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
