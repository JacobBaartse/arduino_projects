
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define i2c_Address 0x3C //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3D //initialize with the I2C addr 0x3D Typically Adafruit OLED's
#define OLED_RESET -1

Adafruit_SSD1306 display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

enum DisplayState { Off, Dim, On };

uint8_t displaystatus = DisplayState::Off;
// void display_oled(bool clear, int x, int y, const char* text) {
//   if (displaystatus == DisplayState::Off) return;
//   // if (clear) display.clearDisplay();
//   display.setCursor(x, y);
//   display.print(text);
//   // display.display();
// }

DisplayState setDisplay(DisplayState statustoset) {
  static DisplayState displaystatus = DisplayState::Dim;
  switch(statustoset){
    case DisplayState::Dim:
      display.ssd1306_command(SSD1306_DISPLAYON);
      //display.dim(true); // dim display
      //displaystatus = DisplayState::Dim; // this display does not support dim
      //displaystatus = DisplayState::On;
      display.ssd1306_command(SSD1306_SETCONTRAST);
      display.ssd1306_command(1);
      displaystatus = DisplayState::Dim; // this display does not support dim ??
      break;
    case DisplayState::On:
      display.ssd1306_command(SSD1306_DISPLAYON);
      display.dim(false);
      displaystatus = DisplayState::On;
      break;
    //case DisplayState::Off:
    default:
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      displaystatus = DisplayState::Off;
  }
  return displaystatus;
}

// void clear_display(){
//   display.clearDisplay();
//   display.display();
// }

char Lines[4][101] = {
  "0", // "Welcome Leo",
  "  1", // "Demo {small disp.}", 
  "    2", // "Whats up?",
  "      3", //"Hello World"
};  
uint8_t LinesYPos[4] = { 16, 32, 48, 64 };
uint8_t upddisplay = 200; // loop amount for text to be visible, TBD

void updateDisplay() {
  //bool fresh = true;
  //Serial.println("updateDisplay 1 ");
  //delay(1000);
  display.clearDisplay();
  for(int lin=0; lin < 4 ; lin++){
    Serial.println(Lines[lin]);
  }
  //Serial.println("updateDisplay 2 ");
  //delay(5000);
  for(int lin=0; lin < 4 ; lin++){
    display.setCursor(0, LinesYPos[lin]);
    display.print(Lines[lin]);
  }
  display.display();
  //Serial.println("updateDisplay 3 ");
  upddisplay = 0;
  // display_oled(false, 0, 32, Lines[1]); 
  // display_oled(false, 0, 48, Lines[2]);  
  // display_oled(false, 0, 64, Lines[3]); 
}

void generateQRCode(const char* text) {
  // Create a QR code object
  QRCode qrcode;
  
  // Define the size of the QR code (1-40, higher means bigger size), this value is the QR code version
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, 0, text);

  // Clear the display
  display.clearDisplay();

  Serial.print(F("QR info: "));
  Serial.println(text);

  // Calculate the scale factor
  int scale = min(SCREEN_WIDTH / qrcode.size, SCREEN_HEIGHT / qrcode.size);
  
  // Calculate horizontal shift
  int shiftX = (SCREEN_WIDTH - qrcode.size*scale)/2;
  
  // Calculate vertical shift
  int shiftY = (SCREEN_HEIGHT - qrcode.size*scale)/2;

  // Draw the QR code on the display
  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        display.fillRect(shiftX + x*scale, shiftY + y*scale, scale, scale, WHITE);
      }
    }
  }

  // Update the display
  display.display();
}

void textdisplay(const char* text) {
  // Clear the display
  display.clearDisplay();

  Serial.print(F("textdisplay: "));
  Serial.println(text);

  //display.setCursor(0, LinesYPos[0]);
  display.setCursor(0, 16);
  display.print(text);

  // Update the display
  display.display();
}

void DisplayProgress(const char* ptext) {
  static bool toggle = false;
  static bool toggle2 = false;
  static bool toggle3 = false;
  if (toggle) {
    if (toggle2) {
      generateQRCode(ptext);
    }
    else {
      generateQRCode("Other text that can be reasonable long (enough) QR");
    }
    toggle2 = !toggle2;
  }
  else{
    if (toggle3) {
      textdisplay(ptext);
    }
    else {
      textdisplay("Very long text that is readable but not very nice on the display to read");
    }
    toggle3 = !toggle3;
  }
  toggle = !toggle; // toggle
}
