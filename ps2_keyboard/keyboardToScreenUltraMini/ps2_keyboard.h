//Works with Ultra Mini Multimedia Keyboard  TKB-1010 German layout after swithing back the Y and Z keys.


// Set up a new SoftwareSerial object uncomment next 5 lines.
// #include <SoftwareSerial.h>
// #define rxPin 2
// #define txPin 2 // for this keyboard application only the rx pin is used,   for arduino r4 ony pin 2 or pin 3 (which have interrupts) can be used.
// #define buffer_size 32
// SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin, buffer_size);  // softwareserial could use any pin

// if available also the hardware uart on D0 could be used uncomment next line.
HardwareSerial &mySerial = Serial1; // if available also the hardware uart on D0 could be used

const int len_arr = 50;
const int conversion_table_int[] =       {0x1c,0x32,0x21,0x23,0x24,0x2b,0x34,0x33,0x43,0x3b,0x42,0x4b,0x3a,0x31,0x44,0x4d,0x15,0x2d,0x1b,0x2c,0x3c,0x2a,0x1d,0x22,0x35,0x1a,0x16,0x1e,0x26,0x25,0x2e,0x36,0x3d,0x3e,0x46,0x45,0x4e,0x55,0x29,0x54,0x5b,0x5d,0x4c,0x41,0x49,0x4a,0x66,0x5a,0x71,0x61};
const char conversion_table_asc[] =       {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 's', ' ', ' ', '[', '+', '#', ';', ',', '.', '/','\b','\n', '.', '<'};
const char conversion_table_asc_shift[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@', ' ', '$', '%', '&', '/', '(', ')', '=', '?', ' ', ' ', '{', '*','\'', ':', ';', '>', '?','\b','\n', ':', '>'};
bool shift = false;
bool ps2_debug = true;

void setup_ps2_keyboard()  {
    // Define pin modes for TX and RX  it looks like this is not required.
    // pinMode(rxPin, INPUT);
    // pinMode(txPin, OUTPUT);
    
    // Set the baud rate for the SoftwareSerial object
    mySerial.begin(12000, SERIAL_8O1);  // depending on the keyboad the baud rate should be set to something between 10000 and 16600 bits per second.
                                        // check with the trace sketch to find the baud rate.
}


int read_keyboard(){
  if (mySerial.available() > 0) {
    int mychar = mySerial.read();
    // Serial.println(mychar, HEX);
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
        return 0;
      }
      mychar = (mychar<<8) + mychar2;
      delay(4);
      mychar = (mychar<<8) + mySerial.read();
      return mychar;
    }
    if (mychar == 0xF0){ // key relase procedure
      delay(4);
      if (mySerial.available() > 0){
        mychar = mySerial.read(); // flush release data
        if (mychar==0x12) shift=false;
        return 0; 
      }
    }
    if (mychar==0x12 ) shift = true;
    if (mychar==0x58 ) shift = !shift;
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
        Serial.println(">");
      }
  } 
  return 0;
}


// void loop() {
//   char keypress = get_keyboard_char();
//   if (keypress > 0) Serial.print(keypress);
// }