#ifndef WIPER
#define WIPER

#include "Animation.h"
#include <Adafruit_NeoPixel.h>

class Wiper : public Animation {
	public:
		Wiper(unsigned long patternInterval, Adafruit_NeoPixel& strip, int maxSteps, int initialBrightness) : Animation(patternInterval, strip, maxSteps, initialBrightness) {};

	protected:
        void updateToStep(int step);
};

#endif