#include "face.h"

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

#ifdef DEFAULT
#undef DEFAULT
#endif

#include <FluxGarage_RoboEyes.h>

#undef N
#undef E
#undef S
#undef W
#undef NE
#undef NW
#undef SE
#undef SW

extern Adafruit_SSD1306 display;

static RoboEyes<Adafruit_SSD1306> roboEyes(display);

void initialisiereGesicht() {
  roboEyes.begin(128, 64, 100);
  roboEyes.setAutoblinker(ON, 3, 2);
  roboEyes.setIdleMode(ON, 2, 2);
  roboEyes.setWidth(36, 36);
  roboEyes.setHeight(36, 36);
  roboEyes.setBorderradius(8, 8);
  roboEyes.setSpacebetween(15);
  roboEyes.setMood(DEFAULT);
  roboEyes.setCuriosity(OFF);
}

void updateGesicht() {
  roboEyes.update();
}