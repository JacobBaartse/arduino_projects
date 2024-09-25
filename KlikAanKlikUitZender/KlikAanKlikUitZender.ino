int OUTPUTPIN = 3;


int stop, start, counter1, counter2;


void setup() {

    Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(OUTPUTPIN, OUTPUT);
}

void send_times_t(int repeats, int state){
  for (int j=0; j<repeats; j++){
    digitalWrite(OUTPUTPIN, state); // this takes also time so do it for every loop
    for  (int i = 1; i<157; i++){  // loop to get 338 useconds
      counter2 = micros();  // takes about 1.8us
    }
  }
}


void send_bit(int bitval){
  if (bitval == 0){
    send_times_t(1, LOW);
    send_times_t(1, HIGH);
    send_times_t(3, LOW);
    send_times_t(1, HIGH);
  }
  else{
    send_times_t(3, LOW);
    send_times_t(1, HIGH);
    send_times_t(1, LOW);
    send_times_t(1, HIGH);
  }
}



void send_code(long code){
    for (int k = 0; k<10; k++){
    //start = millis();
    send_times_t(27, LOW); // delay betwen codes. 9ms

    send_times_t(1, HIGH);// start sequence
    send_times_t(7, LOW);
    send_times_t(1, HIGH);

    for (int p=31; p>=0; p--){  //msb first
      int bitval = (code >> p) % 2;
      send_bit(bitval);
    }
    digitalWrite(OUTPUTPIN, LOW);
  }
}



void loop(){
  //test the timing of the 370u loop 
  // while (true){
  //   start = micros();
  //   delay_370u(100);
  //   stop = micros();  //shoul result in 33800 us
  //   Serial.println(stop-start);
  // }

  //send_code(0x09002309);
  send_code(0x90c40090);   // aan 1
  delay(1000);
  send_code(0x90c40080);   // uit 1
  delay(1000);
  send_code(0x90c40091);   // aan 2
  delay(1000);
  send_code(0x90c40081);   // uit 2
  delay(1000);
  send_code(0x90c40092);   // aan 3
  delay(1000);
  send_code(0x90c40082);   // uit 3
  delay(1000);

  send_code(0x90c40090);   // aan 1
  send_code(0x90c40091);   // aan 2
  send_code(0x90c40092);   // aan 3

  delay(2000);
  send_code(0x90c400A0);  // alles uit
}
