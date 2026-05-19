# Building a Low-Power ESP32 Conference Badge with Custom Display Driver and Web Configurator

## Introduction

Welcome to a deep dive into designing and building a sophisticated conference badge that combines embedded systems engineering, hardware optimization, and web technologies. This project demonstrates full-stack embedded development—from developing low-level SPI drivers to creating responsive web interfaces—all while maintaining impressive battery efficiency.

The **ESP32 Electronic Badge** is a portable, battery-powered device perfect for conferences, trade shows, and events. It features a compact 76x284px OLED display, WiFi-based configuration portal, and intelligent power management that extends operational lifetime on a single charge.

## Project Overview

### What Makes This Badge Special?

Unlike generic name badge projects, this build showcases:

1. **Custom Hardware Driver** – A bespoke ST7789 display driver for non-standard display resolution (76x284px)
2. **Optimized Power Management** – Light Sleep mode maintaining display state while minimizing battery drain
3. **Zero-Config Setup** – Automatic WiFi SSID generation for intuitive user experience
4. **Full-Stack Development** – Hardware integration, firmware architecture, and responsive web UI
5. **Real-Time Battery Analytics** – Advanced ADC calibration and signal processing for accurate capacity estimation

### Technical Specifications

| Component | Details |
|-----------|---------|
| **Microcontroller** | ESP32 Super Mini |
| **Display** | ST7789 LCD (76×284 resolution) |
| **Battery** | Li-Ion with DC/DC converter (3.3V regulated) |
| **Power Modes** | Setup Mode (WiFi enabled) / Display Mode (WiFi off, low-power) |
| **Connectivity** | WiFi 2.4GHz (802.11 b/g/n) |
| **Storage** | LittleFS on Flash (HTML/CSS/JS assets) |
| **Communication Protocol** | SPI (Hardware) |
| **Languages** | C++ (92.7%), JavaScript (3.8%), HTML/CSS (4%) |

## Hardware Deep Dive

### Why a Custom Display Driver?

The 76x284 resolution is unusually narrow—perfect for a badge worn on a lanyard. However, existing ST7789 libraries assumed standard resolutions (320x240, 240x320). Rather than compromise with a generic approach, I developed a dedicated driver from scratch.

**Key Features of the Custom Driver:**

- **Graphics Primitives**: Pixels, lines, rectangles, filled areas
- **Dual Font System**: 5×7 and 10×14 pixel fonts with 3-level scaling
- **PWM Backlight Control**: Smooth 0-100% brightness adjustment via ESP32 LEDC peripheral
- **QR Code Rendering**: Built-in QR code generation for seamless WiFi linking
- **Custom Icons**: Battery indicators, WiFi status symbols, and power mode icons

### EMI Mitigation and Pin Optimization

During prototyping, RF interference between the WiFi antenna and SPI/PWM signals caused display glitches. The solution involved:

1. **Physical Layout Optimization**: Minimizing trace lengths between MCU and display pins
2. **Strategic Pin Assignment**: 
   - SCLK on GPIO 7 (not GPIO 2)
   - MOSI on GPIO 8 (not GPIO 3)
   - Keep WiFi antenna at device edge
3. **Result**: Eliminated EMI issues while maintaining high-speed SPI communication (8 MHz)

**Recommended Pinout for Stable Operation:**

```
SCLK: GPIO 7
MOSI: GPIO 8
CS: GPIO 20
DC: GPIO 10
RST: GPIO 9
BL (Backlight): GPIO 21
Battery ADC: GPIO 0
```

### Power System Architecture

The badge uses a sophisticated two-stage power architecture:

1. **Input Stage**: Li-Ion battery → TP4054 charging controller (5V input)
2. **Regulation Stage**: LP1117 or similar LDO → 3.3V regulated output
3. **Sensing Stage**: 7.5kΩ / 6.2kΩ voltage divider → ADC pin (optimal range)

This design ensures safe charging while maintaining stable MCU and display voltage under varying loads.

## Software Architecture

### Firmware Organization

The firmware is structured for modularity and low memory footprint:

```
src/
├── main.cpp              # Setup/loop entry points
├── wifi_config.cpp       # Configuration portal logic
├── display_manager.cpp   # Display state machine
├── power_manager.cpp     # Sleep/wake routines
├── battery_manager.cpp   # ADC, calibration, analytics
└── helper.cpp            # Utility functions

lib/
└── ST7789_76x284/        # Custom display driver
    ├── ST7789_76x284.h
    ├── ST7789_76x284.cpp
    ├── qrcode.h          # QR code generation
    └── docs/
        └── README.md     # Driver API documentation

data/
├── index.html            # Configuration portal UI
├── style.css             # Styling
└── script.js             # AJAX communication
```

### Two-Mode Operation

#### 1. Setup Mode (Configuration)
- **Active Components**: WiFi, Web Server, Display
- **Current Draw**: ~170 mA @ 50% brightness
- **Duration**: User-dependent (typically 2-5 minutes)
- **User Actions**:
  - Badge broadcasts unique SSID (derived from MAC address)
  - User scans QR code or enters WiFi password
  - Browser opens 192.168.4.1 (AP configuration portal)
  - User enters Title, Subtitle, QR content, and color scheme
  - Test button previews the design
  - Finish button saves configuration and transitions to Display Mode

#### 2. Display Mode (Low-Power)
- **Active Components**: Display, PWM backlight, RTC
- **Current Draw**: 24 mA (min) to 110 mA (max brightness)
- **Duration**: 20-30 hours on single Li-Ion charge (typical usage)
- **Behavior**:
  - ESP32 enters Light Sleep (not Deep Sleep)
  - PWM signal for backlight continues uninterrupted (maintains screen on)
  - System wakes every 60 seconds to:
    - Poll battery voltage
    - Update remaining runtime display
    - Check for interrupts
  - WiFi and web server shut down completely

### Power Management Strategy: Why Light Sleep?

Deep Sleep would reduce current to ~10 mA but would disable the PWM signal—causing the backlight to turn off. Light Sleep maintains:

- CPU oscillator
- GPIO pin states and PWM signal
- RTC memory (for timestamp and configuration persistence)
- Ability to wake on timer or external interrupt

This hybrid approach achieves 78% power reduction vs. full operation while preserving display state and backlight PWM.

### Battery Measurement with ADC Calibration

Accurate battery capacity estimation is critical for displaying remaining runtime. The firmware implements:

#### 1. ADC Characterization
```cpp
esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, 
                         ADC_WIDTH_BIT_12, 1100, &adc_chars);
```

This accounts for ESP32-specific ADC non-linearity across voltage ranges.

#### 2. Exponential Moving Average Filter
```cpp
bat_v = (bat_v_raw * 0.1f) + (bat_v * 0.9f);  // Smooth noisy readings
```

Stabilizes readings from noisy ADC input before processing.

#### 3. Voltage-to-Capacity Mapping
Li-Ion batteries exhibit non-linear discharge curves. The firmware implements piecewise voltage zones:

| Voltage Range | Capacity |
|---------------|----------|
| > 4.0V | 90-100% |
| 3.8-4.0V | 60-90% |
| 3.6-3.8V | 30-60% |
| 3.4-3.6V | 10-30% |
| < 3.4V | 0-10% (warning) |

Runtime estimates account for brightness setting and current mode, displayed on-screen in minutes.

### Asynchronous Web Portal (AJAX)

The configuration interface uses:

- **LittleFS Storage**: Embedded filesystem stores HTML/CSS/JS in Flash
- **AJAX (XMLHttpRequest)**: Seamless form submission without page reloads
- **RESTful API**: Simple `/api/config` endpoint for data exchange
- **Real-Time Preview**: Test button reflects changes instantly

This architecture keeps the firmware footprint minimal while delivering responsive UX.

## Assembly and Deployment

### Bill of Materials (BoM)

| Component | Quantity | Notes |
|-----------|----------|-------|
| ESP32 Super Mini | 1 | 16MB Flash recommended |
| ST7789 Display 76×284 | 1 | Custom resolution module |
| Li-Ion Battery (1000-2000mAh) | 1 | 3.7V nominal |
| TP4054 Charging Controller | 1 | Integrated charger IC |
| LP1117 LDO (3.3V) | 1 | Voltage regulation |
| Resistors (7.5kΩ, 6.2kΩ) | 2 | Voltage divider |
| Capacitors (10µF, 100nF) | 4 | Decoupling & filtering |
| USB Micro-B Connector | 1 | Charging port |
| M2 Mounting Hardware | 4 | PCB assembly |
| Enclosure/Case | 1 | Custom 3D printed or machined |

### Software Installation

The project uses **PlatformIO** for development and deployment:

1. **Clone Repository**:
   ```bash
   git clone https://github.com/DmytroY/ESP32-badge.git
   cd ESP32-badge
   ```

2. **Build Filesystem Image** (contains HTML/CSS/JS):
   - Open PlatformIO sidebar in VS Code
   - Navigate to "Project Tasks" → "Platform"
   - Click "Build Filesystem Image"
   - Click "Upload Filesystem Image"

3. **Upload Firmware**:
   - Click the standard "Upload" arrow icon
   - Firmware and filesystem now reside on ESP32

### First Time User Experience

1. **Power On**: Badge boots in Setup Mode
2. **WiFi Connection**: Display shows unique SSID (e.g., `ESP32-BADGE-A4B2C1`) and WiFi password
3. **Configuration**: User scans QR code or manually connects to hotspot
4. **Web Interface**: Opens 192.168.4.1 in browser
5. **Customization**: Enter name, title, website URL, select colors and brightness
6. **Preview**: Test button shows design on display in real-time
7. **Finish**: Saves configuration and enters Low-Power Display Mode
8. **Runtime**: Badge remains on indefinitely (battery-limited), waking every 60 seconds for battery updates

## Key Achievements

✅ **Full-Stack Development**: From SPI driver authorship to responsive web UI  
✅ **Power Efficiency**: 24-110 mA operating current (mode/brightness dependent)  
✅ **Non-Standard Resolution Support**: Custom 76×284 driver eliminates generic compromises  
✅ **Zero-Config Setup**: Automatic AP generation—no manual WiFi credentials  
✅ **Accurate Battery Analytics**: ADC calibration + EMA filtering + piecewise voltage mapping  
✅ **EMI Optimization**: Pinout strategy eliminates RF interference in compact form factor  
✅ **Responsive UI**: AJAX-based configuration portal with real-time preview  

## Design Insights & Lessons Learned

### Challenge 1: EMI in Compact Layout
**Problem**: WiFi and SPI interference causing display artifacts.  
**Solution**: Strategic GPIO reassignment and minimized trace lengths.  
**Lesson**: Physical layout is as critical as firmware in RF-sensitive designs.

### Challenge 2: ADC Accuracy
**Problem**: Battery readings fluctuated wildly, making runtime estimates unreliable.  
**Solution**: ADC characterization + EMA filtering + lookup table calibration.  
**Lesson**: Raw sensor data requires multi-stage conditioning for real-world use.

### Challenge 3: Display Resolution Limitations
**Problem**: Existing libraries didn't support 76×284 resolution.  
**Solution**: Custom driver from first principles.  
**Lesson**: Sometimes "building it yourself" is the only path to optimization.

### Challenge 4: Power vs. Display State
**Problem**: Deep Sleep saved power but turned off backlight (via PWM).  
**Solution**: Light Sleep provides 78% power savings while maintaining PWM.  
**Lesson**: Understanding peripheral interaction enables creative power optimization.

## Use Cases & Applications

- **Conference Badges**: Dynamic attendee information + WiFi link to schedule
- **Trade Show Displays**: Battery-powered portable product information
- **Event Credentials**: QR code for on-site check-in
- **Museum/Gallery Info**: Standalone information display
- **Personal Portfolio Device**: Networking at tech meetups

## Extending the Project

### Potential Enhancements

1. **Bluetooth Support**: Add BLE for mobile app configuration
2. **Sensors**: Integrate temperature/humidity for environmental display
3. **Multiple Profiles**: Store 3-5 configurations, switch on-device
4. **E-ink Display**: Further power reduction for static content
5. **3D Enclosure**: Design custom case with lanyard attachment
6. **Multi-Language Support**: Dynamically load language packs

## Getting Started

### Repository Links

- **GitHub**: [DmytroY/ESP32-badge](https://github.com/DmytroY/ESP32-badge)
- **Topics**: `esp32`, `embedded`, `hardware-driver`, `low-power-design`, `embedded-web-server`, `adc-calibration`, `littlefs`

### Required Tools & Skills

- **Hardware**: Soldering iron, multimeter, USB FTDI adapter
- **Firmware**: C++, PlatformIO, ESP32 APIs
- **Web Frontend**: Basic HTML/CSS/JavaScript
- **Embedded Concepts**: SPI protocol, PWM, ADC, power management

### Documentation

Complete API reference and pinout diagrams are available in the repository's [`lib/ST7789_76x284/docs/`](https://github.com/DmytroY/ESP32-badge/tree/main/lib/ST7789_76x284) directory.

## Technical Specifications Summary

| Aspect | Specification |
|--------|--------------|
| **Processor** | Xtensa dual-core 32-bit (up to 240MHz) |
| **RAM** | 520KB SRAM |
| **Flash** | 16MB (typical) |
| **Display** | 76×284 TFT LCD (ST7789) |
| **SPI Frequency** | 8 MHz |
| **PWM Frequency** | 5 kHz (backlight) |
| **WiFi Standard** | 802.11 b/g/n (2.4 GHz) |
| **Setup Current** | ~170 mA (50% brightness) |
| **Display Idle Current** | 24-110 mA (brightness-dependent) |
| **Battery** | Li-Ion 1000-2000 mAh |
| **Runtime** | 20-30 hours (typical usage) |

## Conclusion

The ESP32 Electronic Badge demonstrates that thoughtful embedded systems design requires more than firmware—it demands attention to power electronics, PCB layout, driver optimization, and user experience. By combining a custom display driver, intelligent power management, and an intuitive web interface, we've created a device that's both impressive to build and practical to use.

Whether you're preparing for your next conference, building a portfolio device, or exploring embedded systems, this project provides a complete roadmap from concept through optimization. The open-source code, comprehensive documentation, and reusable driver library serve as a foundation for your own badge or portable display projects.

Happy building! 🚀

---

**Project Author**: Dmytro Y.  
**License**: MIT  
**Repository**: https://github.com/DmytroY/ESP32-badge  
**Status**: Open-source, actively maintained
