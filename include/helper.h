#ifndef helper_DY_ESP_bage
#define helper_DY_ESP_bage

#include <Arduino.h>
#include "esp_sleep.h"
#include <ST7789_76x284.h>
#include "esp_adc_cal.h"
#include <WebServer.h>
#include <LittleFS.h>

extern esp_adc_cal_characteristics_t adc_chars; // Declaration of ADC characteristics

struct VoltagePoint {
    float voltage;
    int percentage;
};

// Battery initial cpacity
static const unsigned bat_max_cap = 350;

// Standard Li-ion discharge curve Lookup Table
static const VoltagePoint lut[] = {
    {4.20, 100},
    {4.10, 90},
    {4.00, 80},
    {3.90, 65},
    {3.80, 45},
    {3.70, 20},
    {3.60, 10},
    {3.50, 5},
    {3.20, 0}
};
static const int lut_size = sizeof(lut) / sizeof(lut[0]);

namespace UI {
    constexpr int TITLE_X = 5;
    constexpr int BATTERY_ICON_X = 5;
    constexpr int BATTERY_VOLT_X = 13;
    constexpr int BATTERY_PERCENT_X = 48;
    constexpr int BATTERY_INFO_X = 80;
    constexpr int BATTERY_INFO_Y = 68;
    constexpr int TITLE_1_Y = 14;
    constexpr int TITLE_2_Y = 4;
    constexpr int SUBTITLE_Y = 40;
}

void serveFile(WebServer &server, const char* route, const char* path, const char* mime);
void goToSleep(unsigned sec, int8_t bl);
int get_percentage(float v);
float measureBattery(int8_t pin_bat);
void indicateBatteryLevel(int8_t pin_bat, ST7789_76x284 &tft, unsigned brightness, uint16_t bg_color, bool calculate_remaining_time_flag);

#endif
