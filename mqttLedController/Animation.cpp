#include "Animation.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "RGBWColor.h"

Animation::Animation(unsigned long patternInterval, Adafruit_NeoPixel& strip, int maxSteps, int initialBrightness) :
  _patternInterval(patternInterval),
  _lastUpdate(0),
  _strip(strip),
  MAX_STEPS(maxSteps),
  _step(0),
  _brightness(initialBrightness),
  _color(RGBWColor(0,0,0,255)) // initial color white
{}

unsigned long Animation::getPatternInterval() {
    return _patternInterval;
}

int Animation::getMaxSteps() {
    return MAX_STEPS;
}

void Animation::update() {
    if (millis() - _lastUpdate > _patternInterval) {
        _step++;
        if (_step >= MAX_STEPS) {
            _step = 0;
        }
        updateToStep(_step);
        _lastUpdate = millis();
        _strip.show();
    }
}

void Animation::setBrightness(int brightness) {
    _brightness = brightness;
};

uint32_t Animation::getScaledColor(int newR, int newG, int newB, int newW) {
    return _strip.Color(
            newR / 100 * _brightness,
            newG / 100 * _brightness,
            newB / 100 * _brightness,
            newW / 100 * _brightness
    );
}

void Animation::setColor(RGBWColor color) {
    _color = color;    
};
