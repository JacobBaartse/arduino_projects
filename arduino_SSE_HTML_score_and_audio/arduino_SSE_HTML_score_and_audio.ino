#include "WiFiS3.h"
#include "arduino_secrets.h"
#include "mp3names.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)


WiFiServer server(80);
const int BUTTON_PIN = 2;     // Fysieke knop op de Arduino
int lastButtonState = HIGH;   // Voorgaande status van de knop
int status = WL_IDLE_STATUS;

int mp3_number = 0;

// We houden bij of er een browser actief luistert naar events
WiFiClient eventClient;

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.print("open Chrome of Firefox en ga naar ");
  Serial.println(ip);
}

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Interne pull-up weerstand inschakelen

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
}

void loop() {
  // 1. Controleer op nieuwe browser verbindingen
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    // Als de browser vraagt om de 'event stream' (de luisterlijn)
    if (req.indexOf("/events") != -1) {
      eventClient = client;
      eventClient.println("HTTP/1.1 200 OK");
      eventClient.println("Content-Type: text/event-stream");
      eventClient.println("Cache-Control: no-cache");
      eventClient.println("Connection: keep-alive");
      eventClient.println();
      Serial.println("events requested");
    }
    // Als de browser de gewone webpagina opvraagt
    else {
      Serial.println("fresh web page");
      sendWebPage(client);
    }
  }

  // 2. DYNAMISCHE TRIGGER: Controleer de fysieke knop
  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW && lastButtonState == HIGH) {
    delay(50); // Simpele debouncing tegen ruis

    // Als de browser verbonden is, sturen we het 'alarm' commando
    if (eventClient && eventClient.connected()) {
      eventClient.print("data: speel_geluid_");
      eventClient.print(mp3_number);
      eventClient.println("\n");
      Serial.println("Dynamisch geluidssignaal verzonden naar browser!");
      mp3_number += 1;
      if (mp3_number == mp3_array_size)
      {
        mp3_number = 0;
      }
    }

        // Als de browser verbonden is, sturen we het 'alarm' commando
    if (eventClient && eventClient.connected()) {
      delay(3000);
      eventClient.println("data: stop_geluid\n");
      Serial.println("Dynamisch geluidssignaal stoppen!");
    }
  }
  lastButtonState = buttonState;
}

// Functie om de HTML & JavaScript naar de browser te sturen
void sendWebPage(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();

  client.println("<!DOCTYPE html><html><head><title>Dynamische Audio</title>");
  client.println("<script>");

  // JavaScript opent de luisterlijn naar de Arduino
  client.println("var source = new EventSource('/events');");
  client.println("var audio = new Audio('https://nonexist.mp3');");
  client.println("source.onmessage = function(event) {");
//  client.println("x.innerHTML += event.data + \"<br>\";");
  // client.println("  if (event.data === 'speel_geluid') {");
  // // Link naar een online geluidsfragment (bijv. een korte bliep of alarm)
  // client.print("    audio = new Audio('https://familiebaartse.pythonanywhere.com/static//");
  // client.print(mp3_list[mp3_number].name);
  // client.println(".mp3');");
  // client.println("    audio.play();");
  // client.println("  }");

  for (int i=0; i< mp3_array_size; i++ ){
    client.print("  if (event.data === 'speel_geluid_");
    client.print(i);
    client.println("') {");
    client.print("    audio = new Audio('https://familiebaartse.pythonanywhere.com/static//");
    client.print(mp3_list[i].name);
    client.println(".mp3');");    
    client.println("    audio.play();");
    client.println("  }");  
  }


  client.println("  if (event.data === 'stop_geluid') {");
  // Link naar een online geluidsfragment (bijv. een korte bliep of alarm)
  client.println("    audio.pause();");
  client.println("  }");
  client.println("};");

  client.println("</script></head><body>");
  client.println("<h1>Wachten op signaal van de Arduino...</h1>");
  client.println("<p>Klik op deze pagina om auto play mogelijk te maken.</p>");
  client.println("<p>Druk op de fysieke knop op de Arduino om een geluid te horen. (pin 2 to ground)</p>");
  client.println("</body></html>");
  client.println();
  client.stop();
}
