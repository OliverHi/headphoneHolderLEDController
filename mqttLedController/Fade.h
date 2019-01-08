#ifndef FADE
#define FADE

#include "Animation.h"
#include <Adafruit_NeoPixel.h>

class Fade : public Animation {
	public:
		Fade(unsigned long patternInterval, Adafruit_NeoPixel& strip, int maxSteps, int initialBrightness) : Animation(patternInterval, strip, maxSteps, initialBrightness) {};

	protected:
        void updateToStep(int step);
};

#endif