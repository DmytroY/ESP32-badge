
# ESP32-C3 Badge with Web Server

A lightweight web server for the ESP32-C3 SuperMini bage featuring an interactive UI, LittleFS integration, and power-saving Light Sleep mode.

## Features
- **Asynchronous UI:**  without page refreshes using AJAX.
- **Filesystem:** HTML, CSS, and JS files are stored separately in the Flash memory via LittleFS.
- **Power Management:** Automatically enters Light Sleep after 60 seconds of inactivity.
- **Wakeup:** Hardware wakeup via the BOOT button (GPIO9).

## Hardware Setup
- **Board:** ESP32-C3 SuperMini
- **Onboard LED:** GPIO8 (Active-Low logic)
- **Boot Button:** GPIO9 (Used for wakeup)

## Software Installation
1. Install [PlatformIO](https://platformio.org/) in VS Code.
2. Clone this repository.
3. **Upload Filesystem:** - Open the PlatformIO sidebar.
   - Go to `Project Tasks` > `Platform` > `Build Filesystem Image`.
   - Click `Upload Filesystem Image`.
4. **Upload Code:** Click the standard `Upload` arrow.

## Usage
1. Connect your device to the WiFi network: `ESP32_C3`.
2. Password: `12345678`.
3. Open `http://192.168.4.1` in your browser.