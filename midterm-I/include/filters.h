#ifndef FILTERS_H
#define FILTERS_H

struct Filter {
    const float* kernel;
    int size;
    float scalar;
};

// Declaración de filtros
extern const Filter Blur;
extern const Filter Laplace;
extern const Filter Sharpening;

#endif // FILTERS_H
