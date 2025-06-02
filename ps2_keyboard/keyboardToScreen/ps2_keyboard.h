// #include <SoftwareSerial.h>

#define rxPin 2
#define txPin 2 // for this keyboard application only the rx pin is used, 

// Set up a new SoftwareSerial object
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

const int len_arr = 49;
const int conversion_table_int[] =       {0x1c,0x32,0x21,0x23,0x24,0x2b,0x34,0x33,0x43,0x3b,0x42,0x4b,0x3a,0x31,0x44,0x4d,0x15,0x2d,0x1b,0x2c,0x3c,0x2a,0x1d,0x22,0x35,0x1a,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46,0x45,0x4e,0x55,0x29,0x54,0x5b,0x5d,0x4c,0x52,0x41,0x49,0x4a,0x66,0x5a};
const char conversion_table_asc[] =       {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', ' ', '[', ']','\\', ';','\'', ',', '.', '/','\b','\n'};
const char conversion_table_asc_shift[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', ' ', '{', '}', '|', ':', '"', '<', '>', '?','\b','\n'};
bool shift = false;
bool ps2_debug = false;

void setup_ps2_keyboard()  {
    // Define pin modes for TX and RX
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);
    
    // Set the baud rate for the SoftwareSerial object
    mySerial.begin(12500, SERIAL_8O1);  // depending on the keyboad the boudrate should be set to somting between 10000 and 17000 bits per second.
}


int read_keyboard(){
  if (mySerial.available() > 0) {
    int mychar = mySerial.read();
    // return mychar1;
    if (mychar == 0xe0){
      delay(4);
      int mychar2 = mySerial.read();
      if (mychar2 == 0xF0){ // key relase procedure
        delay(4);
        mychar2 = mySerial.read(); // flush release data
        return 0; 
      }
      mychar = (mychar<<8) + mychar2;
      return mychar;
    }
    if (mychar == 0xe1){
      delay(4);
      int mychar2 = mySerial.read();
      if (mychar2 == 0xF0){ // key relase procedure
        delay(4);
        mychar2 = mySerial.read(); // flush release data
        delay(4);
        mychar2 = mySerial.read(); // flush release data
        return 0; 
      }
      mychar = (mychar<<8) + mychar2;
      delay(4);
      mychar = (mychar<<8) + mySerial.read();
      return mychar;
    }
    if (mychar == 0xF0){ // key relase procedure
      delay(4);
      mychar = mySerial.read(); // flush release data
      if (mychar==0x12 || mychar==0x59) shift=false;
      return 0; 
    }
    if (mychar==0x12 || mychar==0x59) shift=true;
    if (mychar==0x58) shift = !shift;
    return mychar;
  } 
  return 0;
}

char get_keyboard_char(){
    int key = read_keyboard();
  if (key!=0){
    bool char_found=false;
    for (int j=0; j<len_arr; j++){
      if (conversion_table_int[j] == key){
        if (shift) return conversion_table_asc_shift[j]; //Serial.print(conversion_table_asc_shift[j]);
        else return conversion_table_asc[j]; // Serial.print(conversion_table_asc[j]);
        char_found=true;
      }
    }
    if (ps2_debug)
      if (char_found == false){
        Serial.print("<");
        Serial.print(key, HEX);
        Serial.print(">");
      }
  } 
  return 0;
}


// void loop() {
//   char keypress = get_keyboard_char();
//   if (keypress > 0) Serial.print(keypress);
// }