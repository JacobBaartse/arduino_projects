#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <VS1053.h>               // https://github.com/baldram/ESP_VS1053_Library
#include <ESP32_VS1053_Stream.h>



// Component	Pin Functie	ESP32-S3 GPIO	Status / Opmerkingen
// SH1106 Scherm	SDA / SCL	GPIO 8 & GPIO 9	Gereserveerd voor I2C (reeds in gebruik)
// VS1053 MP3	SCK (Clock)	GPIO 12	Standaard Hardware SPI Clock
// VS1053 MP3	MOSI	GPIO 11	Standaard Hardware SPI MOSI
// VS1053 MP3	MISO	GPIO 13	Standaard Hardware SPI MISO
// VS1053 MP3	XCS (Control CS)	GPIO 10	Standaard Hardware SPI CS
// VS1053 MP3	XDCS (Data CS)	GPIO 7	Vrije, veilige pin voor Data Chip Select
// VS1053 MP3	DREQ (Interrupt)	GPIO 14	Data Request Interrupt Pin
// VS1053 MP3	XRST (Reset)	GPIO 47	Optionele Reset Pin



// #define SPI_CLK_PIN 18
// #define SPI_MISO_PIN 19
// #define SPI_MOSI_PIN 23

// #define VS1053_CS 5
// #define VS1053_DCS 21
// #define VS1053_DREQ 22

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
    if (audio.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ))
      Serial.println("Decoder not running1");
    else
      Serial.println("Decoder not running2");

    if (audio.isChipConnected())
      Serial.println("Decoder not running3");
    else
      Serial.println("Decoder not running4");

    // if (!audio.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ) || !)
    //     Serial.println("Decoder not running");


    Serial.println("Starting radio stream2");
    // Set the codec callback
    audio.setCodecCB(codecCallBack);
    Serial.println("Starting radio stream3");
    // Set the bitrate callback
    audio.setBitrateCB(bitrateCallback);   
    Serial.println("Starting radio stream4");
    // Set the station name callback
    audio.setStationCB(stationCallback);
    Serial.println("Starting radio stream5");
    // Set the stream metadata callback
    audio.setInfoCB(infoCallback);
    Serial.println("Starting radio stream6");
    // Set the error callback
    audio.setErrorCB(errorCallback); 
    Serial.println("Starting radio stream7");
    // Set the EOF callback
    audio.setEofCB(eofCallback);    

    Serial.println("Starting radio stream8");

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
