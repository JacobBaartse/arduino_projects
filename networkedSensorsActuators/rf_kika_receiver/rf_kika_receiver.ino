/*
 *
 */

#include "RF24Network.h"
#include "RF24.h"
#include <SPI.h>
 
#define radioChannel 96
#define CE_PIN 10
#define CSN_PIN 9

const int rfReceiverPin = 2; // should be a pin that supports interrupts
const int buffer_size = 1024;
const int start_indicator_val = 6;
const int one_bit_val = 3;

// variables used in and outside the interrupt routine
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
      if (debug) Serial.print(trace_array[index%buffer_size]);       Serial.print(F(" "));
      prev_trace_index = index % buffer_size;
      if (trace_array[index%buffer_size] == start_indicator_val){
        start_indicator_found = true; 
        if (debug) Serial.println();
        if (debug) Serial.println(F("start_indicator_found"));
        break;       
      }
    }

    // decode the data
    bool all_decoded = false;
    if (start_indicator_found){
      if (debug) Serial.println(F("decode the data"));
      tmp_trace_index = trace_index;  // skip the start indicator
      if (tmp_trace_index < prev_trace_index) tmp_trace_index += buffer_size; // trace index wrapped.
      if (tmp_trace_index > prev_trace_index + 64){ // is there enough data to decode?
        if (debug) Serial.println(F("enough data to decode"));
        unsigned long decoded_value = 0;
        int bit_num = 32;
        for (int index = prev_trace_index+1; index < prev_trace_index + 64; index += 2){  // skip the start indicator
          bit_num --;
          unsigned long bitval = trace_array[index % buffer_size] / one_bit_val;
          int next_val = trace_array[(index + 1) % buffer_size] / one_bit_val;
          if (debug) Serial.print(bitval);       Serial.print(F(" "));
          if (debug) Serial.print(next_val);     Serial.print(F(" "));
          if (bitval == next_val){  // a wrong bit sequence has been detected so move forward
            if (debug) Serial.println(F("wrong sequence detected move forward"));
            prev_trace_index = (index + 1) % buffer_size;
            break; 
          }
          if (bitval == start_indicator_val) {  // a new start indicator found so move forward
            if (debug) Serial.println(F("start indicator found go to next"));
            prev_trace_index = (index + 1) % buffer_size;  
            break;
          }
          if (next_val == start_indicator_val) {  // a new start indicator found so move forward
            if (debug) Serial.println(F("next_val start indicator found go to next"));
            prev_trace_index = (index + 2) % buffer_size;
            break; 
          }
          decoded_value += (bitval << bit_num);
          if (bit_num == 0){
            Serial.print(F("0x"));
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
      Serial.print(F("Found RF code: "));
      Serial.println(rfcode);
  }

  Serial.println();
  Serial.print(F("Remote control button: "));
  Serial.println(ButtonId);
  return ButtonId;
}

/**** Configure the nrf24l01 CE and CSN pins ****/
RF24 radio(CE_PIN, CSN_PIN); // nRF24L01 (CE, CSN)

RF24Network network(radio);
 
unsigned long const keywordvalM = 0xfeebbeef; 
unsigned long const keywordvalS = 0xbeeffeeb; 

// Payload from/for MASTER
struct payload_from_master {
  unsigned long keyword;
  uint32_t counter;
  bool relay1;
  bool relay2;
};
 
// Payload from/for SLAVE
struct payload_from_slave {
  unsigned long keyword;
  uint32_t timing;
  bool relayActive;
  uint8_t nodeId;
};
 
void restart_arduino(){
  Serial.println(F("Restart the Arduino board..."));
  delay(2000);
  //NVIC_SystemReset(); // TBD
}

unsigned long rfcommand = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    // some boards need this because of native USB capability
    delay(10);
  }
  Serial.println(F(" ***** <> *****"));  
  Serial.println(__FILE__);
  Serial.print(F("creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush(); 

  for (int i=0;i<buffer_size;i++){
    trace_array[i] = 0;
  }
  if (!radio.begin()){
    Serial.println(F("Radio hardware error."));
    while (true) delay(1000);
  }
  radio.setPALevel(RF24_PA_MIN, 0);
  // radio.setDataRate(RF24_250KBPS); // (RF24_2MBPS);

  pinMode(rfReceiverPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(rfReceiverPin), logRfTime, RISING);

  Serial.println();  
  Serial.println(F(" ***************"));  
  Serial.println(); 
  Serial.flush(); 
}
 
unsigned long runtiming = 0;

void loop() {

  runtiming = millis();

  // //// Receive a message from master if available - START
  // while (network.available()) {
  //   RF24NetworkHeader header;
  //   payload_from_master payload;
  //   network.read(header, &payload, sizeof(payload));
  //   Serial.print(F("Received packet #"));
  //   Serial.print(payload.counter);
  //   Serial.print(F(", relay1="));
  //   Serial.print(payload.relay1);
  //   Serial.print(F(", relay2="));
  //   Serial.println(payload.relay2);
  //   if (payload.keyword == keywordvalM) {

  //   }
  //   else{
  //     Serial.println(F("Wrong keyword")); 
  //   }

  // }
  // //// Receive a message from master if available - END

  // //// Send to the master node every x seconds - BEGIN
  // if (millis() - sleepTimer > 10000) {
  //   sleepTimer = millis();
  //   payload_from_slave payloadM = {keywordvalS, sleepTimer, relayActive, slaveNodeID};
 
  //   // Send an 'M' type message containing the current millis()
  //   if (!mesh.write(&payloadM, 'M', sizeof(payloadM))) {
  //     // If a write fails, check connectivity to the mesh network
  //     if (!mesh.checkConnection()) {
  //       //refresh the network address
  //       Serial.println(F("Renewing Address"));
  //       if (mesh.renewAddress() == MESH_DEFAULT_ADDRESS) {
  //         // If address renewal fails, reconfigure the radio and restart the mesh
  //         // This allows recovery from most, if not all radio errors
  //         meshstartup();
  //       }
  //     }
  //     else {
  //       Serial.println(F("Send fail, Test OK"));
  //       mesherror++;
  //     }
  //   } else {
  //     Serial.print(F("Send to Master OK: "));
  //     Serial.println(payloadM.timing);
  //     mesherror = 0;
  //   }
  // }
  // //// Send to the master node every x seconds - END

  //// check KiKa code received - BEGIN
  rfcommand = getRfCode();
  if (rfcommand > 0){
    if (prv_rfcommand != rfcommand){ // store rf code only once (TBD per 4 seconds)
      String ButtonCode = buttonfromrfcode(rfcommand);
      sequence_index++;
      sequence_index = sequence_index % 256; // keep it in 1 byte
      Serial.print(F("Sequence: "));
      Serial.print(sequence_index);
      Serial.print(F(", control button: "));
      Serial.println(ButtonCode);
    }
    prv_rfcommand = rfcommand;
  }
  //// check KiKa code received - END

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
