#include <iostream>
#include <string>
#include <omp.h>
#include <vector>
#include <chrono>
#include <ctime>

#include "../include/base/imageFile.h"
#include "../include/base/engine.h"
#include "../include/engines/sequentialEngine.h"
#include "../include/filters.h"
#include "../include/utils.h"
#include "../include/images/pgmFile.h"
#include "../include/images/ppmFile.h"

void execute_all_omp(std::string input_file, std::string output_file, Engine* engine) {
    ImageFile* file = nullptr;

    if (endsWith(input_file, ".pgm")) {
        file = new PGMFile();
    } else if (endsWith(input_file, ".ppm")) {
        file = new PPMFile();
    } else {
        std::cerr << "Unrecognized file format" << std::endl;
        return;
    }

    if (!file->load(input_file)) {
        std::cerr << "Failed to load file: " << input_file << std::endl;
        delete file;
        return;
    }

    std::string ext = endsWith(input_file, ".pgm") ? ".pgm" : ".ppm";

    const Filter* filters[3] = { &Blur, &Laplace, &Sharpening };
    const char* names_filters[3] = { "blur", "laplace", "sharpening" };

    std::string prefix = output_file.substr(0, output_file.size() - ext.size());

    // Medir tiempos antes de aplicar filtros
    auto start_wall = std::chrono::high_resolution_clock::now();
    std::clock_t start_cpu = std::clock();

    #pragma omp parallel for
    for (int i = 0; i < 3; ++i) {
        ImageFile* out_file = engine->applyFilter(file, filters[i]);
        std::string out_name = prefix + "_" + std::string(names_filters[i]) + ext;

        if (!out_file->save(out_name)) {
            std::cerr << "Failed to save file: " << out_name << std::endl;
        }
        delete out_file;
    }

    // Medir tiempos después de aplicar filtros
    std::clock_t end_cpu = std::clock();
    auto end_wall = std::chrono::high_resolution_clock::now();

    // Mostrar tiempos
    double cpu_time = double(end_cpu - start_cpu) / CLOCKS_PER_SEC;
    double wall_time = std::chrono::duration<double>(end_wall - start_wall).count();

    std::cout << "CPU time used (sum over all threads): " << cpu_time << " seconds\n";
    std::cout << "Total execution time (wall-clock): " << wall_time << " seconds\n";

    delete file;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Use: " << argv[0]
                  << " <input_file> <output_file>\n";
        std::cerr << "Example: " << argv[0]
                  << " fruit.ppm fruit_blur.ppm\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    try {
        execute_all_omp(input_file, output_file, new SequentialEngine());
    } catch (const std::exception &e) {
        std::cerr << "Error during processing: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
