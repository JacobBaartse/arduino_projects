
// ps2 protocol
// https://www.burtonsys.com/ps2_chapweske.htm


int rfReceiverPin = 2;
const int buffer_size = 1024;

//variables used in and outside the interrupt routine
volatile unsigned long trace_array[buffer_size]; // circulair buffer
volatile int trace_index = 0;
volatile int sequence_index = 0;
volatile unsigned long prev_micros = 0;


void setup() {
    Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
  }

  // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(rfReceiverPin, INPUT);

}
bool prev_RfVal = true;
void loop() {
  unsigned long cur_micros = micros();
  unsigned long elapsed_micros = (cur_micros - prev_micros);

  bool rfVal = digitalRead(rfReceiverPin);
  if (rfVal!=prev_RfVal){
    prev_RfVal = rfVal;
    trace_array[trace_index] = elapsed_micros;
    prev_micros = cur_micros;
    trace_index +=  1;
    if (trace_index > buffer_size){
      trace_index = 0;
    }
  }
  if (elapsed_micros>200000  & trace_index>2){
    int count = 0;
    int sum = 0;
    for (int i=0;i<trace_index; i++){
      Serial.print(trace_array[i]);
      Serial.print(",");
      if (trace_array[i]<110){
        count++;
        sum += trace_array[i];
      }
    }
    Serial.println();
    Serial.print("Estimated baud rate: ");
    Serial.println(1000000 * count / sum);
    Serial.println();
    trace_index = 0;
  }

}
