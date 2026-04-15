#ifndef helper_DY_ESP_bage
#define helper_DY_ESP_bage

#include <Arduino.h>
#include "esp_sleep.h"
#include <ST7789_76x284.h>


void goToSleep(unsigned sec, int8_t bl);
void indicateBatteryLevel(int8_t pin_bat, ST7789_76x284 &tft, uint16_t bg_color);

#endif
