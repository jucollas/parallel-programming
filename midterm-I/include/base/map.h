#ifndef MAP_CLASS
#define MAP_CLASS

#include "pixel.h" 
#include <stdexcept>

class Map {
private:
    int width;
    int height;
    Pixel** pixels;

public:
    Map();
    ~Map();

    int getWidth() const;
    int getHeight() const;
    Pixel* getPixel(int x, int y) const;
    void setPixel(int x, int y, Pixel* p);
    Pixel** getAllPixel() const;
    void setData(int w, int h, Pixel** data);

};

#endif // MAP_CLASS
