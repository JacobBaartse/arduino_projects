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
String currentUrl = "http://playerservices.streamtheworld.com/api/livestream-redirect/GNR.mp3";
bool shouldChangeUrl = false;
int currentVolume = 100; // Standaard volume bij opstarten (0-100)

// Schone, robuuste HTML-code met een invoerveld voor het volume
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
        input[type='text'], input[type='number'], select { width: 100%; padding: 12px; margin: 10px 0; border: 1px solid #ccc; border-radius: 5px; box-sizing: border-box; font-size: 16px; }
        input[type='submit'] { width: 100%; background-color: #007bff; color: white; padding: 12px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; font-weight: bold; margin-top: 5px; }
        input[type='submit']:hover { background-color: #0056b3; }
        .section { margin: 20px 0; padding: 15px; background: #f8f9fa; border-radius: 8px; border: 1px solid #e2e8f0; text-align: left; }
        .status { background: #e9ecef; padding: 10px; border-radius: 5px; margin-top: 20px; font-size: 14px; word-break: break-all; text-align: center; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>ESP32-S3 Web Radio</h1>

        <!-- Formulier voor zenders -->
        <div class='section'>
            <form action='/set-url' method='GET'>
                <label for='url'><strong>Stream-URL invoeren:</strong></label>
                <input type='text' id='url' name='url' placeholder='https://...'>

                <label for='preset'><strong>Of kies een preset:</strong></label>
                <select id='preset' name='preset' onchange='if(this.value) document.getElementById("url").value = this.value;'>
                    <option value=''>-- Kies een zender --</option>
                <option value='https://stream.wildfm.nl/GOD_Radio'>GOD Radio</option>
                <option value='http://icecast.omroep.nl/radio2-bb-mp3'>NPO Radio 2</option>
                <option value='http://playerservices.streamtheworld.com/api/livestream-redirect/GNR.mp3'>Groot Nieuws Radio</option>
                <option value='https://radio.wanderingsheep.net/listen/golden_praise/goldenpraise'>golden praise</option>
                <option value='http://radio.wanderingsheep.net:33001/onechristianradio'>one christian radio</option>
                </select>
                <input type='submit' value='Zender Wijzigen'>
            </form>
        </div>

        <!-- Formulier met numeriek invoerveld voor het volume -->
        <div class='section'>
            <form action='/set-volume' method='GET'>
                <label for='vol-input'><strong>Volume aanpassen (0 - 100):</strong></label>
                <input type='number' id='vol-input' name='val' min='0' max='100' value='%VOL%'>
                <input type='submit' value='Volume Toepassen'>
            </form>
        </div>

        <div class='status'>
            <strong>Huidig volume:</strong> %VOL%%<br>
            <strong>Nu actief:</strong><br>%URL%
        </div>
    </div>
</body>
</html>
)rawliteral";

void codecCallBack(const char *codec) { Serial.printf("codec: %s\n", codec); }
void bitrateCallback(uint32_t bitrate) { Serial.printf("bitrate: %lu kbps\n", bitrate); }
void stationCallback(const char *name) { Serial.printf("station: %s\n", name); }
void infoCallback(const char *info) { Serial.printf("info: %s\n", info); }
void errorCallback(const char *error) { Serial.printf("error: %s\n", error); }
void eofCallback(const char *url) { Serial.printf("eof: %s\n", url); }

void setup() {
    Serial.begin(9600);
    delay(2000);

    Serial.println("\n\nRadio Streaming met Web Menu\n");

    WiFi.begin(SSID, PSK);
    WiFi.setSleep(false);

    while (!WiFi.isConnected()) delay(10);

    Serial.println("WiFi connected");
    Serial.print("Ga in je browser naar IP-adres: http://");
    Serial.println(WiFi.localIP());

    SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, VS1053_CS);

    if (!audio.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ)) Serial.println("startDecoder failed");
    if (!audio.isChipConnected()) Serial.println("isChipConnected FAILED");

    audio.setCodecCB(codecCallBack);
    audio.setBitrateCB(bitrateCallback);
    audio.setStationCB(stationCallback);
    audio.setInfoCB(infoCallback);
    audio.setErrorCB(errorCallback);
    audio.setEofCB(eofCallback);

    audio.setVolume(currentVolume);
    audio.connectToHost(currentUrl.c_str());

    // --- WEB SERVER ROUTES ---

    // Thuispagina route (handmatige string-vervanging)
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = String(index_html);
        html.replace("%VOL%", String(currentVolume));
        html.replace("%URL%", currentUrl);
        request->send(200, "text/html", html);
    });

    // Route om zender aan te passen
    server.on("/set-url", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("url") && request->getParam("url")->value() != "") {
            currentUrl = request->getParam("url")->value();
            shouldChangeUrl = true;
        }
        request->redirect("/");
    });

    // Route om volume aan te passen (verversen na indiening)
    server.on("/set-volume", HTTP_GET, [](AsyncWebServerRequest *request){
        if (request->hasParam("val")) {
            String volVal = request->getParam("val")->value();
            currentVolume = volVal.toInt();
            // Begrenzen tussen 0 en 100
            if(currentVolume > 100) currentVolume = 100;
            if(currentVolume < 0) currentVolume = 0;

            audio.setVolume(currentVolume);
            Serial.printf("Volume aangepast naar: %d%%\n", currentVolume);
        }
        request->redirect("/"); // Stuur de gebruiker direct terug naar de vernieuwde homepagina
    });

    server.begin();
}

void loop() {
    if (shouldChangeUrl) {
        Serial.print("Omschakelen naar: ");
        Serial.println(currentUrl);
        audio.stopSong();
        audio.connectToHost(currentUrl.c_str());
        shouldChangeUrl = false;
    }
    audio.loop();
    delay(1);
}
