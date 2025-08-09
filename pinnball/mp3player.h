/** Demonstrate how to play a file by it's (FAT table) index number.
 *


 * @author James Sleeman,  http://sparks.gogo.co.nz/
 * @license MIT License
 * @file
 */
 
// #include <Arduino.h>
// #include <JQ6500_Serial.h>
// https://github.com/sleemanj/JQ6500_Serial  download zip and install library

// Create the mp3 module object. In this example we're using the Hardware Serial port, so:
// Arduino Pin RX d0 is connected to TX of the JQ6500
// Arduino Pin TX d1 is connected to one end of a  1k resistor, 
// the other end of the 1k resistor is connected to RX of the JQ6500
// If your Arduino is 3v3 powered, you can omit the 1k series resistor
JQ6500_Serial mp3(Serial1);  // for arduino R4 , R4 wifi Serial1 has to be used. while connected to d0, d1

// int num_files = 0;
// int i=0;

const int CANNON_SHOT = 0;
const int DO_RE_MI = 1;
const int FIEEEW = 2;
const int GUN_SHOT = 3;
const int HARP_TRIANGEL = 4;
const int HART_SLAG = 5;
const int HIGH_ANOTHERONE = 6;
const int HIGH_PING = 7;
const int INTRO_MELODY = 8;
const int JAMMER = 9;
const int JUICHEN = 10;
const int KIP = 11;
const int KLIK_KLAK = 12;
const int KLOK_TIKKEN = 13;
const int KLONGNGNG = 14;
const int KOEKOEK_KLOK = 15;
const int KREKEL = 16;
const int LOW_ANOTHERONE = 17;
const int OLD_TELEPHONE_RING = 18;
const int P_OEIJJ = 19;
const int PAARD = 20;
const int POING_5X = 21;
const int POINOININ = 22;
const int PRRRR = 23;
const int SUPER_GOOD = 24;
const int SUPER_SCORE = 25;
const int TADI_TADIADI = 26;
const int TATA_TSJING = 27;
const int TIE_WIEWIE = 28;
const int TING_TING_TING = 29;
const int TOE_OE_3X = 30;
const int TOE_TOKKK = 31;
const int TONG_TING_HIGH = 32;
const int TRRRRR = 33;
const int WOOOWOWOW = 34;
const int XILOFOON_UP = 35;
const int Y1_KORT_PR = 36;


void setup_mp3_player() {
  Serial1.begin(9600);
  mp3.reset();
  mp3.setVolume(30);
  mp3.setLoopMode(MP3_LOOP_NONE);
  // num_files = mp3.countFiles(MP3_SRC_BUILTIN);
}

void Play_mp3_file(byte songNumber) {
  byte command[] = {0x7E, 0x04, 0x03, 0x00, songNumber+1, 0xEF};
  Serial1.write(command, sizeof(command));
  // mp3.playFileByIndexNumber(songNumber+1);  
}

void sendCommand(byte command[], int length) {
  while (Serial1.available()) Serial1.read();
  Serial1.write(command, length);
  delay(4); // do some delay before sendig next mp3 command
}

void reset_mp3(){
  byte command[] = {0x7E, 0x02, 0x0C, 0xEF};
  sendCommand(command, sizeof(command));
}

void playTrack( byte b){
  byte command[] = {0x7E, 0x04, 0x03, 0x00, b+1, 0xEF};
  Serial1.write(command, sizeof(command));
  // int count = 0;
  // while (Serial1.available() == 0  && count < 2){
  //   count++;
  //   delay(100);
  // }
  // while (Serial1.available()) {
  //   byte b = Serial1.read();
  // }
}

