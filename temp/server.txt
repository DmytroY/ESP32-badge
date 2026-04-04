#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "esp_sleep.h"

#define BOOT_BUTTON_PIN 9
#define LED_PIN 8
#define LED_ON  LOW  // because LED is connected between +3.3V and a pin
#define LED_OFF HIGH

WebServer server(80);
unsigned long lastActivity = 0;
const unsigned long INACTIVITY_TIMEOUT = 60000; // ms

void handleActivity() {
    lastActivity = millis();
}

void goToSleep() {
    Serial.println("Inactivity detected. Entering Light Sleep...");
    Serial.flush();
    
    // Configure wake up on GPIO9 (Boot Button) being LOW
    gpio_wakeup_enable((gpio_num_t)BOOT_BUTTON_PIN, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();

    // Start sleep
    esp_light_sleep_start();

    // --- Execution resumes here after wakeup ---
    // 1. Restore WiFi AP
    WiFi.softAP("ESP32_C3", "12345678"); 
    
    // 2. Restart the WebServer listener
    server.begin();
    
    handleActivity();
    Serial.println("Woken up by button!");
}

void setup() {
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  WiFi.softAP("ESP32_C3", "12345678");

  // Serve the index.html file from LittleFS
  server.on("/", []() {
    handleActivity();
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/style.css", []() {
      File file = LittleFS.open("/style.css", "r");
      server.streamFile(file, "text/css");
      file.close();
  });

  server.on("/script.js", []() {
      File file = LittleFS.open("/script.js", "r");
      server.streamFile(file, "application/javascript");
      file.close();
  });

  server.on("/on", []() {
    handleActivity();
    digitalWrite(LED_PIN, LED_ON);
    server.send(200, "text/plain", "ok");
    Serial.println("-- LED ON --");
  });

  server.on("/off", []() {
    handleActivity();
    digitalWrite(LED_PIN, LED_OFF);
    server.send(200, "text/plain", "ok");
    Serial.println("-- LED OFF --");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  // Check for physical button press (counts as activity)
    if (digitalRead(BOOT_BUTTON_PIN) == LOW) {
        handleActivity();
    }

    // Check for timeout
    if (millis() - lastActivity > INACTIVITY_TIMEOUT) {
        goToSleep();
    }
}