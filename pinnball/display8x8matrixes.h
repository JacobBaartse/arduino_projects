#include "simple_matrix.h"

/*
Initialize the library. The 4 means that the CS pin is connected to pin D4.
You can change this to whatever you want, just make sure to connect the CS
pin to the pin number.

The disp means that any future function calls to the library uses "disp" as the
library's object name. For example, the library has a function called
"setIntensity", you need to write "disp.setIntensity" to call that function.
*/
simpleMatrix disp_8x8_matrix(A1, 24);  // cs pin, num modules

/*
Store the text to be displayed in memory
The PROGMEM means that the text will be stored in the Arduino's
flash memory instead of RAM. This means that you can a lot more stuff (~27000 characters available on the Uno)
*/
const char tilt_text[] PROGMEM = "    TILT    TILTTILT    TILT";
//Defines "textDelay" as 25ms (Delay between when the display scroll by 1 pixel to the left).
#define textDelay 25

//This code will run only once when the Arduino is turned on.
void setup_8x8matrix(){
  //Starts the library
  disp_8x8_matrix.begin();
  /*
   * If you notice that the display is upside-down per display, uncomment the following line
   */
  //disp.invertIndividualDisplays();
  
  //Set the LED's intensity. This value can be anywhere between 0 and 15.
  disp_8x8_matrix.setIntensity(0x01);
}

//After void setup(), this code will run and loop forever.
// void loop(){
//   //Scroll the text "text". The "textDelay" determines how long each
//   //    frame is in mS.
//   // disp.scrollTextPROGMEMRightToLeft(text, textDelay);
//   disp.print(text);

//   //After scolling, delay by 1 second.
//   delay(1000);
// }
