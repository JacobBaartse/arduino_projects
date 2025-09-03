/*
 * relays driving stuff
 */

#define pinLightS 4      // relays driver Set pin (Set)
#define pinLightR 5      // relays driver Reset pin (Clear)
#define pinDetect 6      // monitor pin (Detect)

void setupdrivers(){

  pinMode(pinLightS, OUTPUT);
  pinMode(pinLightR, OUTPUT);

  digitalWrite(pinLightS, HIGH);
  digitalWrite(pinLightR, HIGH);

}

uint8_t relaysstate = 0;

bool lightOn(){
  return (relaysstate == 0x0f);
}

void setLight(uint8_t Light, unsigned long Lightmilli){
  static unsigned long LightTime = 0;
  static unsigned long LightOnTime = 0;
  static uint16_t aoncount = 0;
  bool lStatus = lightOn();

  if (Light > 0){
    if (lStatus){
      Serial.print(F("Already ON "));
      aoncount += 1;
      Serial.print(aoncount);
      Serial.print(F(" for "));
      unsigned long timedur = (Lightmilli - LightTime) / 1000;
      unsigned long timediff = (Lightmilli - LightOnTime) / 1000;
      Serial.print(timedur);
      Serial.print(F(" seconds, diff: "));
      Serial.print(timediff);
      Serial.println(F(" seconds"));
      LightOnTime = Lightmilli;
    }
    else {
      digitalWrite(pinLightS, LOW);
      delay(500);
      Serial.println(F("Going ON"));
      digitalWrite(pinLightS, HIGH);
      relaysstate = 0x0f;
      aoncount = 0;
      LightTime = Lightmilli;
      LightOnTime = Lightmilli;
    }
  }
  else {
    if (!lStatus){
      Serial.println(F("Already OFF"));
    }
    else {
      digitalWrite(pinLightR, LOW);
      delay(500);
      Serial.println(F("Going OFF"));
      digitalWrite(pinLightR, HIGH);
      relaysstate = 0xf0;
    }
  }
}

void driveRelays(uint8_t dstatus, unsigned long currentDrivermilli){
  static unsigned long drivertime = 0;
  static uint8_t driverstatus = 0;
  static bool relaysstatus = false;

  if ((unsigned long)(currentDrivermilli - drivertime) > 10000){
    drivertime = currentDrivermilli;
    if ((driverstatus & 0xf0) > 0){ // button pressed
      ButtonActive = false;
      //Serial.println(F("BUTTON reset"));
      setLight(0, currentDrivermilli);
    }
  }
  
  if (driverstatus != dstatus){
    if ((dstatus & 0xf0) > 0){ // button pressed
      //Serial.println(F("BUTTON"));
      drivertime = currentDrivermilli;
    }
    if ((dstatus & 0x0f) > 0){ // PIR detected
      //Serial.println("PIR");
      setLight(dstatus, currentDrivermilli);
    }
    // if (dstatus > 0){
    //   Serial.print(currentDrivermilli);
    //   Serial.print(F(" dstatus "));
    //   Serial.print(dstatus, HEX);
    //   Serial.print(F(" "));
    //   Serial.println(dstatus, BIN);
    // }

    driverstatus = dstatus;
  }

}
