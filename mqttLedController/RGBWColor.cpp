#include "RGBWColor.h"

RGBWColor::RGBWColor(int r, int g, int b, int w) : r(r), g(g), b(b), w(w) {}

int RGBWColor::getR() {
    return r;
};

int RGBWColor::getG() {
    return g;
};

int RGBWColor::getB() {
    return b;
};

int RGBWColor::getW() {
    return w;
};

void RGBWColor::scaleToBrightness(int brightness) {
    r = r * 100 / brightness;
    g = g * 100 / brightness;
    b = b * 100 / brightness;
    w = w * 100 / brightness;
};
