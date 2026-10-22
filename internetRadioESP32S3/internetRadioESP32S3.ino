#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <SPI.h>

#include <VS1053.h>
#include <ESP32_VS1053_Stream.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "arduino_secrets.h"

// =====================================
// VS1053 aansluitingen
// =====================================
#define SPI_CLK_PIN  12
#define SPI_MISO_PIN 13
#define SPI_MOSI_PIN 11

#define VS1053_CS    10
#define VS1053_DCS   7
#define VS1053_DREQ  14

// =====================================
// SH1106 OLED en drukknop
// =====================================
#define OLED_SDA_PIN 8
#define OLED_SCL_PIN 9
#define BUTTON_PIN   4

#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(128, 64, &Wire, -1);

// =====================================
// Wifi
// =====================================
const char* SSID = SECRET_SSID;
const char* PSK  = SECRET_PASS;

// =====================================
// Audio en webserver
// =====================================
ESP32_VS1053_Stream audio;
AsyncWebServer server(80);

// =====================================
// Radiozenders
// =====================================
struct RadioStation {
    const char* name;
    const char* url;
};

RadioStation stations[] = {
    {
        "GOD Radio",
        "https://stream.wildfm.nl/GOD_Radio"
    },
    {
        "NPO Radio 2",
        "http://icecast.omroep.nl/radio2-bb-mp3"
    },
    {
        "Groot Nieuws Radio",
        "http://playerservices.streamtheworld.com/api/livestream-redirect/GNR.mp3"
    },
    {
        "Golden Praise",
        "http://radio.wanderingsheep.net/listen/golden_praise/goldenpraise"
    },
    {
        "One Christian Radio",
        "http://radio.wanderingsheep.net:33001/onechristianradio"
    }
};

const int stationCount =
    sizeof(stations) / sizeof(stations[0]);

// Start met Groot Nieuws Radio
int selectedStation = 2;

String currentUrl =
    "http://playerservices.streamtheworld.com/api/livestream-redirect/GNR.mp3";

bool shouldChangeUrl = false;
bool displayAvailable = false;

const unsigned long SCREEN_TIMEOUT = 10000; // 10 seconden
unsigned long lastButtonActivity = 0;
bool screenIsOn = true;

int currentVolume = 100;

// =====================================
// Knop: debounce
// =====================================
bool lastButtonReading = HIGH;
bool stableButtonState = HIGH;

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 40;

// =====================================
// Callbackfuncties VS1053
// =====================================
void codecCallBack(const char* codec) {
    Serial.printf("Codec: %s\n", codec);
}

void bitrateCallback(uint32_t bitrate) {
    Serial.printf("Bitrate: %lu kbps\n",
                  (unsigned long)bitrate);
}

void stationCallback(const char* name) {
    Serial.printf("Station: %s\n", name);
}

void infoCallback(const char* info) {
    Serial.printf("Info: %s\n", info);
}

void errorCallback(const char* error) {
    Serial.printf("Audiofout: %s\n", error);
}

void eofCallback(const char* url) {
    Serial.printf("Stream einde: %s\n", url);
}

// =====================================
// Zoek de preset bij een URL
// =====================================
int findStationByUrl(const String& url) {
    for (int i = 0; i < stationCount; i++) {
        if (url == stations[i].url) {
            return i;
        }
    }

    return -1;
}

// =====================================
// OLED display bijwerken
// =====================================
void updateDisplay() {
    if (!displayAvailable) {
        return;
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("ESP32-S3 RADIO");

    display.drawLine(0, 11, 127, 11, SH110X_WHITE);

    display.setCursor(0, 15);
    display.print("Zender ");

    if (selectedStation >= 0) {
        display.print(selectedStation + 1);
        display.print("/");
        display.println(stationCount);
    } else {
        display.println("Custom");
    }

    display.setCursor(0, 27);

    if (selectedStation >= 0) {
        display.println(stations[selectedStation].name);
    } else {
        display.println("Eigen stream");
    }

    display.drawLine(0, 40, 127, 40, SH110X_WHITE);

    // display.setCursor(0, 44);
    // display.print("Volume: ");
    // display.print(currentVolume);
    // display.println("%");

    display.setCursor(0, 55);
    display.println("Knop: volgende zender");

    display.display();
}

void handleDisplayTimeout() {
    if (screenIsOn &&
        millis() - lastButtonActivity >= SCREEN_TIMEOUT) {

        display.clearDisplay();
        display.display();
        // display.clearDisplay();

        screenIsOn = false;
    }
}

// =====================================
// Volgende zender selecteren
// =====================================
void nextStation() {
    selectedStation++;

    if (selectedStation >= stationCount) {
        selectedStation = 0;
    }

    currentUrl = stations[selectedStation].url;
    shouldChangeUrl = true;

    Serial.print("Geselecteerde zender: ");
    Serial.println(stations[selectedStation].name);

    updateDisplay();
}

// =====================================
// Drukknop uitlezen met debounce
// =====================================
void handleButton() {
    bool reading = digitalRead(BUTTON_PIN);

    if (reading != lastButtonReading) {
        lastDebounceTime = millis();
    }

    if (millis() - lastDebounceTime > debounceDelay) {
        if (reading != stableButtonState) {
            stableButtonState = reading;

            // INPUT_PULLUP: LOW = ingedrukt
            if (stableButtonState == LOW) {
                lastButtonActivity = millis();

                if (!screenIsOn) {
                    screenIsOn = true;
                    updateDisplay();
                } else {
                    nextStation();
                }
            }
        }
    }

    lastButtonReading = reading;
}

// =====================================
// HTML webinterface
// =====================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="nl">
<head>
<meta charset="UTF-8">
<meta name="viewport"
 content="width=device-width, initial-scale=1">

<title>ESP32-S3 Web Radio</title>

<style>
body {
    font-family: Arial, sans-serif;
    text-align: center;
    background: #f4f4f9;
    color: #333;
    padding: 20px;
}

.container {
    max-width: 500px;
    margin: auto;
    background: white;
    padding: 24px;
    border-radius: 12px;
    box-shadow: 0 4px 8px rgba(0,0,0,.1);
}

h1 {
    color: #007bff;
    font-size: 24px;
}

.section {
    margin: 20px 0;
    padding: 15px;
    background: #f8f9fa;
    border: 1px solid #e2e8f0;
    border-radius: 8px;
    text-align: left;
}

input[type=text],
input[type=number],
select {
    width: 100%;
    padding: 12px;
    margin: 10px 0;
    border: 1px solid #ccc;
    border-radius: 5px;
    box-sizing: border-box;
    font-size: 16px;
}

input[type=submit] {
    width: 100%;
    background: #007bff;
    color: white;
    padding: 12px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    font-size: 16px;
    font-weight: bold;
}

.status {
    background: #e9ecef;
    padding: 12px;
    border-radius: 5px;
    margin-top: 20px;
    font-size: 14px;
    overflow-wrap: anywhere;
}
</style>
</head>

<body>
<div class="container">

<h1>ESP32-S3 Web Radio</h1>

<div class="section">
<form action="/set-url" method="GET">

<label for="url">
<strong>Stream-URL invoeren:</strong>
</label>

<input type="text" id="url" name="url"
 placeholder="https://..." >

<label for="preset">
<strong>Of kies een preset:</strong>
</label>

<select id="preset" name="preset"
 onchange="if(this.value)
 document.getElementById('url').value=this.value;">

<option value="">-- Kies een zender --</option>

<option value="https://stream.wildfm.nl/GOD_Radio">
GOD Radio
</option>

<option value="http://icecast.omroep.nl/radio2-bb-mp3">
NPO Radio 2
</option>

<option value="http://playerservices.streamtheworld.com/api/livestream-redirect/GNR.mp3">
Groot Nieuws Radio
</option>

<option value="https://radio.wanderingsheep.net/listen/golden_praise/goldenpraise">
Golden Praise
</option>

<option value="http://radio.wanderingsheep.net:33001/onechristianradio">
One Christian Radio
</option>

</select>

<input type="submit" value="Zender wijzigen">
</form>
</div>

<div class="section">
<form action="/set-volume" method="GET">

<label for="vol-input">
<strong>Volume (0 - 100):</strong>
</label>

<input type="number" id="vol-input"
 name="val" min="0" max="100" value="%VOL%">

<input type="submit" value="Volume toepassen">
</form>
</div>

<div class="status">
<strong>Huidig volume:</strong> %VOL%%<br><br>
<strong>Actieve stream:</strong><br>
%URL%
</div>

</div>
</body>
</html>
)rawliteral";

// =====================================
// Setup
// =====================================
void setup() {
    Serial.begin(9600);
    delay(1000);

    Serial.println();
    Serial.println("ESP32-S3 Web Radio starten");

    // Knop
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // OLED
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

    displayAvailable = display.begin(OLED_ADDRESS, true);

    if (displayAvailable) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(0, 0);
        display.println("Radio starten...");
        display.display();
    } else {
        Serial.println("SH1106 niet gevonden");
    }

    // Wifi
    WiFi.setSleep(false);
    WiFi.begin(SSID, PSK);

    Serial.print("Verbinden met wifi");

    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }
    WiFi.setAutoReconnect(true);

    Serial.println();
    Serial.println("Wifi verbonden");

    Serial.print("Webinterface: http://");
    Serial.println(WiFi.localIP());

    // SPI en VS1053
    SPI.begin(
        SPI_CLK_PIN,
        SPI_MISO_PIN,
        SPI_MOSI_PIN,
        VS1053_CS
    );

    if (!audio.startDecoder(
            VS1053_CS,
            VS1053_DCS,
            VS1053_DREQ)) {
        Serial.println("startDecoder mislukt");
    }

    if (!audio.isChipConnected()) {
        Serial.println("VS1053 niet gevonden");
    }

    audio.setCodecCB(codecCallBack);
    audio.setBitrateCB(bitrateCallback);
    audio.setStationCB(stationCallback);
    audio.setInfoCB(infoCallback);
    audio.setErrorCB(errorCallback);
    audio.setEofCB(eofCallback);

    audio.setVolume(currentVolume);
    audio.connectToHost(currentUrl.c_str());

    updateDisplay();
    lastButtonActivity = millis();

    // =================================
    // Webserver: hoofdpagina
    // =================================
    server.on("/", HTTP_GET,
        [](AsyncWebServerRequest* request) {
            String html = FPSTR(index_html);

            html.replace("%VOL%", String(currentVolume));
            html.replace("%URL%", currentUrl);

            request->send(200, "text/html", html);
        }
    );

    // =================================
    // Webserver: URL wijzigen
    // =================================
    server.on("/set-url", HTTP_GET,
        [](AsyncWebServerRequest* request) {

            String newUrl = "";

            if (request->hasParam("url")) {
                newUrl = request->getParam("url")->value();
            }

            // Ondersteun ook rechtstreekse presetkeuze
            if (newUrl.length() == 0 &&
                request->hasParam("preset")) {
                newUrl =
                    request->getParam("preset")->value();
            }

            if (newUrl.length() > 0 &&
                (newUrl.startsWith("http://") ||
                 newUrl.startsWith("https://"))) {

                currentUrl = newUrl;
                selectedStation = findStationByUrl(currentUrl);
                shouldChangeUrl = true;

                updateDisplay();
            }

            request->redirect("/");
        }
    );

    // =================================
    // Webserver: volume wijzigen
    // =================================
    server.on("/set-volume", HTTP_GET,
        [](AsyncWebServerRequest* request) {

            if (request->hasParam("val")) {
                String volVal =
                    request->getParam("val")->value();

                currentVolume = volVal.toInt();

                if (currentVolume > 100) {
                    currentVolume = 100;
                }

                if (currentVolume < 0) {
                    currentVolume = 0;
                }

                audio.setVolume(currentVolume);

                Serial.printf(
                    "Volume aangepast: %d%%\n",
                    currentVolume
                );

                updateDisplay();
            }

            request->redirect("/");
        }
    );

    server.begin();

    Serial.println("Webserver gestart");
}

// =====================================
// Loop
// =====================================
void loop() {
    handleButton();
    handleDisplayTimeout();

    if (shouldChangeUrl) {
        shouldChangeUrl = false;

        Serial.print("Omschakelen naar: ");
        Serial.println(currentUrl);

        audio.stopSong();
        audio.connectToHost(currentUrl.c_str());

        updateDisplay();
    }

    audio.loop();
}