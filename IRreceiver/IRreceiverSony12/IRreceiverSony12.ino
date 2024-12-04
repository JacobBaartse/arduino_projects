//https://arduino-tutorials.net/tutorial/control-arduino-with-ir-receiver-and-remote

//https://www.vdrelectronics.com/tutorial-infrarood-zender-en-ontvanger-maken-met-arduino


//https://github.com/Arduino-IRremote/Arduino-IRremote


const int rfReceiverPin = 2; // should be a pin that supports interrupts
const int buffer_size = 1024;
const int start_indicator_val = 5;
const int one_bit_val = 3;
const int zero_bit_val = 2;

//variables used in and outside the interrupt routine
volatile byte trace_array[buffer_size]; // circulair buffer
volatile int trace_index = 0;
volatile int sequence_index = 0;
volatile unsigned long prev_micros = 0;
int prev_trace_index = 0;
unsigned long prv_rfcommand = 0;
const bool debug = false;

unsigned long getIrCode(){
  bool decoding = true;
  int tmp_trace_index;
  bool start_indicator_found;

  while (decoding){

    // find start indicator
    tmp_trace_index = trace_index;
    start_indicator_found = false;
    if (trace_index != prev_trace_index){
      if (debug){ Serial.print("trace_index");    Serial.println(trace_index); }
      if (debug){ Serial.print("prev_trace_index");    Serial.println(prev_trace_index); }
    }

    if (tmp_trace_index<prev_trace_index) tmp_trace_index += buffer_size; // trace index wrapped
    for (int index=prev_trace_index; index<tmp_trace_index; index++){
      if (debug){ Serial.print(trace_array[index%buffer_size]);       Serial.print(" "); }
      prev_trace_index = index % buffer_size;
      if (trace_array[index%buffer_size] == start_indicator_val){
        start_indicator_found = true; 
        if (debug) Serial.println();
        if (debug) Serial.println("start_indicator_found");
        break;       
      }
    }


    // decode the data
    bool all_decoded = false;
    if (start_indicator_found){
      if (debug) Serial.println("decode the data");
      tmp_trace_index = trace_index;  // skip the start indicator
      if (tmp_trace_index < prev_trace_index) tmp_trace_index += buffer_size; // trace index wrapped.
      if (tmp_trace_index > prev_trace_index + 12){ // is there enough data to decode?
        if (debug) Serial.println("enough data to decode");
        unsigned long decoded_value = 0;
        int bit_num = 0;
        for (int index = prev_trace_index+1; index < prev_trace_index + 13; index ++){  // skip the start indicator
          unsigned long bitval = trace_array[index % buffer_size] / one_bit_val;
          if (debug) Serial.print(bitval);

          if (bitval == start_indicator_val) {  // a new start indicator found so move forward
            if (debug) Serial.println("start indicator found go to next");
            prev_trace_index = (index + 1) % buffer_size;  
            break;
          }
          decoded_value += (bitval << bit_num);
          bit_num ++;
          if (bit_num == 11){
            
            if (debug) { Serial.print("0x"); Serial.println(decoded_value, HEX); }
            prev_trace_index = (index + 1) % buffer_size;
            return decoded_value;
          }
        }
      }
      else{
        decoding = false;
      }
    }
    else{
      decoding = false;
    }
  }
  return 0;
}


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  for (int i=0;i<buffer_size;i++){
    trace_array[i] = 0;
  }
  pinMode(rfReceiverPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(rfReceiverPin), logRfTime, RISING);
  Serial.println("Started");
}


int trace_index_read = 0;
void loop() {
  unsigned long rfcommand = getIrCode();
  if (rfcommand>0)  {
    Serial.print("0x");
    Serial.println(rfcommand, HEX);
  }
  // if (rfcommand > 0){
  //   if (prv_rfcommand != rfcommand){ // store rf code only once (TBD per 4 seconds)
  //     // String ButtonCode = buttonfromrfcode(rfcommand);
  //     sequence_index++;
  //     sequence_index = sequence_index % 256; // keep it in 1 byte
  //     Serial.print("Sequence: ");
  //     Serial.print(sequence_index);
  //     Serial.print(", control button: ");
  //     Serial.println(ButtonCode);
  //   }
  //   prv_rfcommand = rfcommand;
  // }
  // while (trace_index_read < trace_index){
  //   Serial.print(trace_array[trace_index_read]);
  //   Serial.print(",");
  //   trace_index_read+=1;
  // }
  // Serial.println();

  delay(100);
  // digitalWrite(LED_BUILTIN, LOW);
}

void logRfTime(){
  /*
    Start indicator   more than: 29809 usec       
    een                           1785 usec 
    nul                           1190 usec
  */
  int cur_duration = 0;
  unsigned long cur_micros = micros();
  int elapsed_micros = (cur_micros - prev_micros);
  if (elapsed_micros > 20000){
    cur_duration = 5;  // start of code detected
  }
  else if (elapsed_micros > 2200){
    cur_duration = 4;  // invalid 
  }
  else if (elapsed_micros > 1480){
    cur_duration = 3;  // one detected
  }
  else if (elapsed_micros > 800){
    cur_duration = 2;  // zero detected
  }
  else{
    cur_duration = 1;  // invalid
  }

  // cur_duration = elapsed_micros;
  
  if (cur_duration > 0){
    trace_array[trace_index] = cur_duration;
    trace_index += 1;
    trace_index = trace_index % buffer_size;
  }
  prev_micros = cur_micros;
}
