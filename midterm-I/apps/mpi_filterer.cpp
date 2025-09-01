#include <iostream>
#include <string>
#include <vector>
#include <mpi.h>

#include "../include/base/imageFile.h"
#include "../include/base/engine.h"
#include "../include/engines/distributedEngine.h"
#include "../include/filters.h"
#include "../include/utils.h"
#include "../include/images/pgmFile.h"
#include "../include/images/ppmFile.h"

void execute_all_mpi(std::string input_file, std::string output_file, DistributedEngine* engine) {
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

    // Vector con todos los filtros y nombres
    std::vector<const Filter*> filters = { &Blur, &Laplace, &Sharpening };
    std::vector<std::string> names = { "blur", "laplace", "sharpening" };

    // Llamar a applyFilters de DistributedEngine
    std::vector<ImageFile*> results = engine->applyFilters(file, filters);
    std::cout << results.size() << "\n";

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        // Guardar resultados en archivos
        std::string prefix = output_file.substr(0, output_file.size() - ext.size());
        for (size_t i = 0; i < results.size(); ++i) {
            std::string out_name = prefix + "_" + names[i] + ext;
            if (!results[i]->save(out_name)) {
                std::cerr << "Failed to save file: " << out_name << std::endl;
            }
            delete results[i];
        }
    }

    delete file;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    if (argc < 3) {
        std::cerr << "Use: " << argv[0]
                  << " <input_file> <output_file>\n";
        std::cerr << "Example: " << argv[0]
                  << " fruit.ppm fruit_blur.ppm\n";
        MPI_Finalize();
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];

    try {
        DistributedEngine* engine = new DistributedEngine();
        execute_all_mpi(input_file, output_file, engine);
        delete engine;
    } catch (const std::exception &e) {
        std::cerr << "Error during processing: " << e.what() << "\n";
        MPI_Finalize();
        return 1;
    }

    MPI_Finalize();
    return 0;
}
