#include "helper.h"

esp_adc_cal_characteristics_t adc_chars;  // Definition of ADC characteristics

void serveFile(WebServer &server, const char* route, const char* path, const char* mime) {
    server.on(route, [&server, path, mime]() {
        File file = LittleFS.open(path, "r");
        server.streamFile(file, mime);
        file.close();
    });
}

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

float measureBattery(int8_t pin_bat){
    // returns volts.
    // Takes in to account ADC calibration and HW divider 
    uint32_t raw_v = analogRead(pin_bat); // read uncalibrated voltage
    uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(raw_v, &adc_chars);  // Convert raw to mV using the calibrated characteristics
    return (voltage_mv * 2.16f) / 1000.0f; // Convert to actual battery voltage (7k5 + 6k2 voltage divider used)
}

int get_percentage(float v) {
    // appoximation of LiIon battery capacity based on voltage
    // lut is lookup table for lineary approximated zones
    if (v >= lut[0].voltage) return 100;
    if (v <= lut[lut_size - 1].voltage) return 0;

    for (int i = 0; i < lut_size - 1; i++) {
        if (v <= lut[i].voltage && v > lut[i+1].voltage) {
            // Linear interpolation between point i and i+1
            float diff_v = lut[i].voltage - lut[i+1].voltage;
            float diff_p = lut[i].percentage - lut[i+1].percentage;
            return lut[i+1].percentage + (int)((v - lut[i+1].voltage) * diff_p / diff_v);
        }
    }
    return 0;
}

void indicateBatteryLevel(int8_t pin_bat, ST7789_76x284 &tft, uint16_t bg_color, bool calculate_remaining_time_flag){
    // display batery icon, capacity % and estimater remaining time

    uint16_t text_color = (bg_color == BLACK) ? WHITE : BLACK;

    static float bat_v = measureBattery(pin_bat); // initial level
    float bat_v_raw = measureBattery(pin_bat); // curent level
    bat_v = (bat_v_raw * 0.1f) + (bat_v * 0.9f); //  EMA filter

    // Display battery voltage
    char bat_v_buff[6];
    snprintf(bat_v_buff, sizeof(bat_v_buff), "%.2fV", bat_v);
    tft.drawText(UI::BATTERY_VOLT_X, UI::BATTERY_INFO_Y, bat_v_buff, text_color, bg_color, 1);

    // Display battery %
    int bat_percent = get_percentage(bat_v);
    char bat_perc_buffer[5]; // 3-digit number + % + null terminator
    snprintf(bat_perc_buffer, sizeof(bat_perc_buffer), "%3d%%", bat_percent);
    tft.drawText(UI::BATTERY_PERCENT_X, UI::BATTERY_INFO_Y, bat_perc_buffer, text_color, bg_color, 1);

    // Display battery icon
    if(bat_percent <= 20){
        tft.drawChar(UI::BATTERY_ICON_X, UI::BATTERY_INFO_Y, (char)0x80, RED, bg_color, 1);
    } else if(bat_percent <= 40){
        tft.drawChar(UI::BATTERY_ICON_X, UI::BATTERY_INFO_Y, (char)0x81, text_color, bg_color, 1);
    } else if(bat_percent <= 60){
        tft.drawChar(UI::BATTERY_ICON_X, UI::BATTERY_INFO_Y, (char)0x82, text_color, bg_color, 1);
     }else if(bat_percent <= 80){
        tft.drawChar(UI::BATTERY_ICON_X, UI::BATTERY_INFO_Y, (char)0x83, text_color, bg_color, 1);
    }else {
        tft.drawChar(UI::BATTERY_ICON_X, UI::BATTERY_INFO_Y, (char)0x84, GREEN, bg_color, 1);
    }

    // Display remaining time
    if(calculate_remaining_time_flag){
        static const unsigned start_time = millis();
        static const float start_bat_v = measureBattery(pin_bat);
        static const int start_bat_percent = get_percentage(start_bat_v);

        unsigned long elapsed = millis() - start_time;
        float discharge_speed = (elapsed > 60000) ? (start_bat_percent - bat_percent) / (elapsed / 3600000.0f) : 10;
        discharge_speed = (discharge_speed  > 0) ? discharge_speed : 2.33;

        div_t remaining = div(bat_percent, discharge_speed);

        int rem_time_h = remaining.quot; // remaining time in hours
        int rem_time_m = static_cast<int>(60.0f * remaining.rem/discharge_speed); // remaining time in minutes
        char rem_time_buffer[17]; // including null terminator
        snprintf(rem_time_buffer, sizeof(rem_time_buffer), "remaining %2dh%2dm", rem_time_h, rem_time_m);
        tft.drawText(UI::BATTERY_INFO_X, UI::BATTERY_INFO_Y, rem_time_buffer, text_color, bg_color, 1);

    } else {
        tft.drawText(UI::BATTERY_INFO_X, UI::BATTERY_INFO_Y, "remaining ..h..m", text_color, bg_color, 1);
    }

}