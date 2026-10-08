#include "pomodoro.h"

#include <Arduino.h>
#include "display.h"

extern bool timerRunning;
extern bool arbeitsphase;
extern unsigned long startTime;
extern unsigned long duration;
extern unsigned long arbeitsDauer;
extern unsigned long pausenDauer;
extern unsigned long elapsedTime;
extern unsigned long remainingTime;

extern bool stehPhase;
extern unsigned long stehSitzStartZeit;
extern unsigned long sitzDauer;
extern unsigned long stehDauer;

// Startet eine neue Fokusphase.
void starteArbeitsphase() {
  timerRunning = true;
  arbeitsphase = true;
  duration = arbeitsDauer;
  startTime = millis();
  remainingTime = duration;
}

// Wechselt in die Pause.
void startePause() {
  timerRunning = true;
  arbeitsphase = false;
  duration = pausenDauer;
  startTime = millis();
  remainingTime = duration;
}

// Rechnet die Restzeit herunter und wechselt bei Bedarf die Phase.
void aktualisiereTimer() {
  if (!timerRunning) {
    return;
  }

  elapsedTime = millis() - startTime;

  if (elapsedTime >= duration) {
    if (arbeitsphase) {
      startePause();
      zeigeNachricht("BREAK", "TIME <3");
    } else {
      timerRunning = false;
      remainingTime = 0;
      zeigeNachricht("GOOD", "JOB <3");
    }
  } else {
    remainingTime = duration - elapsedTime;
  }
}

// Erinnert ans Aufstehen bzw. Hinsetzen.
void pruefeSitzStehPhase() {
  unsigned long vergangeneZeit = millis() - stehSitzStartZeit;

  if (!stehPhase && vergangeneZeit >= sitzDauer) {
    stehPhase = true;
    stehSitzStartZeit = millis();
    Serial.println("Bitte aufstehen / Tisch hochfahren");
    zeigeNachricht("BITTE", "AUFSTEHEN");
  } else if (stehPhase && vergangeneZeit >= stehDauer) {
    stehPhase = false;
    stehSitzStartZeit = millis();
    Serial.println("Bitte wieder hinsetzen");
    zeigeNachricht("BITTE", "HINSETZEN");
  }
}