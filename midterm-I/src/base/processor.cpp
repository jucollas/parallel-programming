//#include <omp.h>
#include <vector>
#include <string>
#include <iostream>

#include "../../include/base/processor.h"
#include "../../include/base/imageFile.h"
#include "../../include/images/pgmFile.h"
#include "../../include/images/ppmFile.h"
#include "../../include/utils.h"
#include "../../include/filters.h"

Processor::Processor(std::string input_file, std::string output_file, std::string filter, Engine* engine)
: input_file(input_file), output_file(output_file), filter(filter), engine(engine) {}

Processor::~Processor(){
  delete this->engine;
}

void Processor::execute() {
    ImageFile* file = nullptr;

    if (endsWith(input_file, ".pgm")) {
        file = new PGMFile();
    } else if (endsWith(input_file, ".ppm")) {
        file = new PPMFile();
    } else {
        std::cerr << "Unrecognized file format" << std::endl;
        return;
    }

    // Cargar imagen
    if (!file->load(input_file)) {
        std::cerr << "Failed to load file: " << input_file << std::endl;
        delete file;
        return;
    }

    Filter fil;

    if (filter == "blur") {
      fil = Blur;
    } else if (filter == "laplace") {
      fil = Laplace;
    } else if (filter == "sharpening") {
      fil = Sharpening;
    } else {
      std::cerr << "Unknown filter: " << filter << std::endl;
      delete file;
      return;
    }


    // Aplicar filtro usando engine
    ImageFile *out_file = engine->applyFilter(file, &fil);



    if (!out_file->save(output_file)){
      std::cerr << "Failed to save file: " << output_file << std::endl;
    }

    delete file;
}
/*
void Processor::execute_all_omp() {
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

*/
