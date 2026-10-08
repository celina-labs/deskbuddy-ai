#include "display.h"

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "face.h"

extern Adafruit_SSD1306 display;

extern bool nachrichtAktiv;
extern unsigned long nachrichtStartZeit;
extern String nachrichtZeile1;
extern String nachrichtZeile2;

extern bool timerRunning;
extern bool arbeitsphase;
extern unsigned long remainingTime;
extern unsigned long gesamtSekunden;
extern unsigned long minuten;
extern unsigned long sekunden;

static bool antwortAktiv = false;
static unsigned long antwortStartZeit = 0;
static const unsigned long antwortDauer = 10000;
static String antwortText = "";

void zeigeNachricht(String zeile1, String zeile2) {
  antwortAktiv = false;
  nachrichtAktiv = true;
  nachrichtStartZeit = millis();
  nachrichtZeile1 = zeile1;
  nachrichtZeile2 = zeile2;
}

void zeigeAntwort(const String& text) {
  nachrichtAktiv = false;
  antwortAktiv = true;
  antwortStartZeit = millis();
  antwortText = text;
}

static void zeichneNachricht() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(8, 12);
  display.println(nachrichtZeile1);
  display.setCursor(0, 38);
  display.println(nachrichtZeile2);
  display.display();
}

static void zeichneAntwort() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setTextWrap(true);
  display.setCursor(0, 0);
  display.println(antwortText);
  display.display();
}

void zeigeTimer() {
  if (antwortAktiv) {
    if (millis() - antwortStartZeit >= antwortDauer) {
      antwortAktiv = false;
    } else {
      zeichneAntwort();
      return;
    }
  }

  if (nachrichtAktiv) {
    zeichneNachricht();
    return;
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  if (timerRunning) {
    gesamtSekunden = remainingTime / 1000;
    minuten = gesamtSekunden / 60;
    sekunden = gesamtSekunden % 60;

    display.setTextSize(1);
    display.setCursor(28, 0);

    if (arbeitsphase) {
      display.println("focustime <3");
    } else {
      display.println("break <3");
    }

    display.setTextSize(3);
    display.setCursor(23, 30);
    display.print(minuten);
    display.print(":");

    if (sekunden < 10) {
      display.print("0");
    }

    display.println(sekunden);
    display.display();
  } else {
    updateGesicht();
  }
}
