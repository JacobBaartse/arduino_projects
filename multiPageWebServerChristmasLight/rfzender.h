

int stop, start, counter1, counter2;
int OUTPUTPIN;
unsigned long RF_LIGHT_ON1 = 0x90c40090;
unsigned long RF_LIGHT_OFF1 = 0x90c40080;

unsigned long RF_LIGHT_ON2 = 0x90c40091;
unsigned long RF_LIGHT_OFF2 = 0x90c40081;

unsigned long RF_LIGHT_ON3 = 0x90c40092;
unsigned long RF_LIGHT_OFF3 = 0x90c40082;

unsigned long RF_LIGHT_ON4 = 0x90c40094;
unsigned long RF_LIGHT_OFF4 = 0x90c40084;

unsigned long RF_LIGHT_ON5 = 0x90c40095;
unsigned long RF_LIGHT_OFF5 = 0x90c40085;

unsigned long RF_LIGHT_ON6 = 0x90c40096;
unsigned long RF_LIGHT_OFF6 = 0x90c40086;

unsigned long RF_LIGHT_ALL_ON = 0x90c40093;
unsigned long RF_LIGHT_ALL_OFF = 0x90c40083;

unsigned long RF_WISS_CONFIG = 0x90c400A0;


void rf_setup(int pin) {
  pinMode(pin, OUTPUT);
  OUTPUTPIN = pin;
}

unsigned long prev_micros = 0;
const int micros_per_t = 338; 


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
  for (int k = 0; k<7; k++){
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



void example_code(){
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
