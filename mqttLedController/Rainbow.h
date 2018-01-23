#include "Arduino.h"
#include "Animation.h"
#include <Adafruit_NeoPixel.h>

class Rainbow : public Animation {
	public:
		Rainbow(unsigned long patternInterval, Adafruit_NeoPixel& strip, int maxSteps) : Animation(patternInterval, strip, maxSteps) {};

	protected:
        void updateToStep(int step);

  private:
        uint32_t Wheel(byte WheelPos);
};
