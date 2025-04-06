/*
  Arduino Wireless Network - Multiple NRF24L01 Tutorial
        == Node 01 (Child of Master node 00) ==

https://howtomechatronics.com/tutorials/arduino/how-to-build-an-arduino-wireless-network-with-multiple-nrf24l01-modules/#h-node-01-source-code

Target: NANO (Atmel atmega328pb Xplained mini), with RF24 module Long Range
nRF24L01 (CE,CSN) connected to pin 8, 7
location JWF21


 Choice between internal and external antenna:

 https://forum.arduino.cc/t/external-antenna-of-rf-nano/1245897

 https://www.instructables.com/Enhanced-NRF24L01/



@todo send message to repeater
@todo define remote channel

*/

#include <SPI.h>
#include <RF24.h>
#include <RF24Network.h>

#define buttonPinGreen 7  
#define buttonPinRed 8  
#define ledPinGreen 4  
#define ledPinRed 6 

enum LEDState {
    LEDOff = 0,
    LEDFlashing = 1,
    LEDOn = 2,
};

const int debounceDelay = 50;

bool greendebounce(bool pressedbutton, unsigned long curtime){
  static bool buttonstate = false;
  static unsigned long lastdebouncetime = 0;

  if (pressedbutton != buttonstate) {
    if ((unsigned long)(curtime - lastdebouncetime) > debounceDelay) {
      buttonstate = pressedbutton;
    }
    lastdebouncetime = curtime;
  }
  return buttonstate;
}

bool reddebounce(bool pressedbutton, unsigned long curtime){
  static bool buttonstate = false;
  static unsigned long lastdebouncetime = 0;

  if (pressedbutton != buttonstate) {
    if ((unsigned long)(curtime - lastdebouncetime) > debounceDelay) {
      buttonstate = pressedbutton;
    }
    lastdebouncetime = curtime;
  }
  return buttonstate;
}

LEDState greenledprocessing(unsigned long curtime, bool buttonpressed) {
  static LEDState ledstatus = LEDState::LEDOff;
  static unsigned long ledinterval = 500;
  static unsigned long ledstateinterval = 5000;
  static unsigned long ledtime = 0;
  static unsigned long leddurationtime = 0;
  static bool previousbuttonpressed = false;

  if (buttonpressed) {
    if (buttonpressed != previousbuttonpressed) { // only start once
      if (ledstatus == LEDState::LEDOff) {
        pinMode(ledPinGreen, OUTPUT);
        digitalWrite(ledPinGreen, LOW);
        ledstatus = LEDState::LEDFlashing;
        ledtime = (unsigned long)(curtime - 2 * ledinterval); // make sure the blinking start directly
        leddurationtime = curtime;
      }
      else { 
        ledstatus = LEDState::LEDOff;
      }
    }
  }

  switch(ledstatus) {
    case LEDState::LEDFlashing:
      if ((unsigned long)(curtime - ledtime) > ledinterval) {
        digitalWrite(ledPinGreen, !digitalRead(ledPinGreen));
        ledtime = curtime;
      }
      break;
    case LEDState::LEDOn:
      digitalWrite(ledPinGreen, HIGH);
      break;
    //case LEDState::LEDOff:
    default:
      digitalWrite(ledPinGreen, LOW);
      pinMode(ledPinGreen, INPUT_PULLUP);
  }

  if ((unsigned long)(curtime - leddurationtime) > ledstateinterval) {
    switch(ledstatus) {
      case LEDState::LEDFlashing:
        ledstatus = LEDState::LEDOn;
      break;
      // case LEDState::LEDOff:
      // case LEDState::LEDOn:
      default:
        ledstatus = LEDState::LEDOff;
    }
    leddurationtime = curtime;
  }

  previousbuttonpressed = buttonpressed;
  return ledstatus;
}

LEDState redledprocessing(unsigned long curtime, bool buttonpressed) {
  static LEDState ledstatus = LEDState::LEDOff;
  static int ledinterval = 250;
  static int ledstateinterval = 10000;
  static unsigned long ledtime = 0;
  static unsigned long leddurationtime = 0;
  static bool previousbuttonpressed = false;

  if (buttonpressed) {
    if (buttonpressed != previousbuttonpressed) { // only start once
      if (ledstatus == LEDState::LEDOff) {
        pinMode(ledPinRed, OUTPUT);
        digitalWrite(ledPinRed, LOW);
        ledstatus = LEDState::LEDFlashing;
        ledtime = (unsigned long)(curtime - 2 * ledinterval); // make sure the blinking start directly
        leddurationtime = curtime;
      }
      else { 
        ledstatus = LEDState::LEDOff;
      }
    }
  }

  switch(ledstatus) {
    case LEDState::LEDFlashing:
      if ((unsigned long)(curtime - ledtime) > ledinterval) {
        digitalWrite(ledPinRed, !digitalRead(ledPinRed));
        ledtime = curtime;
      }
      break;
    case LEDState::LEDOn:
      digitalWrite(ledPinRed, HIGH);
      break;
    //case LEDState::LEDOff:
    default:
      digitalWrite(ledPinRed, LOW);
      pinMode(ledPinRed, INPUT_PULLUP);
  }

  if ((unsigned long)(curtime - leddurationtime) > ledstateinterval) {
    switch(ledstatus) {
      case LEDState::LEDFlashing:
        ledstatus = LEDState::LEDOn;
      break;
      // case LEDState::LEDOff:
      // case LEDState::LEDOn:
      default:
        ledstatus = LEDState::LEDOff;
    }
    leddurationtime = curtime;
  }

  previousbuttonpressed = buttonpressed;
  return ledstatus;
}

RF24 radio(10, 9);               // onboard nRF24L01 (CE, CSN)
RF24Network network(radio);      // Include the radio in the network
//const uint16_t this_node = 00;   // Address of our node in Octal format (04, 031, etc.)
//const uint16_t repeaternode = 01;    // Address of the other node in Octal format
const uint16_t this_node = 01;   // Address of our node in Octal format (04, 031, etc.)
const uint16_t repeaternode = 00;    // Address of the other node in Octal format
const uint16_t wrappingcounter = 255;

unsigned long const keywordval = 0xabcdfedc; 
unsigned long const command_none = 0x00; 
unsigned long const command_clear_counters = 0x01; 
unsigned long const command_status = 0x02; 
//unsigned long const command_reboot = 0x04; 
//unsigned long const command_unixtime = 0x08; 
unsigned long const response_none = 0x00; 
unsigned long const response_button = 0x01; 
unsigned long const response_ack = 0x02; 

// max 32 bytes can be in the FIFO of the nRF24L01
// that means maximum 8 data items of unsigned long
struct network_payload {
  unsigned long keyword;
  unsigned long counter;
  unsigned long timing;
  unsigned long command;
  unsigned long response;
  unsigned long data1;
  //unsigned long data2;
  //unsigned long data3;
};


void setup() {
  Serial.begin(230400); // actual baudrate in IDE: 57600 (RF-NANO, micro USB), there is somewhere a mismatch in clock factor of 4
  Serial.println();
  Serial.print(__FILE__);
  Serial.print(F(", creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush();  

  pinMode(buttonPinGreen, INPUT_PULLUP);
  pinMode(buttonPinRed, INPUT_PULLUP);
  pinMode(ledPinGreen, INPUT_PULLUP);
  pinMode(ledPinRed, INPUT_PULLUP);

  SPI.begin();
  radio.begin();
  //radio.setPALevel(RF24_PA_MIN, false); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_LOW); // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_HIGH=-6dBM, and RF24_PA_MAX=0dBm.
  //radio.setDataRate(RF24_1MBPS); // (RF24_2MBPS);
  //radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);
  radio.setChannel(100);////
  // radio.setAutoAck(true);
  // radio.enableDynamicPayloads();  
  network.begin(this_node); // (channel, node address)

  Serial.println(F("\n ******"));  
  Serial.println(); 
  Serial.flush(); 
}

unsigned long updatecounter(unsigned long countval, unsigned long wrapping=wrappingcounter) {
  countval++;
  if (countval > wrapping) countval = 1;
  return countval;
}

unsigned long receivedmsg = 0;
unsigned long sendmsg = 0;
unsigned long droppedmsg = 0;
unsigned long failedmsg = 0;

void messageStatus(unsigned long interval)
{
  static unsigned long statustime = 0;
  if (interval > 0){
    if (millis() < statustime) return;
    statustime = millis() + interval;
  }
  Serial.print(F("Network messages "));
  Serial.print(F("received: "));
  Serial.print(receivedmsg);
  Serial.print(F(", send: "));
  Serial.print(sendmsg);
  Serial.print(F(", missed: "));
  Serial.print(droppedmsg);
  Serial.print(F(", failed: "));
  Serial.print(failedmsg);
  Serial.println();  
}

unsigned long sendingTimer = 0;
unsigned long sendingCounter = 0;
unsigned long receiveCounter = 0;
unsigned long rcvmsgcount = 10000;

unsigned long commanding = command_none;
unsigned long responding = response_none;
unsigned long data1 = response_none;
unsigned long data2 = response_none;
unsigned long data3 = response_none;

bool printstatus = false;
unsigned long receivedcommand = command_none;
unsigned long commandfrombase = command_none;

unsigned long currentMillis = 0;    // stores the value of millis() in each iteration of loop()
bool buttonGreenPressed = false;
bool buttonRedPressed = false;
LEDState GreenIndication = LEDState::LEDOff;
LEDState RedIndication = LEDState::LEDOff;

void loop() {
  
  currentMillis = millis();   // capture the value of millis() only once in the loop

  GreenIndication = greenledprocessing(currentMillis, buttonGreenPressed);
  RedIndication = redledprocessing(currentMillis, buttonRedPressed);

  buttonGreenPressed = greendebounce((digitalRead(buttonPinGreen) == LOW), currentMillis);
  buttonRedPressed = reddebounce((digitalRead(buttonPinRed) == LOW), currentMillis);

  if (printstatus) {
    messageStatus(0);
    printstatus = false;
  }

  network.update();

  //===== Receiving =====//
  while (network.available()) { // any incoming data?
    RF24NetworkHeader header;
    network_payload incomingData;
    network.read(header, &incomingData, sizeof(incomingData)); // Read the incoming data
    if (header.from_node != repeaternode) {
      Serial.print(F("Received unexpected message, from_node: "));
      Serial.println(header.from_node);
      break;
    }
    receivedmsg++;
    // Serial.print(F("IncomingData: "));
    // Serial.println(incomingData.counter);
    // check keyword and sequencenumber
    if (incomingData.keyword == keywordval){
      receiveCounter = incomingData.counter;
      receivedcommand = incomingData.command;
      if (rcvmsgcount > wrappingcounter) { // initialisation
        rcvmsgcount = receiveCounter;
      }
      else { // check received message value
        if (rcvmsgcount != receiveCounter) {
          if (receivedmsg > 1) {
            droppedmsg++; // this could be multiple as well
            Serial.print(F("Missed network message(s): "));
            Serial.print(F("received id: "));
            Serial.print(receiveCounter);
            Serial.print(F(", expected id: "));
            Serial.println(rcvmsgcount);             
          }
          rcvmsgcount = receiveCounter; // re-synchronize
        }
      }
      rcvmsgcount = updatecounter(rcvmsgcount); // calculate next expected message 
    }
    else {
      Serial.println(F("Keyword failure"));
      Serial.println(incomingData.keyword, HEX);
      receivedcommand = command_none;
    }
    if (receivedcommand > command_none) {
      commandfrombase = receivedcommand;
    }

    network.update();

  }

  //===== Sending =====//
  // Meanwhile, every x seconds...
  unsigned long currentmilli = millis();
  if(currentmilli - sendingTimer > 15000) {
    sendingTimer = currentmilli;
    sendingCounter = updatecounter(sendingCounter); 
    RF24NetworkHeader headerR(repeaternode, "E"); // address where the data is going

    if (commandfrombase > command_none) {
      if ((commandfrombase & command_clear_counters) > 0) {
        printstatus = true;
        receivedmsg = 0;
        sendmsg = 0;
        droppedmsg = 0;
        failedmsg = 0;
      }
      else if ((commandfrombase & command_status) > 0) {
        printstatus = true;
        responding = receivedmsg;
        data1 = sendmsg;
        data2 = droppedmsg;
        data2 = (data2 << 16) + failedmsg;
      }
      else {
        Serial.print(F("TBD implement commandfrombase: "));
        Serial.println(commandfrombase);
      }
      commandfrombase = command_none;
    }

    network_payload outgoing = {keywordval, sendingCounter, currentmilli, commanding, responding, data1};//, data2, data3};
    bool ok = network.write(headerR, &outgoing, sizeof(outgoing)); // Send the data
    if (!ok) {
      Serial.print(F("Retry sending message: "));
      Serial.println(sendingCounter);
      ok = network.write(headerR, &outgoing, sizeof(outgoing)); // retry once
    }
    if (ok) {
      sendmsg++;
    }
    else{
      Serial.print(F("Error sending message: "));
      Serial.println(sendingCounter);
      failedmsg++;
    }
    commanding = command_none;
    responding = response_none;
    data1 = response_none;
    data2 = response_none;
    data3 = response_none;
  }

}
