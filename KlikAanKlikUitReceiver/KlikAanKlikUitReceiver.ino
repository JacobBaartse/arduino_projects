int rfReceiverPin = 2;
const int buffer_size = 1024;
const int start_indicator_val = 5;

//variables used in interrupt routine
volatile byte trace_array[buffer_size] ;  // circulair buffer
volatile int trace_index = 0;
volatile long prev_micros = 0;



int prev_trace_index = 0;


void getRfCode(){
  bool decoding = true;
  int tmp_trace_index;
  bool start_indicator_found;
  while (decoding){

    //find start indicator
    Serial.println("find start indicator");
    tmp_trace_index = trace_index;
    start_indicator_found = false;
    if (tmp_trace_index<prev_trace_index) tmp_trace_index += buffer_size; // trace index wrapped.
    for (int index=prev_trace_index; index<tmp_trace_index ; index++){
      if (trace_array[index%buffer_size] ==start_indicator_val){
        start_indicator_found = true; 
        prev_trace_index = (index + 1) % buffer_size;  // skip to after the start indicator
        break;       
      }
    }

    // decode the data
    bool all_decoded = true;
    if (start_indicator_found){
      Serial.println("decode the data");
      tmp_trace_index = trace_index;
      if (tmp_trace_index<prev_trace_index) tmp_trace_index += buffer_size; // trace index wrapped.
      if (tmp_trace_index > prev_trace_index + 66){ // is there enough data to decode?
        Serial.println("enough data to decode");
        int decoded_value = 0;
        int bit_num = 32;
        for (int index = prev_trace_index; index < prev_trace_index + 64; index += 2){
          int bitval = trace_array[index % buffer_size] / 2;
          Serial.print("index ");
          Serial.println(index);
          Serial.print("bitval ");
          Serial.println(bitval);

          if (bitval>1) {  // a new start indicator found so move forward
              Serial.println("too high value found go to next");
            prev_trace_index = (index + 1) % buffer_size;  
            all_decoded = false;
            break; 
          }
          Serial.print("increase value ");
          Serial.println(bitval << bit_num);
          decoded_value += bitval << bit_num;
        }
        if (all_decoded){
          Serial.print("decoded_val: ");
          Serial.println(decoded_value, HEX);
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
}

void setup() {
  Serial.begin(9600);
  for (int i=0; i< buffer_size; i++){
    trace_array[i] = 0;
  }
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rfReceiverPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(rfReceiverPin), logRfTime, RISING);
}


void loop() {
  // int state = digitalRead(rfReceiverPin);              
  // digitalWrite(LED_BUILTIN, state);
  // delay(10);
  for (int index =0; index<400; index++){
    Serial.print(trace_array[index]);
    Serial.print(" ");
  }
  Serial.println();
  getRfCode();
  delay(1000);
}


void logRfTime(){
  /*
    spacing between codes: 21   will not be stored.
                                          measured         configured
    Start indicator        6          : 2788 .. 2859  >> 2700 -- 2940
    een                    3, 1     3 : 1482 .. 1572  >> 1400 -- 1650
    nul                    1, 3     1 :  460 .. 530   >>  380 -- 610
 */
  byte cur_duration = 0;
  long cur_micros = micros();
  long elapsed_micros = (cur_micros - prev_micros); ///95/5;
  if (elapsed_micros<650){
    if (elapsed_micros>350)
      cur_duration = 1;
  }
  else{
    if (elapsed_micros<1700){
      if (elapsed_micros>1350)
        cur_duration = 3;
    }
    else{
      if (elapsed_micros<3000)
        if (elapsed_micros>2600)
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