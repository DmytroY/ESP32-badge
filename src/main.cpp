#include <Arduino.h>
#include <SPI.h>

// ── Pin definitions ─────────────────────────────────────────────────
#define PIN_MOSI  3
#define PIN_SCLK  2
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

// ── Colors RGB565 ───────────────────────────────────────────────────
#define BLACK      0x0000
#define WHITE      0xFFFF
#define BLUE       0xF800
#define GREEN      0x07E0
#define RED        0x001F
#define YELLOW     0x07FF
#define MAGENTA    0xF81F
#define CYAN       0xFFE0

// ── SPI helpers ─────────────────────────────────────────────────────
inline void cs_lo() { digitalWrite(PIN_CS, LOW); }
inline void cs_hi() { digitalWrite(PIN_CS, HIGH); }
inline void dc_lo() { digitalWrite(PIN_DC, LOW); }
inline void dc_hi() { digitalWrite(PIN_DC, HIGH); }

void write_byte(uint8_t b) {
    SPI.transfer(b);
}

void send_cmd(uint8_t cmd) {
    dc_lo(); cs_lo();
    write_byte(cmd);
    cs_hi();
}

void send_data(uint8_t data) {
    dc_hi(); cs_lo();
    write_byte(data);
    cs_hi();
}

void send_cmd_data(uint8_t cmd, const uint8_t* data, uint8_t len) {
    dc_lo(); cs_lo();
    write_byte(cmd);
    dc_hi();
    for (uint8_t i = 0; i < len; i++) write_byte(data[i]);
    cs_hi();
}

// ── Address window ──────────────────────────────────────────────────
void set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    x0 += X_OFFSET; x1 += X_OFFSET;
    y0 += Y_OFFSET; y1 += Y_OFFSET;

    uint8_t cx[] = { (uint8_t)(x0 >> 8), (uint8_t)(x0), (uint8_t)(x1 >> 8), (uint8_t)(x1) };
    uint8_t cy[] = { (uint8_t)(y0 >> 8), (uint8_t)(y0), (uint8_t)(y1 >> 8), (uint8_t)(y1) };
    send_cmd_data(0x2A, cx, 4); // set column address
    send_cmd_data(0x2B, cy, 4); // set row address
    send_cmd(0x2C);  // Memory write
}

// ── ST7789 init ─────────────────────────────────────────────────────
void tft_init() {
    pinMode(PIN_RST, OUTPUT);
    pinMode(PIN_CS,  OUTPUT);
    pinMode(PIN_DC,  OUTPUT);
    cs_hi();

    // Hardware reset
    digitalWrite(PIN_RST, HIGH); delay(10);
    digitalWrite(PIN_RST, LOW);  delay(50);
    digitalWrite(PIN_RST, HIGH); delay(150);

    send_cmd(0x01); delay(150);  // Software reset
    send_cmd(0x11); delay(255);  // Sleep out

    uint8_t colmod[] = { 0x55 };           // 16-bit color
    send_cmd_data(0x3A, colmod, 1); delay(10);

    // uint8_t madctl[] = { 0x08 };           // BGR, (portrait orientation by default)
    uint8_t madctl[] = { 0x68 };           // MX + MV + BGR (landscape)

    send_cmd_data(0x36, madctl, 1);

    send_cmd(0x20); delay(10);   // invertion OFF
    send_cmd(0x13); delay(10);   // Partial mode OFF
    send_cmd(0x29); delay(255);  // Display ON
}

// ── Graphics primitives ─────────────────────────────────────────────
void fillScreen(uint16_t color) {
    set_window(0, 0, TFT_W - 1, TFT_H - 1);
    dc_hi(); cs_lo();
    uint32_t total = (uint32_t)TFT_W * TFT_H;
    for (uint32_t i = 0; i < total; i++) {
        write_byte(color >> 8);
        write_byte(color & 0xFF);
    }
    cs_hi();
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (x >= TFT_W || y >= TFT_H || w <= 0 || h <= 0) return;
    if (x + w > TFT_W) w = TFT_W - x;
    if (y + h > TFT_H) h = TFT_H - y;
    set_window(x, y, x + w - 1, y + h - 1);
    dc_hi(); cs_lo();
    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        write_byte(color >> 8);
        write_byte(color & 0xFF);
    }
    cs_hi();
}

void drawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= TFT_W || y < 0 || y >= TFT_H) return;
    set_window(x, y, x, y);
    dc_hi(); cs_lo();
    write_byte(color >> 8);
    write_byte(color & 0xFF);
    cs_hi();
}

void drawHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    fillRect(x, y, w, 1, color);
}

void drawVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    fillRect(x, y, 1, h, color);
}

void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawHLine(x,         y,         w, color);
    drawHLine(x,         y + h - 1, w, color);
    drawVLine(x,         y,         h, color);
    drawVLine(x + w - 1, y,         h, color);
}

// ── 5x7 font ────────────────────────────────────────────────────────
static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // ' '
    {0x00,0x00,0x5F,0x00,0x00}, // '!'
    {0x00,0x07,0x00,0x07,0x00}, // '"'
    {0x14,0x7F,0x14,0x7F,0x14}, // '#'
    {0x24,0x2A,0x7F,0x2A,0x12}, // '$'
    {0x23,0x13,0x08,0x64,0x62}, // '%'
    {0x36,0x49,0x55,0x22,0x50}, // '&'
    {0x00,0x05,0x03,0x00,0x00}, // '\''
    {0x00,0x1C,0x22,0x41,0x00}, // '('
    {0x00,0x41,0x22,0x1C,0x00}, // ')'
    {0x14,0x08,0x3E,0x08,0x14}, // '*'
    {0x08,0x08,0x3E,0x08,0x08}, // '+'
    {0x00,0x50,0x30,0x00,0x00}, // ','
    {0x08,0x08,0x08,0x08,0x08}, // '-'
    {0x00,0x60,0x60,0x00,0x00}, // '.'
    {0x20,0x10,0x08,0x04,0x02}, // '/'
    {0x3E,0x51,0x49,0x45,0x3E}, // '0'
    {0x00,0x42,0x7F,0x40,0x00}, // '1'
    {0x42,0x61,0x51,0x49,0x46}, // '2'
    {0x21,0x41,0x45,0x4B,0x31}, // '3'
    {0x18,0x14,0x12,0x7F,0x10}, // '4'
    {0x27,0x45,0x45,0x45,0x39}, // '5'
    {0x3C,0x4A,0x49,0x49,0x30}, // '6'
    {0x01,0x71,0x09,0x05,0x03}, // '7'
    {0x36,0x49,0x49,0x49,0x36}, // '8'
    {0x06,0x49,0x49,0x29,0x1E}, // '9'
    {0x00,0x36,0x36,0x00,0x00}, // ':'
    {0x00,0x56,0x36,0x00,0x00}, // ';'
    {0x08,0x14,0x22,0x41,0x00}, // '<'
    {0x14,0x14,0x14,0x14,0x14}, // '='
    {0x00,0x41,0x22,0x14,0x08}, // '>'
    {0x02,0x01,0x51,0x09,0x06}, // '?'
    {0x32,0x49,0x79,0x41,0x3E}, // '@'
    {0x7E,0x11,0x11,0x11,0x7E}, // 'A'
    {0x7F,0x49,0x49,0x49,0x36}, // 'B'
    {0x3E,0x41,0x41,0x41,0x22}, // 'C'
    {0x7F,0x41,0x41,0x22,0x1C}, // 'D'
    {0x7F,0x49,0x49,0x49,0x41}, // 'E'
    {0x7F,0x09,0x09,0x09,0x01}, // 'F'
    {0x3E,0x41,0x49,0x49,0x7A}, // 'G'
    {0x7F,0x08,0x08,0x08,0x7F}, // 'H'
    {0x00,0x41,0x7F,0x41,0x00}, // 'I'
    {0x20,0x40,0x41,0x3F,0x01}, // 'J'
    {0x7F,0x08,0x14,0x22,0x41}, // 'K'
    {0x7F,0x40,0x40,0x40,0x40}, // 'L'
    {0x7F,0x02,0x0C,0x02,0x7F}, // 'M'
    {0x7F,0x04,0x08,0x10,0x7F}, // 'N'
    {0x3E,0x41,0x41,0x41,0x3E}, // 'O'
    {0x7F,0x09,0x09,0x09,0x06}, // 'P'
    {0x3E,0x41,0x51,0x21,0x5E}, // 'Q'
    {0x7F,0x09,0x19,0x29,0x46}, // 'R'
    {0x46,0x49,0x49,0x49,0x31}, // 'S'
    {0x01,0x01,0x7F,0x01,0x01}, // 'T'
    {0x3F,0x40,0x40,0x40,0x3F}, // 'U'
    {0x1F,0x20,0x40,0x20,0x1F}, // 'V'
    {0x3F,0x40,0x38,0x40,0x3F}, // 'W'
    {0x63,0x14,0x08,0x14,0x63}, // 'X'
    {0x07,0x08,0x70,0x08,0x07}, // 'Y'
    {0x61,0x51,0x49,0x45,0x43}, // 'Z'
    {0x00,0x7F,0x41,0x41,0x00}, // '['
    {0x02,0x04,0x08,0x10,0x20}, // '\'
    {0x00,0x41,0x41,0x7F,0x00}, // ']'
    {0x04,0x02,0x01,0x02,0x04}, // '^'
    {0x40,0x40,0x40,0x40,0x40}, // '_'
    {0x00,0x01,0x02,0x04,0x00}, // '`'
    {0x20,0x54,0x54,0x54,0x78}, // 'a'
    {0x7F,0x48,0x44,0x44,0x38}, // 'b'
    {0x38,0x44,0x44,0x44,0x20}, // 'c'
    {0x38,0x44,0x44,0x48,0x7F}, // 'd'
    {0x38,0x54,0x54,0x54,0x18}, // 'e'
    {0x08,0x7E,0x09,0x01,0x02}, // 'f'
    {0x0C,0x52,0x52,0x52,0x3E}, // 'g'
    {0x7F,0x08,0x04,0x04,0x78}, // 'h'
    {0x00,0x44,0x7D,0x40,0x00}, // 'i'
    {0x20,0x40,0x44,0x3D,0x00}, // 'j'
    {0x7F,0x10,0x28,0x44,0x00}, // 'k'
    {0x00,0x41,0x7F,0x40,0x00}, // 'l'
    {0x7C,0x04,0x18,0x04,0x78}, // 'm'
    {0x7C,0x08,0x04,0x04,0x78}, // 'n'
    {0x38,0x44,0x44,0x44,0x38}, // 'o'
    {0x7C,0x14,0x14,0x14,0x08}, // 'p'
    {0x08,0x14,0x14,0x18,0x7C}, // 'q'
    {0x7C,0x08,0x04,0x04,0x08}, // 'r'
    {0x48,0x54,0x54,0x54,0x20}, // 's'
    {0x04,0x3F,0x44,0x40,0x20}, // 't'
    {0x3C,0x40,0x40,0x40,0x7C}, // 'u'
    {0x1C,0x20,0x40,0x20,0x1C}, // 'v'
    {0x3C,0x40,0x30,0x40,0x3C}, // 'w'
    {0x44,0x28,0x10,0x28,0x44}, // 'x'
    {0x0C,0x50,0x50,0x50,0x3C}, // 'y'
    {0x44,0x64,0x54,0x4C,0x44}, // 'z'
    {0x00,0x08,0x36,0x41,0x00}, // '{'
    {0x00,0x00,0x7F,0x00,0x00}, // '|'
    {0x00,0x41,0x36,0x08,0x00}, // '}'
    {0x10,0x08,0x08,0x10,0x08}, // '~'
};

void drawChar(int16_t x, int16_t y, char c, uint16_t fg, uint16_t bg, uint8_t scale) {
    if (c < ' ' || c > '~') return;
    const uint8_t* glyph = font5x7[c - ' '];
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (uint8_t row = 0; row < 7; row++) {
            uint16_t color = (line & (1 << row)) ? fg : bg;
            fillRect(x + col * scale, y + row * scale, scale, scale, color);
        }
    }
    // Character gap
    fillRect(x + 5 * scale, y, scale, 7 * scale, bg);
}

void drawText(int16_t x, int16_t y, const char* str, uint16_t fg, uint16_t bg, uint8_t scale) {
    int16_t cx = x;
    while (*str) {
        if (*str == '\n') {
            cx = x;
            y += 8 * scale;
        } else {
            drawChar(cx, y, *str, fg, bg, scale);
            cx += 6 * scale;
        }
        str++;
    }
}

// ── Setup ───────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    SPI.begin(PIN_SCLK, -1, PIN_MOSI, PIN_CS);
    SPI.setFrequency(27000000);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);

    tft_init();

    // Color test
    fillScreen(RED);   delay(800);
    fillScreen(GREEN); delay(800);
    fillScreen(BLUE);  delay(800);
    fillScreen(BLACK);

    // Graphics test
    drawRect(0, 0, TFT_W, TFT_H, WHITE);
    drawRect(2, 2, TFT_W-4, TFT_H-4, CYAN);

    drawText(10,  10, "RED", RED, BLACK, 2);
    drawText(100,  10, "GREEN", GREEN, BLACK, 2);
    drawText(190,  10, "BLUE", BLUE, BLACK, 2);

    drawText(10,  50, "CYAN", CYAN, BLACK, 2);
    drawText(100,  50, "MAGENTA", MAGENTA, BLACK, 2);
    drawText(190,  50, "YELLOW", YELLOW, BLACK, 2);

    Serial.println("Done.");
}

void loop() {
    digitalWrite(LED_PIN, HIGH); delay(800);
    digitalWrite(LED_PIN, LOW);  delay(200);
}