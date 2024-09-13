/*
 *
 */

#include "matrix_p.h"

void setup() {
  digitalWrite(LED_BUILTIN, HIGH);
  loadstaticpicture(PICT_LIKE);
  delay(2000);
  digitalWrite(LED_BUILTIN, LOW);
}

int sequencetorun = SEC_NONE;
bool running = false;
void loop() {
  running = runsequence(SEC_NONE, 1000);
}
