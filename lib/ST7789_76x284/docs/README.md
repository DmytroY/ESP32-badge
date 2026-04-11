# ST7789 76x284 ESP32-C3 Driver

A lightweight C++ driver for the ST7789 (76x284px) TFT display, optimized for the ESP32-C3 using the ESP-IDF LEDC peripheral for backlight control and hardware SPI.

## Features
* Custom Resolution: Specifically configured for 76x284 displays.
* Dual Fonts: Built-in 5x7 and 10x14 pixel fonts.
* Predefined colors: BLACK, WHITE, BLUE, GREEN, RED, YELLOW, MAGENTA, CYAN   
* Hardware PWM: Smooth 8-bit brightness control using the ESP32-C3 RTC clock.
* Graphics Primitives: Support for pixels, lines, rectangles, and filled areas.
* QR Code Support: Integrated generation and rendering.

## Hardware Configuration
### Pin Mapping (ESP32-C3)

Connected pins can be redefined in begin().  GPIO pin configurations variants below were tested and confirmed usable:
| Display Pin | Function | Variant 1 | Variant 2|
| -------- | ------- | ------| -----|
| SCLK | SPI Clock | GPIO 2 | GPIO 7 |
| MOSI | SPI Data | GPIO 3 | GPIO 8 |
| CS | Chip Select | GPIO 5| GPIO 20|
| DC | Data/Command | GPIO 4 | GPIO 10|
|RST | Reset | GPIO 10 | GPIO 8|
| BL | Backlight PWM |GPIO 11| GPIO 21|

## API Reference

### Initialization
```
ST7789_76x284(uint16_t w, uint16_t h, uint16_t x_off, uint16_t y_off);
void begin(int8_t sclk, int8_t mosi, int8_t cs, int8_t dc, int8_t rst, int8_t bl);
```

### Display Control
* `fillScreen(uint16_t color)`: Fills the entire display.
* `setBrightness(uint8_t percentage)`: Sets backlight from 0-100%.

### Drawing Primitives
* `drawPixel(int16_t x, int16_t y, uint16_t color)`
* `fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)`
* `drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)`

### Text and Fonts
`drawText(int16_t x, int16_t y, const char* str, uint16_t fg, uint16_t bg, uint8_t scale`

* Scale 1, 3, 5...: Uses the 5x7 font.
* Scale 2, 4, 6...: Uses the 10x14 font.
#### ***str*** can contains printable ASCII simbols or predefined icons:
1. 0x7F - low consumption mode
1. 0x80 - Battery 0%
1. 0x81 - Battety 25%
1. 0x82 - Battety 50%
1. 0x83 - Battety 75%
1. 0x84 - Battety 100%
1. 0x85 - WiFi off
1. 0x86 - WiFi on

example of usage:

``` display.drawChar(x, y, (char)0x7F, GREEN, BLACK, 1);```


### Display QR code
`drawQR(const char* str, int16_t x, int16_t y)`

Renders a QR code at specific coordinates.
In case of optional parameters x and y are not provided QR code renders at the far right position in lanscape mode and at the bottom in portrait mode



## Usage Example
C++
```
#include "ST7789_76x284.h"

// Define display size and offsets
ST7789_76x284 tft(76, 284, 18, 82);

void setup() {
    // Initialize pins (SCLK, MOSI, CS, DC, RST, BL)
    tft.begin(2, 3, 7, 6, 10, 11);
    
    tft.fillScreen(BLACK); // equal of 0x0000 argument
    tft.setBrightness(80); // 80% brightness
    
    tft.drawText(10, 10, "Hello ESP32-C3", WHITE, BLACK, 2);
    tft.drawQR("https://google.com"); // Center QR at bottom
}

void loop() {}
```

## Technical Details
* Color Format: BRG565 (16-bit).
* SPI Frequency: 8MHz (Adjustable in begin).
* PWM Frequency: 5kHz.
* Backlight: Optimized for active-low hardware configurations (100% brightness = 0 duty cycle).