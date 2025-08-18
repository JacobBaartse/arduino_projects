/*
 * relays driving stuff
 */

#define pinLightS 4      // relays driver pin
#define pinLightR 5      // relays driver pin

void setupdrivers(){

  pinMode(pinLightS, OUTPUT);
  pinMode(pinLightR, OUTPUT);

}

uint8_t relaysstate = 0;

void setLight(bool LightOn){
  // if (LightOn){
  //   digitalWrite(pinLight1, HIGH);
  //   digitalWrite(pinLight2, HIGH);
  //   relaysstate = 0xf0;
  // }
  // else {
  //   digitalWrite(pinLight1, LOW);
  //   digitalWrite(pinLight2, LOW);
  //   relaysstate = 0x0f;
  // }
}

bool getLight(){
  if (relaysstate == 0xf0) return true;
  return false;
}
