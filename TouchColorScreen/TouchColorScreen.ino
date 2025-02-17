#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <TouchScreen.h>
#include <VirtualWire.h>

// TFT screen setup
#define TFT_CS   10
#define TFT_RST  9
#define TFT_DC   8
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Touchscreen setup
#define YP A2  // Must be an analog pin, use "An" notation
#define XM A3  // Must be an analog pin, use "An" notation
#define YM 8   // Can be a digital pin
#define XP 9   // Can be a digital pin

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Calibration values (example, may need to adjust)
#define TS_MINX 150
#define TS_MAXX 920
#define TS_MINY 120
#define TS_MAXY 940

void setup() {
  tft.initR(INITR_BLACKTAB); // Initialize TFT screen
  tft.setRotation(0);
  tft.fillScreen(ST7735_BLACK);
  
  // RF module setup
  vw_set_tx_pin(2); // Pin 2 for transmitter
  vw_setup(2000); // 2000 baud rate
  
  // Display welcome message
  tft.setCursor(0, 0);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.println("Touch the screen...");
}

void loop() {
  TSPoint p = ts.getPoint();
  
  // Check if screen is touched
  if (p.z > ts.pressureThreshhold) {
    // Map the touch coordinates
    int x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    int y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    
    // Display touch coordinates
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 0);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.print("X: "); tft.println(x);
    tft.print("Y: "); tft.println(y);
    
    // Send touch coordinates via RF
    char message[20];
    sprintf(message, "X:%d Y:%d", x, y);
    vw_send((uint8_t*)message, strlen(message));
    vw_wait_tx();
  }
  
  delay(100); 

}