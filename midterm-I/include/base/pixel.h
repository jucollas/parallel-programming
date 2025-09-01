#ifndef PIXEL_H
#define PIXEL_H

class Pixel {
public:
    virtual ~Pixel() = default;

    virtual int getChannel(int c) const = 0;
    virtual void setChannel(int c, int value) = 0;

    virtual Pixel* operator+(const Pixel& other) const = 0;
    virtual Pixel* operator*(int scalar) const = 0;
    virtual Pixel* operator/(int scalar) const = 0;

    virtual void serialize(int* buffer) = 0;
    static Pixel* cloneFromBuffer(const int* buffer, bool isGray); 
};

#endif
