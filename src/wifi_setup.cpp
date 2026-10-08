#include "wifi_setup.h"

#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

// Wartet so lange, bis die WLAN-Verbindung steht.
void connectWiFi() {
  Serial.print("Verbinde WLAN");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WLAN verbunden");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}