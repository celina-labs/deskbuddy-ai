#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "display.h"
#include "face.h"
#include "pomodoro.h"
#include "touch.h"
#include "audio.h"
#include "assistant.h"
#include "wifi_setup.h"

// Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// OLED-Pins am XIAO ESP32-S3: D4 = SDA, D5 = SCL
#define OLED_SDA 5
#define OLED_SCL 6

// Gemeinsamer Zustand
bool timerRunning = false;
bool arbeitsphase = true;

unsigned long startTime = 0;
unsigned long duration = 0;
unsigned long arbeitsDauer = 1500000;
unsigned long pausenDauer = 300000;

unsigned long elapsedTime = 0;
unsigned long remainingTime = 0;
unsigned long gesamtSekunden = 0;
unsigned long minuten = 0;
unsigned long sekunden = 0;

bool stehPhase = false;
unsigned long stehSitzStartZeit = 0;
unsigned long sitzDauer = 2400000;
unsigned long stehDauer = 900000;

bool nachrichtAktiv = false;
unsigned long nachrichtStartZeit = 0;
unsigned long nachrichtDauer = 3000;
String nachrichtZeile1 = "";
String nachrichtZeile2 = "";

bool requestRunning = false;

void setup() {
  Serial.begin(115200);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Fehler");
    while (true) {}
  }

  connectWiFi();

  Wire.begin(OLED_SDA, OLED_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("Display Fehler");
    while (true) {}
  }

  initialisiereGesicht();

  display.clearDisplay();
  display.display();

  stehSitzStartZeit = millis();
  initialisiereTouch();

  Serial.println("Bereit.");
  Serial.println("Kurze Beruehrung: Pomodoro starten");
  Serial.println("Lange Beruehrung: Sprachassistent starten");
}

void loop() {
  pruefeTouchAktionen();
  aktualisiereTimer();
  pruefeSitzStehPhase();

  if (nachrichtAktiv && millis() - nachrichtStartZeit >= nachrichtDauer) {
    nachrichtAktiv = false;
  }

  zeigeTimer();
}
