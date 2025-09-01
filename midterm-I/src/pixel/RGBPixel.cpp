#include "../../include/pixel/RGBPixel.h"

RGBPixel::RGBPixel(int rr, int gg, int bb) : r(rr), g(gg), b(bb) {}

int RGBPixel::getChannel(int c) const {
    if (c == 0) return r;
    if (c == 1) return g;
    return b;
}

void RGBPixel::setChannel(int c, int value) {
    if (c == 0) r = value;
    else if (c == 1) g = value;
    else b = value;
}

Pixel* RGBPixel::operator+(const Pixel& other) const {
    int rr = r + other.getChannel(0);
    int gg = g + other.getChannel(1);
    int bb = b + other.getChannel(2);
    return new RGBPixel(rr, gg, bb);
}

Pixel* RGBPixel::operator*(int scalar) const {
    return new RGBPixel(r * scalar, g * scalar, b * scalar);
}

Pixel* RGBPixel::operator/(int scalar) const{
  return new RGBPixel((int) r / scalar, (int) g * scalar, (int) b * scalar);
}


void RGBPixel::serialize(int* buffer){
    buffer[0] = r;
    buffer[1] = g;
    buffer[2] = b;
}

/*Pixel* RGBPixel::cloneFromBuffer(const int* buffer) const {
    return new RGBPixel(buffer[0], buffer[1], buffer[2]);
}*/
