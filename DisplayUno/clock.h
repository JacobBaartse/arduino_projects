/*
 * get actual time information from https://www.tdic.nl/localtime.php
 * or from https://www.spoorstra.nl/bcso/localtime.php
 *
 */

const PROGMEM char* root_ca = 
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----\n";

unsigned long startup_unix_time_internet;
unsigned long startup_unix_time_rtc;
RTCTime currentTime;

void set_clock(unsigned long unix_time){
  startup_unix_time_rtc = unix_time;
  currentTime.setUnixTime(unix_time);
  RTC.setTime(currentTime);
}

void restart_uno(){
  Serial.println("Restart the UNO R4 WiFi...");
  delay(2000);
  NVIC_SystemReset();
}

void get_time_from_hsdesign(){
  char c;
  RTC.begin();
  WiFiSSLClient client;
  String httpResponseString; 
  char server[] = "www.tdic.nl";

  bool time_response_received = false;
  int counter = 0;
  client.setCACert(root_ca);

  while (!time_response_received){
    if (client.connect(server, 443)) {
      // send the HTTP request:
      client.println("GET /localtime.php HTTP/1.1");
      client.println("Host: www.tdic.nl");
      client.println("Connection: close");
      client.println();
      client.flush();
    };

    counter += 1;
    for (int i=0; i<200; i++){
      delay(20);
      while (client.available()) {
        c = client.read();
        httpResponseString += c;
        time_response_received = true;
      }
      if (time_response_received) break;
      if (i == 199) counter += 1;
    }
    if (counter > 60) restart_uno();
  }
  client.stop();

  // parse the httpResponseString to get the utc time and timeoffset.
  int unix_time_start = httpResponseString.indexOf("\r\n\r\n") + 7; // 3 bytes after the headers.
  String date_time = httpResponseString.substring(unix_time_start, unix_time_start+15);
  int unix_time_len = date_time.indexOf(";");
  int utc_offset_start = date_time.indexOf(";") + 1;
  String utc_offset_string = date_time.substring(utc_offset_start, utc_offset_start + 4); // todo negative timezones
  int timezone_seconds = utc_offset_string.toInt();
  // if (utc_offset_string.substring(0,1) == "-") timezone_minutes = 0-timezone_minutes; // todo negative timezones

  startup_unix_time_internet = date_time.substring(0, unix_time_len).toInt();
  startup_unix_time_internet += timezone_seconds;

  // correct startup value for clock for internet lag..
  startup_unix_time_internet += 1;
  set_clock(startup_unix_time_internet);   
  Serial.println("Time retrieved from the internet and set locally.");
}

bool update_time(int toggle_time){
  static unsigned long timetime = 0;
  unsigned long timing = millis();
  if(timing < timetime) return false;
  timetime = timing + toggle_time;
  return true;
}

int toggle_data(int choises, int toggle_time){
  static int datatracking = 0;
  if (update_time(toggle_time)){
    if (++datatracking > choises){ 
      datatracking = 0;
    }
  }
  return datatracking;
}

// int prev_second;
// const int hour = 60*60;

// int Hour = 0;
// int Minutes = 0;
// int Seconds = -1;

// void update_clock(){
//   RTC.getTime(currentTime);
//   if (currentTime.getSeconds() != prev_second){
//     prev_second = currentTime.getSeconds();
//     unsigned long unix_time = currentTime.getUnixTime();
//     unsigned long elapsed_seconds = unix_time - startup_unix_time_rtc;
//     elapsed_seconds -= elapsed_seconds/53;
//     // elapsed_seconds += elapsed_seconds/3400;  // additional correction if required.

//     unsigned long corrected_unix_time = startup_unix_time_rtc + elapsed_seconds;
//     Minutes = (corrected_unix_time / 60) % 60;
//     Hour = (corrected_unix_time / hour) % 24;
//     Seconds = corrected_unix_time % 60;

//     if (currentTime.getHour()== 4 && (millis()/1000/hour)>2) restart_uno();  //restart every night at 4 a clock once.
//   }
// }
