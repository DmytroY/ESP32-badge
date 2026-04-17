# Electronic Conference Badge (ESP32)
A low-power electronic conference badge utilizing an ESP32 Super Mini and a custom-sized 76x284 ST7789 LCD. The badge features a web-based configuration portal to customize text, colors, and QR codes via WiFi.

## Features
- **Asynchronous Web Interface:** Configure badge details without page refreshes (AJAX).
- **Dynamic QR Generation:** Displays a WiFi setup QR code on boot and a user-defined QR code after configuration.
- **Persistent Display:** Shuts down WiFi and server after setup, going to light sleem to save power while keeping the information on screen.
- **Power Management:** Wakes every minute to update battery level and estimated remaining runtime.
- **LittleFS Storage:** HTML, CSS, and JS assets are stored separately in flash memory.

## Hardware Requirements
- MCU: ESP32 Super Mini
- Display: ST7789 LCD (76x284 resolution)
- Power: Li-Ion Battery + DC/DC converter to 3.3V
- Charging: 5V Li-Ion charger controller
- Battery Sensing: Voltage divider (7.5k + 6.2k resistors) connected to PIN_0 (ADC)

## Software Architecture
### Custom Display Driver
The project includes a dedicated cusom driver located in lib/ST7789_76x284/.
It supports basic display operations including drawing primitives (lines, rects), text rendering, and brightness control. See API documentation in lib/ST7789_76x284/docs/README.md.

### Helper Utilities
The helper.cpp file provides procedures for
- File Serving: Logic for handling LittleFS file requests for the web server.
- Power Management: Routines for deep sleep and wake cycles.
 - Battery Diagnostics: Procedures for measuring ADC voltage and rendering battery status/remaining time indicators on the screen.

## Pinout Configuration
- SCLK: 7
- MOSI: 8
- CS: 20
- DC: 10
- RST: 9
- Backlight (BL): 21
- Battery ADC: 0

## Software Installation
### Prerequisites
- Install VS Code
- Install the PlatformIO IDE extension
- Clone the repository.
- Upload Filesystem: Open the PlatformIO sidebar, go to Project Tasks > Platform > Build Filesystem Image, then click Upload Filesystem Image.
- Upload Code: Click the standard Upload arrow icon.

## Usage
- **On power on** the badge generates a unique SSID based on its MAC address (e.g., ESP-AABBCC).
- You can scan the QR code on the display or manually **connect to the WiFi** using password on screen.
- **Visit 192.168.4.1** in your browser.
- **Input** Title (Name), Subtitle (Position or Surname), and string for generating QR Code (Link), choose colors and brightness.
- **Use the Test button** to preview. Once satisfied, **press Finish**.
- **The badge keep screen on but enters sleep mode**, waking briefly every 60 seconds to monitor battery and dispaly remaining time.