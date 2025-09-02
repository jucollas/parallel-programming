#include <iostream>
#include <string>
#include <vector>
#include <ctime>
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

    // Obtener el rank y size del nodo
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Medir tiempo antes de aplicar filtros
    double start_wall = MPI_Wtime();
    std::clock_t start_cpu = std::clock();

    // Aplicar filtros
    std::vector<ImageFile*> results = engine->applyFilters(file, filters);

    // Medir tiempo después de aplicar filtros
    std::clock_t end_cpu = std::clock();
    double end_wall = MPI_Wtime();

    // Calcular tiempos locales
    double cpu_time = double(end_cpu - start_cpu) / CLOCKS_PER_SEC;
    double wall_time = end_wall - start_wall;

    // Recolectar los tiempos de todos los nodos
    double local_times[2] = {cpu_time, wall_time};
    std::vector<double> all_times;
    if (rank == 0) {
        all_times.resize(size * 2);
    }

    MPI_Gather(local_times, 2, MPI_DOUBLE,
               all_times.data(), 2, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    // Nodo 0 guarda resultados y muestra tiempos
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

        // Mostrar tiempos de todos los nodos
        std::cout << "\n=== Tiempos de ejecución por nodo ===\n";
        for (int i = 0; i < size; i++) {
            double cpu = all_times[i * 2];
            double wall = all_times[i * 2 + 1];
            std::cout << "Rank " << i
                      << " -> CPU: " << cpu << " s, Wall: " << wall << " s\n";
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
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        if (rank == 0) {
            std::cerr << "Error during processing: " << e.what() << "\n";
        }
        MPI_Finalize();
        return 1;
    }

    MPI_Finalize();
    return 0;
}
