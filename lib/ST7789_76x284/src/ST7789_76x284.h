#ifndef ST7789_76x284_H
#define ST7789_76x284_H

#include <Arduino.h>
#include <SPI.h>
#include <qrcode.h>
#include "driver/ledc.h"


// ── Colors BRG565 ───
#define BLACK      0x0000
#define WHITE      0xFFFF
#define BLUE       0xF800
#define GREEN      0x07E0
#define RED        0x001F
#define YELLOW     0x07FF
#define MAGENTA    0xF81F
#define CYAN       0xFFE0

class ST7789_76x284 {
    public:
        ST7789_76x284(uint16_t w, uint16_t h, uint16_t x_off, uint16_t y_off);

        void begin(int8_t sclk, int8_t mosi, int8_t cs, int8_t dc, int8_t rst, int8_t bl);
        void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        void fillScreen(int16_t color);
        void drawPixel(int16_t x, int16_t y, uint16_t color);
        void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
        void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
        void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
        void drawChar(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale);
        void drawText(int16_t x, int16_t y, const char* str, uint16_t fg, uint16_t bg, uint8_t scale);
        void drawQR(const char* str, int16_t x = -1, int16_t y = -1);
        void setBrightness(uint8_t percentage); // 0 to 100%

    private:
        uint16_t _w, _h, _x_off, _y_off;
        int8_t _cs, _dc, _rst, _bl;

        void send_cmd(uint8_t cmd);
        void send_data(uint8_t data);
        void send_cmd_data(uint8_t cmd, const uint8_t* data, uint8_t len);
        void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
        void write_byte(uint8_t b);
        void initPWM(int8_t pin);
};

#endif