/** Demonstrate how to play a file by it's (FAT table) index number.
 *
 * @author James Sleeman,  http://sparks.gogo.co.nz/
 * @license MIT License
 * @file
 */
 
#include <Arduino.h>
#include <JQ6500_Serial.h>
// https://github.com/sleemanj/JQ6500_Serial  download zip and install library

// Create the mp3 module object. In this example we're using the Hardware Serial port, so:
// Arduino Pin RX d0 is connected to TX of the JQ6500
// Arduino Pin TX d1 is connected to one end of a  1k resistor, 
// the other end of the 1k resistor is connected to RX of the JQ6500
// If your Arduino is 3v3 powered, you can omit the 1k series resistor
JQ6500_Serial mp3(Serial1);  // for arduino R4 wifi Serial1 has to be used. while connected to d0, d1
const int buttonPin=2;
int num_files = 0;

void setup() {
  Serial1.begin(9600);
  mp3.reset();
  mp3.setVolume(30);
  mp3.setLoopMode(MP3_LOOP_NONE);
  pinMode(buttonPin, INPUT_PULLUP);
  num_files = mp3.countFiles(MP3_SRC_BUILTIN);
}
int i = 0;
void loop() {

  if (digitalRead(buttonPin) == LOW)
  {
    // mp3.next();
    // mp3.setLoopMode(MP3_LOOP_ONE_STOP)
    // mp3.
    mp3.playFileByIndexNumber((i % num_files)+1);  
    i++;
  }
}
