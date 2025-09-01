#ifndef GRAY_PIXEL_H
#define GRAY_PIXEL_H

#include "../base/pixel.h"

class GrayPixel : public Pixel {
private:
    int gray;
public:
    GrayPixel(int g = 0);

    int getChannel(int c) const override;
    void setChannel(int c, int value) override;

    Pixel* operator+(const Pixel& other) const override;
    Pixel* operator*(int scalar) const override;
    Pixel* operator/(int scalar) const override;
};

#endif // GRAY_PIXEL_H
