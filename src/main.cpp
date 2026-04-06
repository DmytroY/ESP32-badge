#include <Arduino.h>
#include <ST7789_76x284.h>
#include "esp_pm.h"
#include "esp_sleep.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/gpio.h"

// ── Pin definitions ─────────────────────────────────────────────────
#define PIN_SCLK  2
#define PIN_MOSI  3
#define PIN_CS    5
#define PIN_DC    4
#define PIN_RST   10
#define PIN_BL    1
#define LED_PIN   8
#define BOOT_BUTTON_PIN 9

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

ST7789_76x284 tft(TFT_W, TFT_H, X_OFFSET, Y_OFFSET);

esp_pm_lock_handle_t pwm_lock; // Define the lock handle

void goToSleep() {
    tft.drawText(5,  42, "Going to sleep", RED, WHITE, 2);
    pinMode(PIN_RST, INPUT_PULLUP); // prevent display reset during sleep
    
    // Configure wake up on GPIO9 (Boot Button) being LOW
    gpio_wakeup_enable((gpio_num_t)BOOT_BUTTON_PIN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    // Force the RC_FAST (8MHz) clock to stay on
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_ON);
    
    // prevent GPIO PIN_BL isolation during sleep
    gpio_sleep_sel_dis((gpio_num_t)PIN_BL);

    // Start sleep
    esp_light_sleep_start();
    
    pinMode(PIN_RST, OUTPUT); // Restore RST pin to OUTPUT after waking
    digitalWrite(PIN_RST, HIGH);

    tft.drawText(5,  42, "   Woked up   ", GREEN, BLACK, 2);
}

// ── Setup ───────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

    // Create the lock for the Internal 8MHz oscillator
    // This prevents the system from turning off the RC_FAST clock during sleep
    // so PWD can use it during light sleep
    esp_pm_lock_create(ESP_PM_APB_FREQ_MAX, 0, "pwm_keep_alive", &pwm_lock);
    
    // Acquire the lock
    esp_pm_lock_acquire(pwm_lock);


    tft.begin(PIN_SCLK, PIN_MOSI, PIN_CS, PIN_DC, PIN_RST, PIN_BL);

    tft.fillScreen(BLACK);
    // tft.drawRect(0, 0, TFT_W, TFT_H, WHITE);

    tft.drawQR("https://github.com/DmytroY/ESP32-badge");

    tft.drawText(5,   5, "Hello World!", WHITE, BLACK, 2);


    Serial.println("Done.");
}

void loop() {
    for(int i = 0; i < 3; i++){
        tft.setBrightness(20); // 50% brightness
        digitalWrite(LED_PIN, HIGH); delay(2000);
        tft.setBrightness(100);; // 100% brightness
        digitalWrite(LED_PIN, LOW);  delay(2000);
    }
    tft.setBrightness(50); // 20% brightness
    goToSleep();
}