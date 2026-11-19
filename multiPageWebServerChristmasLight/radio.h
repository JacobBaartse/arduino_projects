
bool debug_radio=false;

void command_to_radio(String url){
  WiFiClient client;
  String httpResponseString; 
  char server[] = "192.168.2.8";

  bool time_response_received = false;
  int counter = 0;
  // client.setCACert(root_ca);
    if (client.connect(server, 80)) {
      if (debug_radio) Serial.println("connected to server");
      if (debug_radio) Serial.println(url);
      // send the HTTP request:
      client.println("GET /" + url);
      client.println("Host: 182.186.2.8");
      client.println("Connection: close");
      client.println();
      client.flush();
    };

    // don't wait for the response just disconnect to speed up.
    // if (debug_radio) Serial.println("wait for client available");
    // counter += 1;
    // for (int i = 0 ; i<200; i++){
    //   delay(20);
    //   while (client.available()) {
    //     char c = client.read();
    //     httpResponseString += c;
    //     if (debug_radio) Serial.print(c);

    //   }
    // }
  client.stop();
}


void radio_channel_plus(){
  command_to_radio("ch-up");
}
void radio_channel_min(){
  command_to_radio("ch-down");
}
void radio_volume_plus(){
  command_to_radio("vol-up");
}
void radio_volume_min(){
  command_to_radio("vol-down");
}


const char *HTML_CONTENT_RADIO = R""""(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>Radio</title>
</head>
<body>
    <a href="/radio?0aan">Radio aan</a> &nbsp;&nbsp;&nbsp;  &nbsp;&nbsp;&nbsp;   <a href="/radio?0uit">Radio uit</a>
    </font>
</body>
</html>
)"""";
