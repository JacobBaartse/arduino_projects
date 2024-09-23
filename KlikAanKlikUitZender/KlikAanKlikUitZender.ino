int OUTPUTPIN = LED_BUILTIN;


int stop, start, counter1, counter2;


void setup() {

    Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void delay_370u(int repeats){
  for (int j=0; j<repeats; j++){
    for  (int i = 1; i<178; i++){  // loop to get 370useconds
      counter2 = micros();
    }
  }
}

void set_output(int duration, int state){
  digitalWrite(OUTPUTPIN, state);
  delay_370u(duration);
}

void send_bit(int bitval){
  set_output(1, LOW);
  if (bitval == 0){
    set_output(1, HIGH);
    set_output(1, LOW);
    set_output(3, HIGH);
    set_output(1, LOW);
  }
  else{
    set_output(3, HIGH);
    set_output(1, LOW);
    set_output(1, HIGH);
    set_output(1, LOW);
  }
}


long code = 0x50505050;


void loop(){
  //test the timing of the 370u loop 
  // while (true){
  //   start = micros();
  //   delay_370u(100);
  //   stop = micros();  //shoul result in 37000 us
  //   Serial.println(stop-start);
  // }

  for (int k = 0; k<3; k++){
    set_output(25, HIGH); // delay betwen codes. 9ms

    set_output(1, LOW);// start sequence
    set_output(7, HIGH);
    set_output(1, LOW);

    for (int p=0; p<32; p++){  //lsb first
      int bitval = (code >> p) % 2;
      send_bit(bitval);
    }
  }
  delay(1000);
}
