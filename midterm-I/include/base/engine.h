#ifndef ENGINE_CLASS
#define ENGINE_CLASS

#include "../filters.h"
#include "imageFile.h"

class Engine {
public:
    virtual ~Engine() = default;
    virtual ImageFile* applyFilter(ImageFile* file, const Filter* filter) = 0;
};

#endif
