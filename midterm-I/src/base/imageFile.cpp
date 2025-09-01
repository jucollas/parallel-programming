#include "../../include/base/imageFile.h"

// Destructor virtual definido explícitamente para que se genere la vtable
ImageFile::~ImageFile() {
    // vacío, no necesita hacer nada
}

// Implementación por defecto de setData
void ImageFile::setData(int w, int h, Pixel** pixels) {
    width = w;
    height = h;
    // El arreglo pixels será manejado por la clase derivada (PGMFile, PPMFile)
}
