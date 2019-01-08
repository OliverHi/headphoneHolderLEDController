#include "StaticColor.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

void StaticColor::updateToStep(int step) {
    for (int i = 0; i < _strip.numPixels(); i++) {
        _strip.setPixelColor(i, getScaledColor(_color.getR(), _color.getG(), _color.getB(), _color.getW()));
    }
}