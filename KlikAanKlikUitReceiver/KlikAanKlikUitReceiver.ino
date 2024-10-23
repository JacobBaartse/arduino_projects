
int rfReceiverPin = 2; // should be a pin that supports interrupts
const int buffer_size = 1024;
const int start_indicator_val = 6;
const int one_bit_val = 3;

//variables used in and outside the interrupt routine
volatile byte trace_array[buffer_size]; // circulair buffer
volatile int trace_index = 0;
volatile int sequence_index = 0;
volatile unsigned long prev_micros = 0;

int prev_trace_index = 0;
unsigned long prv_rfcommand = 0;
const bool debug = false;

unsigned long getRfCode(){
  bool decoding = true;
  int tmp_trace_index;
  bool start_indicator_found;

  while (decoding){

    // find start indicator
    tmp_trace_index = trace_index;
    start_indicator_found = false;
    if (trace_index != prev_trace_index){
      if (debug) Serial.print("trace_index");    Serial.println(trace_index);
      if (debug) Serial.print("prev_trace_index");    Serial.println(prev_trace_index);
    }
    
    if (tmp_trace_index<prev_trace_index) tmp_trace_index += buffer_size; // trace index wrapped
    for (int index=prev_trace_index; index<tmp_trace_index; index++){
      if (debug) Serial.print(trace_array[index%buffer_size]);       Serial.print(" ");
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
      if (tmp_trace_index > prev_trace_index + 64){ // is there enough data to decode?
        if (debug) Serial.println("enough data to decode");
        unsigned long decoded_value = 0;
        int bit_num = 32;
        for (int index = prev_trace_index+1; index < prev_trace_index + 64; index += 2){  // skip the start indicator
          bit_num --;
          unsigned long bitval = trace_array[index % buffer_size] / one_bit_val;
          int next_val = trace_array[(index + 1) % buffer_size] / one_bit_val;
          if (debug) Serial.print(bitval);       Serial.print(" ");
          if (debug) Serial.print(next_val);     Serial.print(" ");
          if (bitval == next_val){  // a wrong bit sequence has been detected so move forward
            if (debug) Serial.println("wrong sequence detected move forward");
            prev_trace_index = (index + 1) % buffer_size;  
            break; 
          }
          if (bitval == start_indicator_val) {  // a new start indicator found so move forward
            if (debug) Serial.println("start indicator found go to next");
            prev_trace_index = (index + 1) % buffer_size;  
            break; 
          }
          if (next_val == start_indicator_val) {  // a new start indicator found so move forward
            if (debug) Serial.println("next_val start indicator found go to next");
            prev_trace_index = (index + 2) % buffer_size;  
            break; 
          }
          decoded_value += (bitval << bit_num);
          if (bit_num == 0){
            Serial.print("0x");
            if (debug) Serial.println(decoded_value, HEX);
            prev_trace_index = (index+2) % buffer_size;  
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

String buttonfromrfcode(unsigned long rfcode){
  String ButtonId = "noId";
  switch(rfcode){
    case 0x90C40090:
      ButtonId = "R1B1";
      break;
    case 0x90C40080:
      ButtonId = "R1B2";
      break;
    case 0x90C40091:
      ButtonId = "R1B3";
      break;
    case 0x90C40081:
      ButtonId = "R1B4";
      break;
    case 0x90C40092:
      ButtonId = "R1B5";
      break;
    case 0x90C40082:
      ButtonId = "R1B6";
      break;
    case 0x90C400A0:
      ButtonId = "R1B7";
      break;
    default:
      Serial.print("Found RF code: ");
      Serial.println(rfcode);
  }

  Serial.println(" ");
  Serial.print("Remote control button: ");
  Serial.println(ButtonId);
  return ButtonId;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }
  for (int i=0;i<buffer_size;i++){
    trace_array[i] = 0;
  }
  // pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rfReceiverPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(rfReceiverPin), logRfTime, RISING);
  Serial.println("Started");
}

void loop() {
  unsigned long rfcommand = getRfCode();
  if (rfcommand > 0){
    if (prv_rfcommand != rfcommand){ // store rf code only once (TBD per 4 seconds)
      // digitalWrite(LED_BUILTIN, HIGH);
      String ButtonCode = buttonfromrfcode(rfcommand);
      sequence_index++;
      sequence_index = sequence_index % 256; // keep it in 1 byte
      Serial.print("Sequence: ");
      Serial.print(sequence_index);
      Serial.print(", control button: ");
      Serial.println(ButtonCode);
    }
    prv_rfcommand = rfcommand;
  }

  delay(100);
  // digitalWrite(LED_BUILTIN, LOW);
}

void logRfTime(){
  /*
    spacing between codes: 21   will not be stored.
                                          measured         configured
    Start indicator        6          : 2788 .. 2859  >> 2600 -- 3000
    een                    3, 1     3 : 1482 .. 1572  >> 1350 -- 1700
    nul                    1, 3     1 :  460 .. 530   >>  350 -- 650
  */
  byte cur_duration = 0;
  unsigned long cur_micros = micros();
  unsigned long elapsed_micros = (cur_micros - prev_micros); ///95/5;
  if (elapsed_micros < 650){
    if (elapsed_micros > 350)
      cur_duration = 1;
  }
  else{
    if (elapsed_micros < 1700){
      if (elapsed_micros > 1350)
        cur_duration = 3;
    }
    else{
      if (elapsed_micros < 3000)
        if (elapsed_micros > 2600)
          cur_duration = 6;      
    }
  }
  
  if (cur_duration > 0){
    trace_array[trace_index] = cur_duration;
    trace_index += 1;
    trace_index = trace_index % buffer_size;
  }
  prev_micros = cur_micros;
}
