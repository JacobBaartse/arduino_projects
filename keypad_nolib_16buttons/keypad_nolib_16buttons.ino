/*
 * Keypad example did not work
 *
 * use scanning algorithm from:
 * https://www.makerguides.com/how-to-use-a-keypad-with-arduino-for-password-security/
 *
 * Idea: - connect to RF-NANO, battery powered and send button data over RF to UNO controller
 *       - feedback LED that the information is received (ACK)
 */


const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3}; 
byte colPins[COLS] = {A4, A5, A6, A7}; 

// byte rowPins[ROWS] = {A4, A5, A6, A7}; 
// byte colPins[COLS] = {A0, A1, A2, A3}; 

//Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 


const uint8_t maxkeys = 10;
char keytracking[11]; // 10 characters + room for the null terminator
const uint16_t maxtime = 4000; // maximum time to type a multidigit number
unsigned long keyingtime = 0;
unsigned long runningtime = 0;
uint8_t keyindex = 0;
bool transmit = false;

char keypadentry(){
  char keyinput;

/* Routine to scan the key pressed */
unsigned char key_scan()
{
       unsigned char i, j, temp1, temp2;

        while( 1 ) /* keep waiting for a key to be pressed */

                for(i=0; i<4; i++) {

                        /* Set each row to 0 */
                        P1 = 0xff & ~(1<<i); 

                        /* Scan each column to see which key was pressed */
                        for (j=4; j<8; j++) {

                              /* Code to determine the position of the
                                 key which was pressed */
                              /* return(position) */
                        }
                }
}


  // for (int i=0;i<=maxkeys;i++){
  //   keytracking[i] = 0; // place null character
  // }
  // keyindex = 0;
  return keyinput;
}

void clearkeypadcache(){
  for (int i=0;i<=maxkeys;i++){
    keytracking[i] = 0; // place null character
  }
  keyindex = 0;
}

void setup(){
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  delay(1000);
  Serial.println(F("Starting 16 button keypad without library"));
  Serial.println();
  Serial.print(__FILE__);
  Serial.print(F("\n, creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  clearkeypadcache();
}

void loop(){

  runningtime = millis();

  char customKey = keypadentry();
  
  //if (customKey){
  if (false){
    if (keyindex == 0){ // start timing
      keyingtime = runningtime + maxtime;
    }
    if (keyindex < maxkeys){
      keytracking[keyindex++] = customKey;
    }
    Serial.print(customKey);
    Serial.print(F(", time: "));
    Serial.print(keyingtime);
    Serial.print(F(", index: "));
    Serial.println(keyindex);
  }

  if (keyindex > 0){
    transmit = runningtime > keyingtime;
  }

  if (transmit){
    if (keyindex > 0){
      Serial.print(F("Data collected: '"));
      Serial.print(keytracking);
      Serial.println(F("'"));
      clearkeypadcache();
    }
    else{
      Serial.print(runningtime);
      Serial.println(F(" No button data collected"));
    }
    transmit = false;
  }
}


// /*
//  * Created by ArduinoGetStarted.com
//  *
//  * This example code is in the public domain
//  *
//  * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-keypad
//  */

// #include <Keypad.h>

// const int ROW_NUM = 4; //four rows
// const int COLUMN_NUM = 3; //three columns

// char keys[ROW_NUM][COLUMN_NUM] = {
//   {'1','2','3'},
//   {'4','5','6'},
//   {'7','8','9'},
//   {'*','0','#'}
// };

// byte pin_rows[ROW_NUM] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
// byte pin_column[COLUMN_NUM] = {5, 4, 3}; //connect to the column pinouts of the keypad

// Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

// const String password = "1234"; // change your password here
// String input_password;

// void setup(){
//   Serial.begin(9600);
//   input_password.reserve(32); // maximum input characters is 33, change if needed
// }

// void loop(){
//   char key = keypad.getKey();

//   if (key){
//     Serial.println(key);

//     if(key == '*') {
//       input_password = ""; // clear input password
//     } else if(key == '#') {
//       if(password == input_password) {
//         Serial.println("password is correct");
//         // DO YOUR WORK HERE
        
//       } else {
//         Serial.println("password is incorrect, try again");
//       }

//       input_password = ""; // clear input password
//     } else {
//       input_password += key; // append new character to input password string
//     }
//   }
// }
