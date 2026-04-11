#include "helper.h"

void goToSleep(unsigned sec, int8_t bl) {
    // send ESP32 in to light sleep and keep operational PWM on bl pin.
    // Usage: goToSleep(60, 10) where 60 is sleep time in secunds and 10 is pin connected to TFT backlight PWM.

    if(sec > 3600) sec = 3600; //limit sleep time up to 1 hour

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