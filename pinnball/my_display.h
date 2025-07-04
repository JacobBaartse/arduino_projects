/*
  Hello World!
  Displays the milliseconds passed since power on.
  Using an HT1621 based 7-segment LCD with 6 digits
  The circuit:
  cs to pin 13
  wr to pin 12
  Data to pin 8
  backlight to pin 10
  Created 9 dec 2018
  By valerio\new (5N44P)

  https://github.com/5N44P/ht1621-7-seg
*/

HT1621 lcd; // create an "lcd" object

void display_setup(){

  // lcd.begin(13, 12, 8, 10); // (cs, wr, Data, backlight)
  // lcd.backlight(); // turn on the backlight led

  // if no backlight control is given, you can also use:
  lcd.begin(5, 12, 4); // (cs, wr, Data)
  lcd.clear(); // clear the screen
}

