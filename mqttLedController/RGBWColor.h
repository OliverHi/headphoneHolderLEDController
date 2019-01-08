#ifndef RGBWCOLOR
#define RGBWCOLOR

#include <Arduino.h>

class RGBWColor {
    public:
        RGBWColor(int r, int g, int b, int w);
        int getR();
        int getG();
        int getB();
        int getW();
        void scaleToBrightness(int brightness);

    private:
        int r;
        int g;
        int b;
        int w;
};

#endif