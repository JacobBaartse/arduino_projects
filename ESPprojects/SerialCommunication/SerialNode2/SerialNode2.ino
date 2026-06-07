#include <SoftwareSerial.h>

// Use D5 (RX) and D6 (TX)
SoftwareSerial mySerial(D5, D6); 

unsigned long runtiming = 0;
unsigned long sequencenumber = 0;
unsigned long serialdata = 0;
char cpm_array[101];
size_t bytesread = 0;
bool resetclear = false;

uint32_t storeData[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
uint32_t checkData[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// function to indicate the passing of certain duration
bool timepassing(unsigned long curtime, unsigned long duration){
  static unsigned long rtime = 0;
  if(rtime + duration > curtime) return false;
  rtime = millis(); // get fresh time to base the new interval on
  return true;
}

void sendserialtext(const char *text){
  mySerial.print("t");
  mySerial.println(text);
}

void sendserialhex(uint32_t val, int8_t idx){
  mySerial.print("x");
  mySerial.print(idx, HEX);
  mySerial.println(val, HEX);
}

uint32_t converttoval(char *bdata, uint8_t start, uint8_t end){
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

bool readserialdata(){
  bool datafound = false;

  if (mySerial.available()) {
    memset(cpm_array, 0, 101);
    bytesread = mySerial.readBytesUntil('\n', cpm_array, 101);
    char mtype = cpm_array[0]; // first char is the message type
    if (bytesread > 2){
      datafound = true;
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
        uint32_t answerindex = converttoval(cpm_array, 1, 2); // second char is the index
        Serial.print(", idx: ");
        Serial.print(answerindex, DEC);
        uint32_t answer = converttoval(cpm_array, 2, bytesread - 1);
        Serial.print(", HEX: 0x");
        Serial.println(answer, HEX);

        if (answerindex == 15){ // check other board is restarted
          resetclear = answer == 0xffffffff;
        }
        storeData[answerindex] = answer;
      }
      // Serial.print(" data: ");
      // Serial.println(cpm_array + 1);
    } 
  }

  // String data = mySerial.readStringUntil('\n');
  // Serial.println("Received: " + data);
  // serialdata = getvalfromstring(data);
  // Serial.print("Value: ");
  // Serial.println(serialdata);

  return datafound;
}

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

  sendserialhex(0xffffffff, 15); // signal a reset/startup condition
}

// // function to indicate the passing of certain duration
// bool timepassing(unsigned long curtime, unsigned long duration){
//   static unsigned long rtime = 0;
//   if(rtime + duration > curtime) return false;
//   rtime = millis(); // get fresh time to base the new interval on
//   return true;
// }

// unsigned long getvalfromstring(String serialdata){
//   unsigned long serialval = 0;
//   // char valholder[9];
//   // serialdata.toCharArray(valholder, 9);
//   // serialval = strtol(valholder, NULL, 16);
//   return serialval;
// }

bool newdata = false;
bool action = false;

void loop() {

  runtiming = millis();

  action = timepassing(runtiming, 14000);
  if (action){
    // mySerial.print("t");
    // mySerial.println("Hay from Board 2");
    sendserialtext("Hay from Board 2");

    // mySerial.print("x");
    // mySerial.println(0xbeefdead, HEX);
    // mySerial.print("x");
    // mySerial.println(runtiming, HEX);
    // //slen = mySerial.write(pmessage);

    sendserialhex(0xbeefdead, 13);
    sendserialhex(runtiming, 15);
    sendserialhex(++sequencenumber, 14);

    Serial.print("Sent message at: ");
    Serial.println(runtiming);
  }

  newdata = readserialdata();
  if (newdata){
    if (resetclear){ // other board is restarted
      Serial.println(F("reset detected other node"));

      for (uint8_t ai=0;ai<16;ai++){
        Serial.print("Stored at index: ");
        Serial.print(ai);
        Serial.print(", 0x");
        Serial.println(storeData[ai], HEX);
        storeData[ai] = 0;
      }
      //ESP.restart(); // do not restart this board, endless loop will happen for serial1 and serial2

      resetclear = false;
    }
    else {

    }
    newdata = false;
  }

}
