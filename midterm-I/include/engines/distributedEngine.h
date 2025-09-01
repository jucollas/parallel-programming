#ifndef DISTRIBUTED_ENGINE
#define DISTRIBUTED_ENGINE

#include "../base/engine.h"
#include "sequentialEngine.h"
#include <vector>
#include <iostream>
#include <mpi.h>


class DistributedEngine : public Engine {
public:
    // Recibe un vector de filtros y devuelve un vector de imágenes filtradas en el rank 0
    std::vector<ImageFile*> applyFilters(ImageFile* file, const std::vector<const Filter*>& filters) const ;
    ImageFile* applyFilter(ImageFile* file, const Filter* filter) override;
};

#endif
