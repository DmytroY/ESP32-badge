#include <Arduino.h>
#include "ST7789_76x284.h"

// ── Pin definitions ─────────────────────────────────────────────────
#define PIN_SCLK  2
#define PIN_MOSI  3
#define PIN_CS    5
#define PIN_DC    4
#define PIN_RST   10
#define LED_PIN   8

// ── Display geometry ────────────────────────────────────────────────
// portrait
#define TFT_W      76
#define TFT_H      284
#define X_OFFSET   82
#define Y_OFFSET   18

// landscape
// #define TFT_W      284
// #define TFT_H      76
// #define X_OFFSET   18
// #define Y_OFFSET   82


// ── Setup ───────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    ST7789_76x284 tft(TFT_W, TFT_H, X_OFFSET, Y_OFFSET);
    tft.begin(PIN_SCLK, PIN_MOSI, PIN_CS, PIN_DC, PIN_RST);

    // Color test
    tft.fillScreen(RED);   delay(800);
    tft.fillScreen(GREEN); delay(800);
    tft.fillScreen(BLUE);  delay(800);
    tft.fillScreen(BLACK);

    // Graphics test
    tft.drawRect(0, 0, TFT_W, TFT_H, WHITE);
    tft.drawRect(2, 2, TFT_W - 4, TFT_H - 4, CYAN);

    if(TFT_W > TFT_H){
        tft.drawText(10,  10, "RED", RED, BLACK, 2);
        tft.drawText(100,  10, "GREEN", GREEN, BLACK, 2);
        tft.drawText(190,  10, "BLUE", BLUE, BLACK, 2);

        tft.drawText(10,  50, "CYAN", CYAN, BLACK, 2);
        tft.drawText(100,  50, "MAGENTA", MAGENTA, BLACK, 2);
        tft.drawText(190,  50, "YELLOW", YELLOW, BLACK, 2);
    } else {
        tft.drawText(5,  10, "RED", RED, BLACK, 1);
        tft.drawText(5,  50, "GREEN", GREEN, BLACK, 1);
        tft.drawText(5,  100, "BLUE", BLUE, BLACK, 1);

        tft.drawText(5,  150, "CYAN", CYAN, BLACK, 1);
        tft.drawText(5,  200, "MAGENTA", MAGENTA, BLACK, 1);
        tft.drawText(5,  250, "YELLOW", YELLOW, BLACK, 1);
    }

    Serial.println("Done.");
}

void loop() {
    digitalWrite(LED_PIN, HIGH); delay(800);
    digitalWrite(LED_PIN, LOW);  delay(200);
}