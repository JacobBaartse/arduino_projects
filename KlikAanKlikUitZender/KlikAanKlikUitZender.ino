int OUTPUTPIN = 8;


int stop, start, counter1, counter2;


void setup() {

    Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  pinMode(OUTPUTPIN, OUTPUT);
}

unsigned long prev_micros = 0;
//const int micros_per_t = 100;  // tested with receiver, receiver could barely follow the sort pulses.
//const int micros_per_t = 200;  // fails
//const int micros_per_t = 250;  //works
//const int micros_per_t = 300;  //works
const int micros_per_t = 338;  //works same as remote
// const int micros_per_t = 350; //works
// const int micros_per_t = 400;  //works
// const int micros_per_t = 500;  // fails

void send_times_t(int repeats, int state){
  digitalWrite(OUTPUTPIN, state);
  unsigned long end_micros = prev_micros + (micros_per_t*repeats);  // to prevent looping of micros every 71 minutes
  if (end_micros < prev_micros){
    //todo handle wrapping of micros
  }

  //wait until time is elapsed.
  while ( prev_micros <= end_micros){
    prev_micros = micros();
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
    for (int k = 0; k<5; k++){
    //start = millis();
    prev_micros = micros();
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
  delay(2000);
  send_code(0x90c40080);   // uit 1
  delay(2000);
  send_code(0x90c40091);   // aan 2
  delay(2000);
  send_code(0x90c40081);   // uit 2
  delay(2000);
  send_code(0x90c40092);   // aan 3
  delay(2000);
  send_code(0x90c40082);   // uit 3
  delay(2000);

  send_code(0x90c40090);   // aan 1
  send_code(0x90c40091);   // aan 2
  send_code(0x90c40092);   // aan 3

  delay(2000);
  send_code(0x90c400A0);  // alles uit
  delay(2000);
}
