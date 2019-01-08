#ifndef STATICCOLOR
#define STATICCOLOR

#include "Animation.h"
#include <Adafruit_NeoPixel.h>

class StaticColor : public Animation {
	public:
		StaticColor(unsigned long patternInterval, Adafruit_NeoPixel& strip, int maxSteps, int initialBrightness) : Animation(patternInterval, strip, maxSteps, initialBrightness) {};

	protected:
        void updateToStep(int step);
};

#endif