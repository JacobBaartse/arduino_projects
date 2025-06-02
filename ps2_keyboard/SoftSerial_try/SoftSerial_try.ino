#include <SoftwareSerial.h>

#define rxPin 2
#define txPin 2 // for this keyboard application only the rx pin is used, 

// Set up a new SoftwareSerial object
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

void setup()  {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
    // Define pin modes for TX and RX
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);
    
    // Set the baud rate for the SoftwareSerial object
    mySerial.begin(12600, SERIAL_8O1);  // depending on the keyboad the boudrate should be set to somting between 10000 and 17000 bits per second.
}

void loop() {
    if (mySerial.available() > 0) {
        int mychar = mySerial.read();
        // mychar &= 0xff;
        if (mychar != 0xf0) 
          if 
            (mychar == 0xe0) Serial.print(mychar, HEX);
          else  
            Serial.println(mychar, HEX);
    }
}