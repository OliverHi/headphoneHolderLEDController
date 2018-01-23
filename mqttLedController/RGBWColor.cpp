#include "RGBWColor.h"

RGBWColor::RGBWColor(int r, int g, int b, int w) {
    this->r = r;
    this->g = g;
    this->b = b;
    this->w = w;
}

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

RGBWColor RGBWColor::scaledToBrightness(int brightness) {
    return RGBWColor(r * 100 / brightness, g * 100 / brightness, b * 100 / brightness, w * 100 / brightness);
};
