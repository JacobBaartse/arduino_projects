/*
 *
 */

#include "matrix_p.h"

void setup() {
  digitalWrite(LED_BUILTIN, HIGH);

  matrix.begin();
  loadstaticpicture(PICT_LIKE);
  Serial.begin(115200);

  digitalWrite(LED_BUILTIN, LOW);
}

int sequencetorun = SEC_NONE;
bool running = false;
void loop() {
  running = runsequence(sequencetorun);
  if (!running){
    sequencetorun = SEC_HEART; // random(0, 2); //sizeof(definedSequences));
  }
}
