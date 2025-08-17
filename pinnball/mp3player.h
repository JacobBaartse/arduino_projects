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

#define DEBUG 0

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif


JQ6500_Serial mp3(Serial1);  // for arduino R4 , R4 wifi Serial1 has to be used. while connected to d0, d1

// int num_files = 0;
// int i=0;

#define  CANNON_SHOT 0
#define  DO_RE_MI 1
#define  FIEEEW 2
#define  GUN_SHOT 3
#define  HARP_TRIANGEL 4
#define  HART_SLAG 5
#define  HIGH_ANOTHERONE 6
#define  HIGH_PING 7
#define  INTRO_MELODY 8
#define  JAMMER 9
#define  JUICHEN 10
#define  KIP 11
#define  KLIK_KLAK 12
#define  KLOK_TIKKEN 13
#define  KLONGNGNG 14
#define  KOEKOEK_KLOK 15
#define  KREKEL 16
#define  LOW_ANOTHERONE 17
#define  OLD_TELEPHONE_RING 18
#define  P_OEIJJ 19
#define  PAARD 20
#define  POING_5X 21
#define  POINOININ 22
#define  PRRRR 23
#define  SUPER_GOOD 24
#define  SUPER_SCORE 25
#define  TADI_TADIADI 26
#define  TATA_TSJING 27
#define  TIE_WIEWIE 28
#define  TING_TING_TING 29
#define  TOE_OE_3X 30
#define  TOE_TOKKK 31
#define  TONG_TING_HIGH 32
#define  TRRRRR 33
#define  WOOOWOWOW 34
#define  XILOFOON_UP 35
#define  Y1_KORT_PR 36


void setup_mp3_player() {
  Serial1.begin(9600);
  mp3.reset();
  mp3.setVolume(15);
  mp3.setLoopMode(MP3_LOOP_NONE);
  // num_files = mp3.countFiles(MP3_SRC_BUILTIN);
}

void Play_mp3_file(byte songNumber) {
  debug("play mp3 file");
  debugln(songNumber);
  byte command[] = {0x7E, 0x04, 0x03, 0x00, songNumber+1, 0xEF};
  Serial1.write(command, sizeof(command));
  // mp3.playFileByIndexNumber(songNumber+1);  
  delay(50); // do some delay before sendig next mp3 command
}

void sendCommand(byte command[], int length) {
  debug("mp3 send command");
  while (Serial1.available()) Serial1.read();
  Serial1.write(command, length);
  delay(50); // do some delay before sendig next mp3 command
}

void reset_mp3(){
  debug("mp3 reset");
  byte command[] = {0x7E, 0x02, 0x0C, 0xEF};
  sendCommand(command, sizeof(command));
}

void playTrack( byte b){
  debug("mp3 play track");
  byte command[] = {0x7E, 0x04, 0x03, 0x00, b+1, 0xEF};
  Serial1.write(command, sizeof(command));
  delay(50); // do some delay before sendig next mp3 command
  // int count = 0;
  // while (Serial1.available() == 0  && count < 2){
  //   count++;
  //   delay(100);
  // }
  // while (Serial1.available()) {
  //   byte b = Serial1.read();
  // }
}

#undef debug
#undef debugln