#include "Fade.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

void Fade::updateToStep(int step) {
    int halfOfSteps = MAX_STEPS / 2;

    if (_step <= halfOfSteps) {
        // fade up
        RGBWColor fadedColor = RGBWColor(
            _color.getR() * _step / halfOfSteps,
            _color.getG() * _step / halfOfSteps,
            _color.getB() * _step / halfOfSteps,
            _color.getW() * _step / halfOfSteps
        );
        
        fadedColor.scaleToBrightness(_brightness);

        for (int i = 0; i < _strip.numPixels(); i++) {
            _strip.setPixelColor(i, _strip.Color(fadedColor.getR(), fadedColor.getG(), fadedColor.getB(), fadedColor.getW()));
        }
    } else {
        // fade down
        RGBWColor fadedColor = RGBWColor(
            _color.getR() * (MAX_STEPS - _step) / halfOfSteps,
            _color.getG() * (MAX_STEPS - _step) / halfOfSteps,
            _color.getB() * (MAX_STEPS - _step) / halfOfSteps,
            _color.getW() * (MAX_STEPS - _step) / halfOfSteps
        );
        
        fadedColor.scaleToBrightness(_brightness);

        for (int i = 0; i < _strip.numPixels(); i++) {
            _strip.setPixelColor(i, _strip.Color(fadedColor.getR(), fadedColor.getG(), fadedColor.getB(), fadedColor.getW()));
        }
    }
}
