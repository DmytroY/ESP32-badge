#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ST7789_76x284.h>
#include "esp_pm.h"
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/gpio.h"

// --- WiFi settings ------
#define SSID "DY-BADGE"
#define PASS "12345678"

// ── Pin definitions ─────────────────────────────────────────────────
#define PIN_SCLK  7
#define PIN_MOSI  8
#define PIN_CS    20
#define PIN_DC    10
#define PIN_RST   9
#define PIN_BL    21
#define BOOT_BUTTON_PIN 9

// ── Display geometry ────────────────────────────────────────────────
// landscape
#define TFT_W      284
#define TFT_H      76
#define X_OFFSET   18
#define Y_OFFSET   82

ST7789_76x284 tft(TFT_W, TFT_H, X_OFFSET, Y_OFFSET);
WebServer server(80);
esp_pm_lock_handle_t pwm_lock; // Define the lock handle

unsigned long lastUpdate = 0;
unsigned brightness = 80;
String pendingMessage = "";
bool newMessageReady = false;

void goToSleep() {

    server.stop();
    WiFi.mode(WIFI_OFF);

    // Force the RC_FAST (8MHz) clock to stay on during sleep - required for backlight PWM
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_ON);
    // prevent GPIO PIN_BL isolation during sleep - required for backlight PW
    gpio_sleep_sel_dis((gpio_num_t)PIN_BL);
    
    // prevent display reset during sleep
    pinMode(PIN_RST, OUTPUT);
    digitalWrite(PIN_RST, HIGH);
    gpio_hold_en((gpio_num_t)PIN_RST); // Lock the pin state in the RTC pad
    gpio_sleep_sel_dis((gpio_num_t)PIN_RST); // Disable GPIO isolation

    // Configure wake up on GPIO9 (Boot Button) being LOW
    gpio_wakeup_enable((gpio_num_t)BOOT_BUTTON_PIN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    // Start sleep
    tft.setBrightness(50);
    Serial.println("going to light sleep");
    sleep(100);
    esp_light_sleep_start();

    //waking up
    gpio_hold_dis((gpio_num_t)PIN_RST); // Release display reset the pin hold so the CPU can control it again
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SSID, PASS); // Restore WiFi AP
    server.begin();  // Restart the WebServer listener
    tft.setBrightness(80);
    Serial.println("waked up");
}

// ── Setup ───────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(5000);
    Serial.println("Serial started");

    if (!LittleFS.begin()) {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    WiFi.mode(WIFI_OFF); // Force radio off first
    delay(100);
    WiFi.mode(WIFI_AP);
    delay(100); 
    WiFi.setTxPower(WIFI_POWER_8_5dBm); // decreased WiFi power
    if(WiFi.softAP(SSID, PASS, 1)) {
        Serial.println("SoftAP Started Successfully");
        Serial.print("IP Address: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("SoftAP Failed to Start");
    }


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
            pendingMessage = server.arg("text");
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
    snprintf(buf, sizeof(buf), "SSID: %s", SSID);
    tft.drawText(5,   35, buf, WHITE, BLACK, 2);
    snprintf(buf, sizeof(buf), "Pass: %s", PASS);
    tft.drawText(5,   57, buf, WHITE, BLACK, 2);

    // QR fot WiFi connection
    static char qrBuffer[128];
    snprintf(qrBuffer, sizeof(qrBuffer), "WIFI:S:%s;T:WPA;P:%s;;", SSID, PASS);
    tft.drawQR(qrBuffer);
    tft.setBrightness(brightness);

    Serial.println("Done.");
}

void loop() {
    server.handleClient();

    if(millis() - lastUpdate > 1000){
        if(brightness < 50){
            brightness = 80;
        } else {
            brightness = 20;
        }
        tft.setBrightness(brightness);
        lastUpdate = millis();
        Serial.println(brightness);
    }

    if (newMessageReady) {
        newMessageReady = false;
        if(pendingMessage == "sleep"){
            goToSleep();
        } else {
             tft.fillScreen(BLACK);
            tft.drawText(5, 5, pendingMessage.c_str(), WHITE, BLACK, 2);
        }
    }
}