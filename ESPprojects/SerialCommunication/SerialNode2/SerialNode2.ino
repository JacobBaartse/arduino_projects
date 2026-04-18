#include <SoftwareSerial.h>

// Use D5 (RX) and D6 (TX)
SoftwareSerial mySerial(D5, D6); 

unsigned long runtiming = 0;
bool action = false;
unsigned long serialdata = 0;
char cpm_array[101];
size_t bytesread = 0;

void setup() {
  Serial.begin(115200);   // USB Serial
  mySerial.begin(9600);   // Software Serial to other D1 Mini

  Serial.println(F(" "));
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 
  Serial.println("Serial communication initialized");
  Serial.flush(); 

}

// function to indicate the passing of certain duration
bool timepassing(unsigned long curtime, unsigned long duration){
  static unsigned long rtime = 0;
  if(rtime + duration > curtime) return false;
  rtime = millis(); // get fresh time to base the new interval on
  return true;
}

unsigned long getvalfromstring(String serialdata){
  unsigned long serialval = 0;
  // char valholder[9];
  // serialdata.toCharArray(valholder, 9);
  // serialval = strtol(valholder, NULL, 16);
  return serialval;
}


void loop() {

  runtiming = millis();

  action = timepassing(runtiming, 14000);
  if (action){
    mySerial.print("t");
    mySerial.println("Hay from Board 2");
    mySerial.print("x");
    mySerial.println(0xbeefdead, HEX);
    mySerial.print("x");
    mySerial.println(runtiming, HEX);
    //slen = mySerial.write(pmessage);
    Serial.print("Sent message at: ");
    Serial.println(runtiming);
  }

  if (mySerial.available()) {
    memset(cpm_array, 0, 101);
    bytesread = mySerial.readBytesUntil('\n', cpm_array, 101);
    Serial.print("Data length: ");
    Serial.println(bytesread);    
    Serial.print("Data: ");
    Serial.println(cpm_array);

    // mySerial.readBytesUntil
    // String data = mySerial.readStringUntil('\n');
    // Serial.println("Received: " + data);
    // serialdata = getvalfromstring(data);
    // Serial.print("Value: ");
    // Serial.println(serialdata);
  }
}
