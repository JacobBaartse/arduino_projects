#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <VS1053.h>               // https://github.com/baldram/ESP_VS1053_Library
#include <ESP32_VS1053_Stream.h>
#include <ESPAsyncWebServer.h>    // Voeg deze bibliotheek toe (en AsyncTCP.h)
#include "arduino_secrets.h"

#define SPI_CLK_PIN 12
#define SPI_MISO_PIN 13
#define SPI_MOSI_PIN 11

#define VS1053_CS 10
#define VS1053_DCS 7
#define VS1053_DREQ 14

ESP32_VS1053_Stream audio;
AsyncWebServer server(80);

const char* SSID = SECRET_SSID;
const char* PSK = SECRET_PASS;

// Variabele om de huidige stream-URL bij te houden
String currentUrl = "https://stream.wildfm.nl/GOD_Radio"; //http://radio.wanderingsheep.net:33001/onechristianradio    https://radio.wanderingsheep.net/listen/golden_praise/goldenpraise
bool shouldChangeUrl = false;

// HTML en CSS code voor de webpagina
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>ESP32-S3 Radio Menu</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; color: #333; padding: 20px; }
        .container { max-width: 500px; margin: auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); }
        h1 { color: #007bff; font-size: 24px; }
        input[type='text'], select { width: 100%; padding: 12px; margin: 15px 0; border: 1px solid #ccc; border-radius: 5px; box-sizing: border-radius; font-size: 16px; }
        input[type='submit'] { width: 100%; background-color: #007bff; color: white; padding: 12px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; font-weight: bold; }
        input[type='submit']:hover { background-color: #0056b3; }
        .status { background: #e9ecef; padding: 10px; border-radius: 5px; margin-top: 20px; font-size: 14px; word-break: break-all; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>ESP32-S3 Web Radio</h1>

        <!-- Formulier voor handmatige URL -->
        <form action='/set-url' method='GET'>
            <label for='url'>Voer handmatig een Stream-URL in:</label>
            <input type='text' id='url' name='url' placeholder='https://...'>
                        </select>
            </br></br>
            
            <label for='preset'>Of kies een preset:</label>
            <select id='preset' name='preset' onchange='if(this.value) document.getElementById("url").value = this.value;'>
                <option value=''>-- Kies een zender --</option>
                <option value='https://stream.wildfm.nl/GOD_Radio'>GOD Radio</option>
                <option value='http://icecast.omroep.nl/radio2-bb-mp3'>NPO Radio 2</option>
                <option value='http://playerservices.streamtheworld.com/api/livestream-redirect/GNR.mp3'>Groot Nieuws Radio</option>
            </select>
            </br></br>

            <input type='submit' value='Zender Wijzigen'>
        </form>

        <div class='status'>
            <strong>Nu actief:</strong><br>%CURRENT_URL%
        </div>
    </div>
</body>
</html>
)rawliteral";

// Vervangt de %CURRENT_URL% placeholder in de HTML met de echte URL
String processor(const String& var) {
    if (var == "CURRENT_URL") return currentUrl;
    return String();
}

// Oude callbacks behouden
void codecCallBack(const char *codec) { Serial.printf("codec: %s\n", codec); }
void bitrateCallback(uint32_t bitrate) { Serial.printf("bitrate: %lu kbps\n", bitrate); }
void stationCallback(const char *name) { Serial.printf("station: %s\n", name); }
void infoCallback(const char *info) { Serial.printf("info: %s\n", info); }
void errorCallback(const char *error) { Serial.printf("error: %s\n", error); }
void eofCallback(const char *url) { Serial.printf("eof: %s\n", url); }

void setup() {
    Serial.begin(115200); // Verhoogd naar 115200 voor vlottere logs icm netwerk
    delay(2000);
    Serial.println("\n\nVS1053 Radio Streaming met Web Menu\n");

    // Connect to Wi-Fi
    WiFi.begin(SSID, PSK);
    WiFi.setSleep(false);

    while (!WiFi.isConnected()) delay(10);

    Serial.println("WiFi connected");
    Serial.print("Ga in je browser naar IP-adres: http://");
    Serial.println(WiFi.localIP()); // DIT IS DE URL VOOR JOUW MENU!

    // Start SPI bus
    SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, VS1053_CS);

    if (!audio.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ)) Serial.println("startDecoder failed");
    if (!audio.isChipConnected()) Serial.println("isChipConnected FAILED");

    audio.setCodecCB(codecCallBack);
    audio.setBitrateCB(bitrateCallback);
    audio.setStationCB(stationCallback);
    audio.setInfoCB(infoCallback);
    audio.setErrorCB(errorCallback);
    audio.setEofCB(eofCallback);

    // Start eerste stream
    audio.connectToHost(currentUrl.c_str());

    // --- WEB SERVER ROUTES CONFIGUREREN ---

    // Serveer de homepagina
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html, processor);
    });

    // Verwerk het formulier wanneer er op de knop wordt gedrukt
    server.on("/set-url", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("url") && request->getParam("url")->value() != "") {
            currentUrl = request->getParam("url")->value();
            shouldChangeUrl = true; // Zet een vlag om in de loop te verwerken
        }
        request->redirect("/"); // Stuur de gebruiker direct terug naar de homepagina
    });

    server.begin(); // Start de webserver
}

void loop() {
    // Als de webserver een nieuwe URL heeft opgevangen, switch dan veilig van zender
    if (shouldChangeUrl) {
        Serial.print("Omschakelen naar: ");
        Serial.println(currentUrl);

        audio.stopSong(); // Stop de huidige stream netjes
        audio.connectToHost(currentUrl.c_str()); // Start de nieuwe stream

        shouldChangeUrl = false;
    }

    audio.loop();
    delay(1); // Iets kortere delay voor optimale streaming stabiliteit
}
