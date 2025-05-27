
#include <Arduino.h>


// Arduino Pin RX d0 is connected to TX of the JQ6500
// Arduino Pin TX d1 is connected to one end of a  1k resistor, 
// the other end of the 1k resistor is connected to RX of the JQ6500
// If your Arduino is 3v3 powered, you can omit the 1k series resistor
// JQ6500_Serial mp3(Serial1);  // for arduino R4 wifi Serial1 has to be used. while connected to d0, d1
const int MP3_LOOP_NONE=4;
int num_files = 35;


void sendCommand(byte command[], int length) {
  while (Serial1.available()) Serial1.read();
  Serial1.write(command, length);
}

void playTrack(byte a, byte b){
  byte command[] = {0x7E, 0x04, 0x03, a, b+1, 0xEF};
  sendCommand(command, sizeof(command));
  int count = 0;
  while (Serial1.available() == 0  && count < 10){
    count++;
    delay(100);
  }
  while (Serial1.available()) {
    count++;
    byte b = Serial1.read();
    Serial.print("0x");
    if (b < 16) Serial.print("0");
    Serial.print(b, HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void setVolume(byte a){
  byte command[] = {0x7E, 0x03, 0x06, a, 0xEF};
  sendCommand(command, sizeof(command));
}

void getStatus(){
  byte command[] = {0x7E, 0x02, 0x42, 0xEF};
  sendCommand(command, sizeof(command));
}

void reset(){
  byte command[] = {0x7E, 0x02, 0x0C, 0xEF};
  sendCommand(command, sizeof(command));
}

void setLoopMode(byte a){
  byte command[] = {0x7E, 0x03, 0x11, a, 0xEF};
  sendCommand(command, sizeof(command)); 
}

void countFiles(){
  byte command[] = {0x7E, 0x02, 0x47, 0xEF};
  sendCommand(command, sizeof(command));
}


void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);
  reset();
  setVolume(30);
  setLoopMode(MP3_LOOP_NONE);
  // pinMode(buttonPin, INPUT_PULLUP);
  // num_files = countFiles(MP3_SRC_BUILTIN);
  // Serial.println("num files");
  // Serial.print(num_files);
}
int i = 0;

void loop() {
  for (int i=0; i<num_files ; i++)
  {
      playTrack(0, i);
  }  
}
