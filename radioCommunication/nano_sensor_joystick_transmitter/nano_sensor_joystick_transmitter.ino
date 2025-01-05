/*
 * RF-Nano, no headers, USB-C with joystick connected
 */

#include "RF24.h"
#include <SPI.h>

#define radioChannel 106

#define VRX_PIN  A1 // Arduino pin connected to VRX pin
#define VRY_PIN  A0 // Arduino pin connected to VRY pin
#define SW_PIN   2  // Arduino pin connected to SW  pin, supporting interrupts

int xValue = 0; // To store value of the X axis
int yValue = 0; // To store value of the Y axis

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(7, 8); // nRF24L01 (CE, CSN)

const byte addresses [][6] = {"00011", "00012"};  //Setting the two addresses. One for transmitting and one for receiving

unsigned long const keywordvalM = 0xfeebbeef; 
unsigned long const keywordvalS = 0xbeeffeeb; 

char dataToSend[10] = "Sender1 0";
char rxNum = '0';

void setup() {
  Serial.begin(115200);
  Serial.println(F(" *****<>*****"));  

  SPI.begin();
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (1) {
      // hold in an infinite loop
    }
  }
  radio.openWritingPipe(addresses[0]);    // Setting the address at which we will send the data
  radio.openReadingPipe(1, addresses[1]); // Setting the address at which we will receive the data

  radio.setPALevel(RF24_PA_MIN, 0);
  radio.setDataRate( RF24_250KBPS );

  radio.startListening();

  pinMode(SW_PIN, INPUT);
  //attachInterrupt(digitalPinToInterrupt(SW_PIN), joyButton, RISING);
}
 
unsigned long receiveTimer = 0;
unsigned long currentmilli = 0;

void updateMessage() {
  // so you can see that new data is being sent
  rxNum += 1;
  if (rxNum > '9') {
    rxNum = '0';
  }
  dataToSend[8] = rxNum;
}

typedef struct
{
  unsigned long timestamp;
  char dataText[10];
}
sendingdata;
sendingdata Txdata;
sendingdata Rxdata;

//===== Sending =====//
void transmitRFnetwork(){
  static unsigned long sendingTimer = 0;

  // Every x seconds...
  unsigned long currentmilli = millis();
  if(currentmilli - sendingTimer > 15000){
    sendingTimer = currentmilli;

    updateMessage();
    Txdata.timestamp = currentmilli;
    for(int i=0; i<10; i++)
    {
      Txdata.dataText[i] = dataToSend[i];
    }
    radio.stopListening();                // This sets the module as transmitter
    radio.write(&Txdata, sizeof(Txdata)); // Sending the data
    radio.startListening();
  }
}

void loop() {

  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);

  Serial.print(F("X: "));
  Serial.print(xValue);
  Serial.print(F(", Y: "));
  Serial.println(yValue);

  currentmilli = millis();

  while (radio.available()){
    radio.read(&Rxdata, sizeof(Rxdata));
    receiveTimer = currentmilli;
  }

  transmitRFnetwork();

  delay(300);

}

void joyButton(){
  Serial.println(F("Button pressed"));
}
