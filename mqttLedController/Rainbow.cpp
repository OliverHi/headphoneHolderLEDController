#include "Rainbow.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

void Rainbow::updateToStep(int step) {
    for (int i = 0; i < _strip.numPixels(); i++) {
        _strip.setPixelColor(i, Wheel((i + step) & 255));
    }
}

uint32 Rainbow::Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        int wheelR = 255 - WheelPos * 3, wheelG = 0, wheelB = WheelPos * 3;
        return getScaledColor(wheelR, wheelG, wheelB, 0);
    }

    if (WheelPos < 170) {
        WheelPos -= 85;
        int wheelR = 0, wheelG = WheelPos * 3, wheelB = 255 - WheelPos * 3;
        return getScaledColor(wheelR, wheelG, wheelB, 0);
    }

    WheelPos -= 170;
    int wheelR = WheelPos * 3, wheelG = 255 - WheelPos * 3, wheelB = 0;
    return getScaledColor(wheelR, wheelG, wheelB, 0);
}