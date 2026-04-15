#include <SoftwareSerial.h>

// Use D6 (RX) and D5 (TX)
SoftwareSerial mySerial(D6, D5); 

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

uint32_t converttoint(char *bdata, uint8_t start, uint8_t end){
  uint32_t serialval = 0;
  uint8_t nibbleval = 0;
  char nibblechar;

  for (int i=start; i<end; i++){
    nibblechar = bdata[i];
    switch(nibblechar){
      case 'a':
      case 'A': 
        nibbleval = 10;
        break;
      case 'b':
      case 'B':
        nibbleval = 11;
        break;
      case 'c':
      case 'C':
        nibbleval = 12;
        break;
      case 'd':
      case 'D':
        nibbleval = 13;
        break;
      case 'e':
      case 'E':
        nibbleval = 14;
        break;        
      case 'f':
      case 'F':
        nibbleval = 15;
        break;
      default:
        nibbleval = uint8_t(nibblechar - '0');
    }
    // Serial.print("char: ");
    // Serial.print(nibblechar);
    // Serial.print(", val: ");
    // Serial.println(nibbleval);    
    serialval = serialval * 16 + nibbleval;
  }
  return serialval;
}

// unsigned long getvalfromstring(String serialdata){
//   unsigned long serialval = 0;
//   // char valholder[9];
//   // serialdata.toCharArray(valholder, 9);
//   // serialval = strtol(valholder, NULL, 16);
//   return serialval;
// }

void loop() {

  //pmessage.keyword = ;
  runtiming = millis();

  action = timepassing(runtiming, 9000);
  if (action){
    mySerial.print("t");
    mySerial.println("Hello from Board 1");
    mySerial.print("x");
    mySerial.println(0xdeadbeef, HEX);
    mySerial.print("x");
    mySerial.println(runtiming, HEX);
    //slen = mySerial.write(pmessage);
    Serial.print("Sent message at: ");
    Serial.println(runtiming);
  }

  if (mySerial.available()) {
    memset(cpm_array, 0, 101);
    bytesread = mySerial.readBytesUntil('\n', cpm_array, 100);
    char mtype = cpm_array[0];
    if (bytesread > 2){
      Serial.print("mtype: ");
      Serial.print(mtype); 
      // Serial.print("Data length: ");
      // Serial.println(bytesread - 2);      
      switch(mtype){
      case 't':
        Serial.print(", TXT: ");
        Serial.println(cpm_array + 1);
        break;
      case 'x':
      default:
        Serial.print(", HEX: 0x");
        uint32_t answer = converttoint(cpm_array, 1, bytesread - 1);
        Serial.println(answer, HEX);
      }
      // Serial.print(" data: ");
      // Serial.println(cpm_array + 1);
    } 

    // String data = mySerial.readStringUntil('\n');
    // Serial.println("Received: " + data);
    // serialdata = getvalfromstring(data);
    // Serial.print("Value: ");
    // Serial.println(serialdata);
  }

}
