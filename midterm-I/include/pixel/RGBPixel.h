#ifndef RGB_PIXEL_H
#define RGB_PIXEL_H

#include "../base/pixel.h"

class RGBPixel : public Pixel {
private:
    int r, g, b;

public:
    RGBPixel(int rr = 0, int gg = 0, int bb = 0);

    int getChannel(int c) const override;
    void setChannel(int c, int value) override;

    Pixel* operator+(const Pixel& other) const override;
    Pixel* operator*(int scalar) const override;
    Pixel* operator/(int scalar) const override;

    void serialize(int* buffer) override;
    //Pixel* cloneFromBuffer(const int* buffer) const override; 
};

#endif // RGB_PIXEL_H
