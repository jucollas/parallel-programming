#include "../../include/engines/multithreadEngine.h"
#include <algorithm>
#include <iostream>

void* MultithreadEngine::threadFunc(void* arg) {
    ThreadData* data = (ThreadData*) arg;
 
    int kSize = data->filter->size;
    int kHalf = kSize / 2;

    for (int y = data->startY; y < data->endY; ++y) {
        for (int x = data->startX; x < data->endX; ++x) {
            Pixel* acc = nullptr;

            for (int ky = 0; ky < kSize; ++ky) {
                for (int kx = 0; kx < kSize; ++kx) {
                    int px = std::clamp(x + kx - kHalf, 0, data->width - 1);
                    int py = std::clamp(y + ky - kHalf, 0, data->height - 1);

                    Pixel* p = data->input[py * data->width + px];
                    Pixel* weighted = (*p) * data->filter->kernel[ky * kSize + kx];

                    if (!acc) {
                        acc = weighted;
                    } else {
                        Pixel* tmp = (*acc) + (*weighted);
                        delete acc;
                        delete weighted;
                        acc = tmp;
                    }
                }
            }

            Pixel* finalPixel = (*acc) / data->filter->scalar;
            delete acc;
            data->output[y * data->width + x] = finalPixel;
        }
    }

    return nullptr;
}

ImageFile* MultithreadEngine::applyFilter(ImageFile* file, const Filter* filter) {
    int width = file->getWidth();
    int height = file->getHeight();
    Pixel** input = file->getData();

    // Crear arreglo de salida
    Pixel** output = new Pixel*[width * height];

    // Crear hilos
    pthread_t threads[4];
    ThreadData threadData[4];

    int midX = width / 2;
    int midY = height / 2;

    // Arriba-izquierda
    threadData[0] = {input, output, 0, 0, midX, midY, width, height, filter};
    // Arriba-derecha
    threadData[1] = {input, output, midX, 0, width, midY, width, height, filter};
    // Abajo-izquierda
    threadData[2] = {input, output, 0, midY, midX, height, width, height, filter};
    // Abajo-derecha
    threadData[3] = {input, output, midX, midY, width, height, width, height, filter};

    for (int i = 0; i < 4; ++i) {
        pthread_create(&threads[i], nullptr, threadFunc, &threadData[i]);
    }

    for (int i = 0; i < 4; ++i) {
        pthread_join(threads[i], nullptr);
    }

    // Crear nueva imagen del mismo tipo
    ImageFile* out = file->createEmpty();
    out->setData(width, height, output);

    return out;
}
