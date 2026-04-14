#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <WebServer.h>
#include <LittleFS.h>
#include <ST7789_76x284.h>
#include "helper.h"

// --- WiFi settings ------
uint8_t mac[6];
char ssid[20]; // wil be determined based on WiFi MAC to be unique
#define PASS "12345678"

// ── Pin definitions ─────────────────────────────────────────────────
#define PIN_SCLK  7
#define PIN_MOSI  8
#define PIN_CS    20
#define PIN_DC    10
#define PIN_RST   9
#define PIN_BL    21
#define PIN_BAT   0

// ── Display geometry ────────────────────────────────────────────────
// landscape
#define TFT_W      284
#define TFT_H      76
#define X_OFFSET   18
#define Y_OFFSET   82

ST7789_76x284 tft(TFT_W, TFT_H, X_OFFSET, Y_OFFSET);
WebServer server(80);

float bat_v;
int bat_percent = 0;
char bat_perc_buffer[5]; // 3-digit number + % + null terminator

unsigned long lastUpdate = 0; // for brightness blinking
unsigned brightness = 80;  // initial brightness 80%
String userMessage = "";
bool newMessageReady = false;

// ── Setup ───────────────────────────────────────────────────────────
void setup() {
    LittleFS.begin();

    WiFi.macAddress(mac);
    snprintf(ssid, sizeof(ssid), "ESP-%02X%02X%02X", mac[3], mac[4], mac[5]);
    WiFi.mode(WIFI_AP);
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // decreased WiFi power
    WiFi.softAP(ssid, PASS, 1);

    server.on("/", []() {
        File file = LittleFS.open("/index.html", "r");
        server.streamFile(file, "text/html");
        file.close();
    });

    server.on("/style.css", []() {
        File file = LittleFS.open("/style.css", "r");
        server.streamFile(file, "text/css");
        file.close();
    });

    server.on("/script.js", []() {
        File file = LittleFS.open("/script.js", "r");
        server.streamFile(file, "application/javascript");
        file.close();
    });

    server.on("/msg", []() {
        if (server.hasArg("text")) {
            userMessage = server.arg("text");
            newMessageReady = true;
            server.send(200, "text/plain", "Queued");
        }
    });

    server.begin();

    tft.begin(PIN_SCLK, PIN_MOSI, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
    tft.fillScreen(BLACK);
    tft.drawRect(0, 0, TFT_W, TFT_H, WHITE);
    
    tft.drawText(20,   5, "WELCOME !", GREEN, BLACK, 1);
    tft.drawText(5,   15, "Connect and visit 192.168.4.1", GREEN, BLACK, 1);

    static char buf[32];
    snprintf(buf, sizeof(buf), "ssid: %s", ssid);
    tft.drawText(5,   35, buf, WHITE, BLACK, 2);
    snprintf(buf, sizeof(buf), "Pass: %s", PASS);
    tft.drawText(5,   57, buf, WHITE, BLACK, 2);

    // QR fot WiFi connection
    static char qrBuffer[128];
    snprintf(qrBuffer, sizeof(qrBuffer), "WIFI:S:%s;T:WPA;P:%s;;", ssid, PASS);
    tft.drawQR(qrBuffer);

    tft.setBrightness(brightness);

    // ------ User data input -----
    while (userMessage != "done")
    {
        server.handleClient();
        if (newMessageReady && userMessage != "done") {
            newMessageReady = false;
            tft.fillScreen(BLACK);
            tft.drawText(5, 25, userMessage.c_str(), WHITE, BLACK, 2);
        }
    }
    // ---- stop server ----
    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();

    // ADC for battery level measurement
    analogReadResolution(12);
}

void loop() {
    // indicate battery level
    // bat_v = 2 * 2.5 * analogRead(PIN_BAT) / 4095;
    // bat_percent = static_cast<int>(round(100 * (bat_v - 3.2)));

    bat_percent-- ;
    if(bat_percent <= 1) bat_percent = 105;

    snprintf(bat_perc_buffer, sizeof(bat_perc_buffer), "%3d%%", bat_percent);
    tft.drawText(13, 65, bat_perc_buffer, WHITE, BLACK, 1);

    if(bat_percent <= 20){
        tft.drawChar(5, 65, (char)0x80, RED, BLACK, 1);
    } else if(bat_percent <= 40){
        tft.drawChar(5, 65, (char)0x81, YELLOW, BLACK, 1);
    } else if(bat_percent <= 60){
        tft.drawChar(5, 65, (char)0x82, WHITE, BLACK, 1);
    }else if(bat_percent <= 80){
        tft.drawChar(5, 65, (char)0x83, GREEN, BLACK, 1);
    }else {
        tft.drawChar(5, 65, (char)0x84, GREEN, BLACK, 1);
    }

    tft.drawText(45, 65, "REMAINING BATTERY, 00 min", WHITE, BLACK, 1);
    
    goToSleep(1, PIN_BL);
}