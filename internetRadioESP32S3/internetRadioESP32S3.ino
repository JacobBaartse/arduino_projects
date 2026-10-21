#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <VS1053.h>               // https://github.com/baldram/ESP_VS1053_Library
#include <ESP32_VS1053_Stream.h>
#include "arduino_secrets.h"


#define SPI_CLK_PIN 12
#define SPI_MISO_PIN 13
#define SPI_MOSI_PIN 11

#define VS1053_CS 10
#define VS1053_DCS 7
#define VS1053_DREQ 14

ESP32_VS1053_Stream audio;

const char* SSID = SECRET_SSID;
const char* PSK = SECRET_PASS;

// Called when codec is detected
void codecCallBack(const char *codec)
{
    Serial.printf("codec: %s\n", codec);
}

// Called when bitrate is detected (cbr) and changes (vbr)
void bitrateCallback(uint32_t bitrate)
{
    Serial.printf("bitrate: %lu kbps\n", bitrate);
}

// Called when a stream has an ICY name header set
void stationCallback(const char *name)
{
    Serial.printf("station: %s\n", name);
}

// Called when stream metadata is available
void infoCallback(const char *info)
{
    Serial.printf("info: %s\n", info);
}

// Called on stream errors
void errorCallback(const char *error)
{
    Serial.printf("error: %s\n", error);
}

// Called on end-of-file
void eofCallback(const char *url)
{
    Serial.printf("eof: %s\n", url);
}

void setup() {
    Serial.begin(9600);
    delay(2000);
    Serial.println("\n\nVS1053 Radio Streaming Example\n");

    // Connect to Wi-Fi
    Serial.printf("Connecting to WiFi network: %s\n", SSID);
    WiFi.begin(SSID, PSK);  
    WiFi.setSleep(false);  // Important to disable sleep to ensure stable connection

    while (!WiFi.isConnected())
        delay(10);

    Serial.println("WiFi connected");

    // Start SPI bus
    SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, VS1053_CS);

    Serial.println("Starting radio stream1");
    // Initialize the VS1053 decoder
    if (!audio.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ))
      Serial.println("startDecoder failed");

    if (!audio.isChipConnected())
      Serial.println("isChipConnected FAILED");

    // Set the codec callback
    audio.setCodecCB(codecCallBack);
    // Set the bitrate callback
    audio.setBitrateCB(bitrateCallback);   
    // Set the station name callback
    audio.setStationCB(stationCallback);
    // Set the stream metadata callback
    audio.setInfoCB(infoCallback);
    // Set the error callback
    audio.setErrorCB(errorCallback); 
    // Set the EOF callback
    audio.setEofCB(eofCallback);    


    // Connect to the radio stream
    // audio.connectToHost("http://icecast.omroep.nl/radio2-bb-mp3");
    audio.connectToHost("https://stream.wildfm.nl/GOD_Radio");

    if (!audio.isRunning())
       Serial.println("No audio running");
}

void loop() {
    audio.loop();
    delay(5);
}
