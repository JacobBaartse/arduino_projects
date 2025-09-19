/*
 * PIR sensor and distance sensor as well as on and off buttons
 */

// #define pinPIR1 6        // PIR pin connection
// #define pinPIR2 7        // PIR pin connection

#define pinPressButton 2 // light off button

bool ButtonActive = false;

void setupsensors(){

  // pinMode(pinPIR1, INPUT);
  // pinMode(pinPIR2, INPUT);

  pinMode(pinPressButton, INPUT_PULLUP);
}

// uint8_t checkSensors(unsigned long currentSensormilli)
// {
//   static unsigned long sensortime = 0;
//   static bool detectionval = false;
//   uint8_t status = 0;

//   bool det1 = digitalRead(pinPIR1) == HIGH;
//   //bool det2 = digitalRead(pinPIR2) == HIGH;    

//   if (detectionval != det1){
//     Serial.print(currentSensormilli);
//     Serial.print(F(" PIR1 "));
//     Serial.println(digitalRead(pinPIR1));      
//     detectionval = det1;
//   }

//   if (detectionval){
//     status = 3;
//   }
//   if (ButtonActive){
//     // if ((unsigned long)(currentSensormilli - sensortime) > 10000){
//     // }
//     // else {
//     //   sensortime = currentSensormilli;
//     //   status += 128;
//     // }
//     status += 128;
//   }  
//   return status;
// }

void buttonPress(){
  static uint8_t pressamount = 0;

  if (!ButtonActive){
    pressamount += 1;
    ButtonActive = true;
    Serial.print(millis());
    Serial.print(F(" Buttonpress: "));
    Serial.println(pressamount);
  }
}
