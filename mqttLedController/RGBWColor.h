class RGBWColor {
    public:
        RGBWColor(int r, int g, int b, int w);
        int getR();
        int getG();
        int getB();
        int getW();
        RGBWColor scaledToBrightness(int brightness0);

    private:
        int r;
        int g;
        int b;
        int w;
};