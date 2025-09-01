#include "../../include/base/map.h"

Map::Map()
    : width(0), height(0), pixels(nullptr) {}

Map::~Map() {
    int n = width * height;
    for (int i = 0; i < n; i++) {
        delete pixels[i];   
    }
    delete[] pixels;
}


Pixel* Map::getPixel(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        throw std::out_of_range("Coordinates out of range in getPixel");
    }
    return pixels[y * width + x];
}

// Reemplazar un pixel
void Map::setPixel(int x, int y, Pixel* p) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        throw std::out_of_range("Coordinates out of range in setPixel");
    }
    int idx = y * width + x;
    delete pixels[idx];
    pixels[idx] = p;
}

// Obtener todos los pixeles
Pixel** Map::getAllPixel() const {
    return pixels;
}

int Map::getWidth() const { return width; }
int Map::getHeight() const { return height; }


void Map::setData(int w, int h, Pixel** data) {
    // Liberar datos viejos si existen
    if (pixels) {
        for (int i = 0; i < width * height; ++i) {
            delete pixels[i];
        }
        delete[] pixels;
    }

    width = w;
    height = h;
    pixels = data;
}
