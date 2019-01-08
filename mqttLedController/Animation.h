#ifndef ANIMATION
#define ANIMATION

#include <Adafruit_NeoPixel.h>
#include "RGBWColor.h"

class Animation {
	public:
		Animation(unsigned long patternInterval, Adafruit_NeoPixel& strip, int maxSteps, int initialBrightness);
		unsigned long getPatternInterval();
		int getMaxSteps();
    void update();
		void setBrightness(int brightness);
		void setColor(RGBWColor color);

	protected:
		unsigned long _patternInterval;	
    unsigned long _lastUpdate;
		int _step;
		int _brightness;
		const int MAX_STEPS;
		Adafruit_NeoPixel _strip;
		RGBWColor _color;
		uint32_t getScaledColor(int newR, int newG, int newB, int newW);
    virtual void updateToStep(int step) = 0;
};

#endif