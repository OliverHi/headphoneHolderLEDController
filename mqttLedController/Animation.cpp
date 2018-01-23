#include "Animation.h"
#include <Adafruit_NeoPixel.h>
#include "Arduino.h"

Animation::Animation(unsigned long patternInterval, Adafruit_NeoPixel& strip, int maxSteps) :
  _patternInterval(patternInterval),
  _strip(strip),
  MAX_STEPS(maxSteps),
  _step(0)
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
