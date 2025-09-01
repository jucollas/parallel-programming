#ifndef MULTITHREAD_ENGINE_CLASS
#define MULTITHREAD_ENGINE_CLASS

#include "../base/engine.h"
#include "../base/imageFile.h"
#include "../base/pixel.h"
#include <pthread.h>

class MultithreadEngine : public Engine {
public:
    ImageFile* applyFilter(ImageFile* file, Filter* filter) override;

private:
    struct ThreadData {
        Pixel** input;
        Pixel** output;
        int startX, startY;
        int endX, endY;
        int width, height;
        const Filter* filter;
    };

    static void* threadFunc(void* arg);
};

#endif // MULTITHREAD_ENGINE_CLASS
