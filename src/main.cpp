#include <Arduino.h>
#include <ST7789_76x284.h>

// ── Pin definitions ─────────────────────────────────────────────────
#define PIN_SCLK  2
#define PIN_MOSI  3
#define PIN_CS    5
#define PIN_DC    4
#define PIN_RST   10
#define LED_PIN   8

// ── Display geometry ────────────────────────────────────────────────
// portrait
// #define TFT_W      76
// #define TFT_H      284
// #define X_OFFSET   82
// #define Y_OFFSET   18

// landscape
#define TFT_W      284
#define TFT_H      76
#define X_OFFSET   18
#define Y_OFFSET   82


// ── Setup ───────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    ST7789_76x284 tft(TFT_W, TFT_H, X_OFFSET, Y_OFFSET);
    tft.begin(PIN_SCLK, PIN_MOSI, PIN_CS, PIN_DC, PIN_RST);

    tft.fillScreen(BLACK);
    // tft.drawRect(0, 0, TFT_W, TFT_H, WHITE);

    tft.drawQR("https://github.com/DmytroY/ESP32-badge");

    // char buff[50];
    // sprintf(buff, "QR version: %d", version_QR);
    // tft.drawText(10,  10, buff, WHITE, BLACK, 2);
// !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~
    tft.drawText(5,   5, "Dmytro", WHITE, BLACK, 4);
    tft.drawText(5,  42, "Yakovenko", WHITE, BLACK, 2);

    Serial.println("Done.");
}

void loop() {
    digitalWrite(LED_PIN, HIGH); delay(800);
    digitalWrite(LED_PIN, LOW);  delay(200);
}