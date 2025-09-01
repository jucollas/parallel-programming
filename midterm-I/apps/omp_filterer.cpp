#include <iostream>
#include <string>
#include <omp.h>
#include <vector>

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

    // Definición correcta de arreglos de punteros
    const Filter* filters[3] = { &Blur, &Laplace, &Sharpening };
    const char* names_filters[3] = { "blur", "laplace", "sharpening" };

    std::string prefix = output_file.substr(0, output_file.size() - 5); 

    #pragma omp parallel for
    for (int i = 0; i < 3; ++i) {
        ImageFile* out_file = engine->applyFilter(file, filters[i]);
        std::string out_name = prefix + "_" +std::string(names_filters[i]) + ext;

        if (!out_file->save(out_name)) {
            std::cerr << "Failed to save file: " << out_name << std::endl;
        }
        delete out_file;
    }

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
