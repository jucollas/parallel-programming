#include "../../include/pixel/grayPixel.h"

GrayPixel::GrayPixel(int g) : gray(g) {}

int GrayPixel::getChannel(int) const {
    return gray;
}

void GrayPixel::setChannel(int, int value) {
    gray = value;
}

Pixel* GrayPixel::operator+(const Pixel& other) const {
    int sum = gray + other.getChannel(0);
    return new GrayPixel(sum);
}

Pixel* GrayPixel::operator*(int scalar) const {
    return new GrayPixel(gray * scalar);
}

Pixel* GrayPixel::operator/(int scalar) const{
  return new GrayPixel((int) gray / scalar);
}

