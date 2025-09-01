#include "../include/filters.h"

// Definición de kernels
static const float blurKernel[9] = {
    1,1,1,
    1,1,1,
    1,1,1
};

static const float laplaceKernel[9] = {
     0,-1, 0,
    -1, 4,-1,
     0,-1, 0
};

static const float sharpeningKernel[9] = {
     0,-1, 0,
    -1, 5,-1,
     0,-1, 0
};

// Filtros
const Filter Blur       = { blurKernel, 3, 9.0f };
const Filter Laplace    = { laplaceKernel, 3, 1.0f };
const Filter Sharpening = { sharpeningKernel, 3, 1.0f };
