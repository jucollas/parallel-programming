#include "../../include/base/pixel.h"
#include "../../include/pixel/grayPixel.h"
#include "../../include/pixel/RGBPixel.h"

Pixel* Pixel::cloneFromBuffer(const int* buffer, bool isGray) {
    if (isGray) {
        return new GrayPixel(buffer[0]);
    } else {
        return new RGBPixel(buffer[0], buffer[1], buffer[2]);
    }
}
