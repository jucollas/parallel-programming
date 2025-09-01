#include "../../include/engines/sequentialEngine.h"
#include <algorithm>
#include <iostream>

ImageFile* SequentialEngine::applyFilter(ImageFile* file, Filter* filter) {
    int width = file->getWidth();
    int height = file->getHeight();
    Pixel** data = file->getData();

    // Crear arreglo de salida
    Pixel** newData = new Pixel*[width * height];

    int kSize = filter->size;
    int kHalf = kSize / 2;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Pixel* acc = nullptr; // acumulador de píxel

            for (int ky = 0; ky < kSize; ++ky) {
                for (int kx = 0; kx < kSize; ++kx) {
                    int px = std::clamp(x + kx - kHalf, 0, width - 1);
                    int py = std::clamp(y + ky - kHalf, 0, height - 1);

                    Pixel* p = data[py * width + px];

                    // multiplicar por el valor de la matriz
                    Pixel* weighted = (*p) * filter->kernel[ky * kSize + kx];

                    if (acc == nullptr) {
                        acc = weighted; // primer pixel
                    } else {
                        Pixel* tmp = (*acc) + (*weighted); // sumar
                        delete acc;
                        delete weighted;
                        acc = tmp;
                    }
                }
            }

            // dividir por el escalar
            Pixel* finalPixel = (*acc) / filter->scalar;
            delete acc;

            newData[y * width + x] = finalPixel;
        }
    }

    // Crear una nueva ImageFile del mismo tipo (PGM/PPM)
    //ImageFile* out = nullptr;

    // Aquí podrías detectar dinámicamente el tipo de imagen
    // Si sabes que siempre es PGM, simplemente:


    ImageFile* out = file->createEmpty();
    //std::cout << "amamam\n";

    out->setData(width, height, newData);

    return out;
}
