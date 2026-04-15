#include "helper.h"


void goToSleep(unsigned sec, int8_t bl) {
    // send ESP32 in to light sleep and keep operational PWM on bl pin.
    // Usage: goToSleep(60, 10) where 60 is sleep time in secunds and 10 is pin connected to TFT backlight PWM.

    if(sec > 3600) sec = 3600; // max sleep not more than 1 hour

    // Keep RTC peripherals and 8M clock alive for PWM and GPIO holds
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC8M, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    // prevent GPIO PIN_BL isolation during sleep - required for backlight PW
    gpio_sleep_sel_dis((gpio_num_t)bl);

    // Configure Timer Wakeup (Time is in microseconds)
    uint64_t sleepTimeInUs = sec * 1000000ULL;
    esp_sleep_enable_timer_wakeup(sleepTimeInUs);

    // Start sleep
    esp_light_sleep_start();
    // waking up
    delay(100); // internal regulator and clocks to stabilize
}

void indicateBatteryLevel(int8_t pin_bat, ST7789_76x284 &tft, uint16_t bg_color){
    uint16_t text_color = (bg_color == BLACK) ? WHITE : BLACK;
    float bat_v = 2 * 2.5 * analogRead(pin_bat) / 4095;
    int bat_percent = static_cast<int>(round(100 * (bat_v - 3.2)));

    char bat_perc_buffer[5]; // 3-digit number + % + null terminator
    snprintf(bat_perc_buffer, sizeof(bat_perc_buffer), "%3d%%", bat_percent);
    tft.drawText(13, 68, bat_perc_buffer, text_color, bg_color, 1);

    if(bat_percent <= 20){
        tft.drawChar(5, 68, (char)0x80, RED, bg_color, 1);
    } else if(bat_percent <= 40){
        tft.drawChar(5, 68, (char)0x81, text_color, bg_color, 1);
    } else if(bat_percent <= 60){
        tft.drawChar(5, 68, (char)0x82, text_color, bg_color, 1);
     }else if(bat_percent <= 80){
        tft.drawChar(5, 68, (char)0x83, text_color, bg_color, 1);
    }else {
        tft.drawChar(5, 68, (char)0x84, GREEN, bg_color, 1);
    }

    tft.drawText(45, 68, "REMAINING BATTERY", text_color, bg_color, 1);
}