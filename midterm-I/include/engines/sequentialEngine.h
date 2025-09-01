#ifndef SEQUENTIAL_ENGINE_CLASS
#define SEQUENTIAL_ENGINE_CLASS

#include "../base/engine.h"
#include "../base/imageFile.h"
#include "../base/pixel.h"

class SequentialEngine : public Engine {
public:
    ImageFile* applyFilter(ImageFile* file, const Filter* filter) override;
};

#endif // SEQUENTIAL_ENGINE_CLASS
