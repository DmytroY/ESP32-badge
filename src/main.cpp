#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
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

unsigned brightness = 50;  // initial brightness 50%

String userTitle = "";
String userSubtitle = "";
String userQR = "";
String currentAction = "";;
bool newMessageReady = false;

uint16_t userTextColor = WHITE;
uint16_t userBgColor = BLACK;

// ── Setup ───────────────────────────────────────────────────────────
void setup() {

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars); // ADC calibration

    LittleFS.begin();

    WiFi.macAddress(mac);
    snprintf(ssid, sizeof(ssid), "ESP-%02X%02X%02X", mac[3], mac[4], mac[5]);
    WiFi.mode(WIFI_AP);
    WiFi.setTxPower(WIFI_POWER_2dBm); // decreased WiFi power
    WiFi.softAP(ssid, PASS, 1);

    serveFile(server, "/", "/index.html", "text/html");
    serveFile(server, "/style.css", "/style.css", "text/css");
    serveFile(server, "/script.js", "/script.js", "application/javascript");

    server.on("/msg", []() {
        userTitle = server.arg("title");
        userSubtitle = server.arg("subtitle");
        userQR = server.arg("qr");
        currentAction = server.arg("action"); // "test" or "finish"

        userTextColor = (uint16_t)strtol(server.arg("textColor").c_str(), NULL, 0);
        userBgColor = (uint16_t)strtol(server.arg("bgColor").c_str(), NULL, 0);
        brightness = server.arg("bright").toInt();

        // Safety Fallback: if colors match, force text to contrast
        if (userTextColor == userBgColor) {
            userTextColor = (userBgColor == 0x0000) ? 0xFFFF : 0x0000;
        }
        
        newMessageReady = true;
        server.send(200, "text/plain", "OK");
    });

    server.begin();

    tft.begin(PIN_SCLK, PIN_MOSI, PIN_CS, PIN_DC, PIN_RST, PIN_BL);
    tft.fillScreen(BLACK);
    tft.drawRect(0, 0, TFT_W, TFT_H, WHITE);
    
    tft.drawText(40,  5, "WELCOME !", GREEN, BLACK, 1);
    tft.drawText(4,   20, "Connect WiFi and visit 192.168.4.1", GREEN, BLACK, 1);

    static char buf[32];
    snprintf(buf, sizeof(buf), "SSID: %s", ssid);
    tft.drawText(5,   35, buf, WHITE, BLACK, 2);
    snprintf(buf, sizeof(buf), "Pass: %s", PASS);
    tft.drawText(5,   57, buf, WHITE, BLACK, 2);

    // QR for WiFi connection
    static char qrBuffer[128];
    snprintf(qrBuffer, sizeof(qrBuffer), "WIFI:S:%s;T:WPA;P:%s;;", ssid, PASS);
    tft.drawQR(qrBuffer);

    tft.setBrightness(brightness);

    analogReadResolution(12);    // set resolution for ADC

    // ------ User data input -----
    while (currentAction != "finish")
    {
        server.handleClient();
        if (newMessageReady) {
            newMessageReady = false;
            tft.setBrightness(brightness);
            tft.fillScreen(userBgColor);

            // Render Preview
            if (userSubtitle.length() == 0){
                tft.drawText(UI::TITLE_X, UI::TITLE_1_Y, userTitle.c_str(), userTextColor, userBgColor, 6);
            } else {
                tft.drawText(UI::TITLE_X, UI::TITLE_2_Y, userTitle.c_str(), userTextColor, userBgColor, 4);
                tft.drawText(UI::TITLE_X, UI::SUBTITLE_Y, userSubtitle.c_str(), userTextColor, userBgColor, 3);
            }

            if (userQR.length() > 0) 
                tft.drawQR(userQR.c_str());
            
            indicateBatteryLevel(PIN_BAT, tft, userBgColor, false);
        }
    }
    // ---- stop server ----
    server.stop();
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();
}

void loop() {
    indicateBatteryLevel(PIN_BAT, tft, userBgColor, true);
    goToSleep(1, PIN_BL);
}