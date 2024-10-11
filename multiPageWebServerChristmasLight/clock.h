unsigned long startup_unix_time_internet;
unsigned long startup_unix_time_rtc;
RTCTime currentTime;

void set_clock(unsigned long unix_time){
  startup_unix_time_rtc = unix_time;
  currentTime.setUnixTime(unix_time);
  RTC.setTime(currentTime);
}


void get_time_form_worldtimeapi_org(){
  WiFiClient client;
  String readString; 
  char server[] = "worldtimeapi.org"; 

  if (client.connect(server, 80)) {
    // Serial.println("connected to server");
    // send the HTTP request:
    client.println("GET /api/timezone/Europe/Amsterdam HTTP/1.1");
    client.println("Host: worldtimeapi.org");
    client.println("Connection: close");
    client.println();
  };
  int counter = 0;
  while (!client.available()){
    counter += 1;
    delay(1000);
  }

  while (client.available()) {
    char c = client.read();
    readString += c;
  }

  client.stop();

  // parse the readString to get the utc time and timezone.
  int unix_time_start = readString.indexOf("\"unixtime\":") + 11;
  String date_time = readString.substring(unix_time_start, unix_time_start+12);
  int date_time_start = readString.indexOf("\"datetime\":\"") + 13;
  date_time += readString.substring(date_time_start, date_time_start+26);
  int unix_time_len = date_time.indexOf(",");

  int utc_offset_start = readString.indexOf("\"utc_offset\":\"") + 14;
  String utc_offset_string = readString.substring(utc_offset_start, utc_offset_start + 6);
  int timezone_minutes = utc_offset_string.substring(1,3).toInt()*60 + utc_offset_string.substring(4,6).toInt();

  if (utc_offset_string.substring(0,1) == "-") timezone_minutes = 0-timezone_minutes;

  startup_unix_time_internet = date_time.substring(0, unix_time_len).toInt();

  startup_unix_time_internet += timezone_minutes * 60;

  // correct startup value for clock for internet lag..
  startup_unix_time_internet += 1;
  set_clock(startup_unix_time_internet);   
}


int prev_second;
const int hour = 60*60;

int Hour = 0;
int Minutes = 0;
int Seconds = -1;


void(* restart_uno) (void) = 0; //declare reset function @ address 0

void update_clock(){
  RTC.getTime(currentTime);
  if (currentTime.getSeconds() != prev_second){
    //Serial.println("clock loop once");
    prev_second = currentTime.getSeconds();
    unsigned long unix_time = currentTime.getUnixTime();
    unsigned long elapsed_seconds = unix_time - startup_unix_time_rtc;
    elapsed_seconds -= elapsed_seconds/53;
    // elapsed_seconds += elapsed_seconds/3400;  // additional correction if required.

    unsigned long corrected_unix_time = startup_unix_time_rtc + elapsed_seconds;
    Minutes = (corrected_unix_time / 60) % 60;
    Hour = (corrected_unix_time / hour) % 24;
    Seconds = corrected_unix_time % 60;

    if (currentTime.getHour()== 4 && (millis()/1000/hour)>6) restart_uno();  //restart every night at 4 a clock once.
  }
}

