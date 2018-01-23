#include <Adafruit_NeoPixel.h>

class Animation {
	public:
		Animation(unsigned long patternInterval, Adafruit_NeoPixel& strip, int maxSteps);
		unsigned long getPatternInterval();
		int getMaxSteps();
    void update();

	protected:
		unsigned long _patternInterval;	
    unsigned long _lastUpdate;
		int _step;
		const int MAX_STEPS;
		Adafruit_NeoPixel _strip;
    virtual void updateToStep(int step) = 0;
};
