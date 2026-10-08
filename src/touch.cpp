#include "touch.h"

#include <Arduino.h>
#include "pomodoro.h"
#include "assistant.h"

// Touch: D3 = GPIO4
static const int touchPin = 4;

static bool touchAktiv = false;
static bool letzterTouchAktiv = false;
static bool touchGedrueckt = false;

static unsigned long touchStartZeit = 0;
static unsigned long letzteTouchAktion = 0;
static unsigned long touchSperreBis = 0;

static const unsigned long debounceMs = 100;
static const unsigned long longPressMs = 650;

static int touchRuhewert = 0;
static int touchAnSchwelle = 0;
static int touchAusSchwelle = 0;
static int touchVorwarnSchwelle = 0;
static int touchLetzterWert = 0;
static int touchFehlkalibrierungTreffer = 0;

// Feinjustierung fuer die Empfindlichkeit.
// Ruhe liegt bei dir meist bei ca. 22.700 bis 22.800.
// Eine leichte Beruehrung liegt schon bei ca. 24.600.
// Eine normale Beruehrung liegt oft bei ca. 25.000 bis 25.400.
// Deshalb soll der Touch schon deutlich frueher reagieren.
static const int touchOffsetVorwarnung = 900;
static const int touchOffsetAn = 1700;
static const int touchOffsetAus = 700;

// Damit nicht ein einzelner Ausreisser reicht, zaehlen wir mehrere Treffer.
// Gleichzeitig soll der Touch aber frueh genug anspringen.
static int touchTreffer = 0;
static const int touchTrefferZumStart = 1;
static const int touchTrefferZumLoslassen = 2;
static int touchLoslassenTreffer = 0;

extern bool timerRunning;
extern bool requestRunning;

// Liest den Touch-Pin mehrfach und bildet daraus einen Mittelwert.
static int leseTouchGemittelt(int anzahl) {
  long summe = 0;
  for (int i = 0; i < anzahl; i++) {
    summe += touchRead(touchPin);
    delay(3);
  }
  return summe / anzahl;
}

// Kalibriert den Ruhewert beim Start, damit der Touch stabiler reagiert.
static void kalibriereTouch() {
  const int messungen = 25;
  int kleinsterWert = 1000000;
  long summeKlein = 0;
  int anzahlKlein = 0;

  // Wir nehmen bewusst die kleineren, stabilen Werte als Ruhebasis.
  // So faengt eine versehentliche Beruehrung beim Start die Kalibrierung
  // nicht komplett kaputt an.
  for (int i = 0; i < messungen; i++) {
    int wert = leseTouchGemittelt(2);
    if (wert < kleinsterWert) {
      kleinsterWert = wert;
    }
    delay(6);
  }

  for (int i = 0; i < messungen; i++) {
    int wert = leseTouchGemittelt(2);
    if (wert <= kleinsterWert + 600) {
      summeKlein += wert;
      anzahlKlein++;
    }
    delay(6);
  }

  if (anzahlKlein > 0) {
    touchRuhewert = summeKlein / anzahlKlein;
  } else {
    touchRuhewert = kleinsterWert;
  }

  // Bei deinem Board steigt der Wert beim Beruehren an.
  // Vorwarnung ist schon knapp ueber dem Ruhewert,
  // die eigentliche AN-Schwelle liegt in deinem typischen Reaktionsbereich.
  touchVorwarnSchwelle = touchRuhewert + touchOffsetVorwarnung;
  touchAnSchwelle = touchRuhewert + touchOffsetAn;
  touchAusSchwelle = touchRuhewert + touchOffsetAus;

  touchTreffer = 0;
  touchLoslassenTreffer = 0;
  touchAktiv = false;
  letzterTouchAktiv = false;
  touchGedrueckt = false;
  touchFehlkalibrierungTreffer = 0;

  Serial.println("Touch neu kalibriert");
  Serial.print("Touch-Ruhewert: ");
  Serial.println(touchRuhewert);
  Serial.print("Touch Vorwarn Schwelle: ");
  Serial.println(touchVorwarnSchwelle);
  Serial.print("Touch AN Schwelle: ");
  Serial.println(touchAnSchwelle);
  Serial.print("Touch AUS Schwelle: ");
  Serial.println(touchAusSchwelle);
}

// Wird einmal im Setup aufgerufen.
void initialisiereTouch() {
  kalibriereTouch();
  touchSperreBis = millis() + 1200;
}

// Erkennt kurze und lange Beruehrungen und startet die passende Aktion.
void pruefeTouchAktionen() {
  if (millis() < touchSperreBis) {
    letzterTouchAktiv = false;
    touchAktiv = false;
    touchGedrueckt = false;
    return;
  }

  int touchWert = leseTouchGemittelt(1);
  touchLetzterWert = touchWert;

  // Wenn der gespeicherte Ruhewert offensichtlich nicht zu den echten Messwerten passt,
  // kalibrieren wir automatisch neu. Genau das ist in deinem Log passiert:
  // Ruhewert ~50k, echte Werte aber ~22.7k.
  if (!touchGedrueckt && !touchAktiv) {
    if (touchRuhewert - touchWert > 8000) {
      touchFehlkalibrierungTreffer++;
    } else {
      touchFehlkalibrierungTreffer = 0;
    }

    if (touchFehlkalibrierungTreffer >= 8) {
      Serial.println("Touch-Kalibrierung unplausibel, kalibriere neu...");
      kalibriereTouch();
      touchSperreBis = millis() + 800;
      return;
    }
  }

  static unsigned long letzteDebugAusgabe = 0;
  /*
  if (millis() - letzteDebugAusgabe > 250) {
    Serial.print("Touch aktuell: ");
    Serial.print(touchWert);
    Serial.print(" | Ruhe: ");
    Serial.print(touchRuhewert);
    Serial.print(" | Vorwarn: ");
    Serial.print(touchVorwarnSchwelle);
    Serial.print(" | AN: ");
    Serial.print(touchAnSchwelle);
    Serial.print(" | AUS: ");
    Serial.println(touchAusSchwelle);
    letzteDebugAusgabe = millis();
  }
*/
  if (!touchAktiv) {
    if (touchWert >= touchAnSchwelle) {
      touchTreffer++;
    } else if (touchWert >= touchVorwarnSchwelle) {
      touchTreffer = 1;
    } else {
      touchTreffer = 0;
    }

    if (touchTreffer >= touchTrefferZumStart) {
      touchAktiv = true;
      touchTreffer = 0;
      touchLoslassenTreffer = 0;
    }
  } else {
    if (touchWert <= touchAusSchwelle) {
      touchLoslassenTreffer++;
    } else {
      touchLoslassenTreffer = 0;
    }

    if (touchLoslassenTreffer >= touchTrefferZumLoslassen) {
      touchAktiv = false;
      touchLoslassenTreffer = 0;
    }
  }

  if (touchAktiv && !letzterTouchAktiv && millis() - letzteTouchAktion > debounceMs) {
    touchGedrueckt = true;
    touchStartZeit = millis();
    Serial.println("Touch erkannt: START");
  }

  if (!touchAktiv && letzterTouchAktiv && touchGedrueckt) {
    unsigned long beruehrDauer = millis() - touchStartZeit;
    touchGedrueckt = false;
    letzteTouchAktion = millis();

    Serial.print("Touch erkannt: ENDE, Dauer ms = ");
    Serial.println(beruehrDauer);

    if (beruehrDauer >= longPressMs) {
      Serial.println("Touch-Aktion: Sprachablauf");
      if (!requestRunning) {
        requestRunning = true;
        starteSprachAblauf();
        requestRunning = false;
      }
    } else {
      Serial.println("Touch-Aktion: Pomodoro");
      if (!timerRunning) {
        starteArbeitsphase();
      } else {
        Serial.println("Pomodoro lief schon, daher keine neue Arbeitsphase gestartet.");
      }
    }
  }

  letzterTouchAktiv = touchAktiv;
}