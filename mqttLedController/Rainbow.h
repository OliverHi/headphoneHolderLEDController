#ifndef RAINBOW
#define RAINBOW

#include "Animation.h"
#include <Adafruit_NeoPixel.h>

class Rainbow : public Animation {
	public:
		Rainbow(unsigned long patternInterval, Adafruit_NeoPixel& strip, int maxSteps, int initialBrightness) : Animation(patternInterval, strip, maxSteps, initialBrightness) {};

	protected:
        void updateToStep(int step);

  private:
        uint32_t Wheel(byte WheelPos);
};

#endif