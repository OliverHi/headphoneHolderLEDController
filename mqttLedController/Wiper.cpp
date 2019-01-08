#include "StaticColor.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

void Wiper::updateToStep(int step) {

    int ledsPerStep = _strip.numPixels / MAX_STEPS;

    for (int i = 0; i < _strip.numPixels(); i++) {
        _strip.setPixelColor(i, getScaledColor(_color.getR(), _color.getG(), _color.getB(), _color.getW()));
    }
}

void colorWipe(uint32_t c) { // modified from Adafruit example to make it a state machine
    static int i = 0;
    strip.setPixelColor(i, c);
    strip.show();
    i++;
    if (i >= strip.numPixels()) {
        i = 0;
        wipe(); // blank out strip
    }
    lastUpdate = millis(); // time for next change to the display
}