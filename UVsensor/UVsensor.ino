  /*
  GUVA-S12SD-UV-Module
  Made on 03 Feb 2020
  by Amir Mohammad Shojaee @ Electropeak
  Home

*/

// void setup() 
// {
//   Serial.begin(9600);
// }
 
// void loop() 
// {
//   float sensorVoltage; 
//   float sensorValue;
 
//   sensorValue = analogRead(A0);
//   sensorVoltage = sensorValue/1024*5.0;
//   Serial.print("sensor reading = ");
//   Serial.print(sensorValue);
//   Serial.print("        sensor voltage = ");
//   Serial.print(sensorVoltage);
//   Serial.println(" V");
//   delay(1000);
// }


// --------------------

void uv_index()
{
  float sensorVoltage; 
  float sensorValue;
  int UV_index;
  String quality = ""; 
  sensorValue = analogRead(A0);
  sensorVoltage = sensorValue/1024*5.0;
  UV_index= sensorVoltage/0.1;

  //condition for UV state
  if(UV_index<=2){ 
    quality = " LOW ";
  }
  else if(UV_index > 2 && UV_index <=5){
   quality = " MODERATE ";
  }
  else if(UV_index>5 && UV_index<=7){
   quality = " HIGH ";
  }
  else if(UV_index>7 && UV_index<=10){
   quality = "VERY HIGH ";
  }
  else{ // > 10
   quality = " EXTREME ";
  }

  // result printing
  Serial.print(F("UV index: "));
  Serial.print(UV_index);
  Serial.print(F(', '));
  Serial.println(quality);
}

void setup() {
  Serial.begin(11500);
  sleep(1);
  Serial.print(__FILE__);
  Serial.print(F("\n, creation/build time: "));
  Serial.println(__TIMESTAMP__);
  Serial.flush();  
}

void loop() {

  uv_index();

  delay(10000);
}
