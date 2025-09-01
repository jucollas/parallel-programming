#include <iostream>
#include <string>
#include <chrono>
#include <ctime>

#include "../include/base/processor.h"
#include "../include/engines/multithreadEngine.h"

int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cerr << "Use: " << argv[0]
                  << " <input_file> <output_file> --f <filter>\n";
        std::cerr << "Example: " << argv[0]
                  << " fruit.ppm fruit_blur.ppm --f blur\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = argv[2];
    std::string flag = argv[3];
    std::string filter = argv[4];

    if (flag != "--f") {
        std::cerr << "Error: Flag expected '--f'\n";
        return 1;
    }

    try {
        // Medir tiempo antes de ejecutar
        auto start_wall = std::chrono::high_resolution_clock::now();
        std::clock_t start_cpu = std::clock();

        Processor p(input_file, output_file, filter, new MultithreadEngine());
        p.execute();

        // Medir tiempo después de ejecutar
        std::clock_t end_cpu = std::clock();
        auto end_wall = std::chrono::high_resolution_clock::now();

        // Calcular tiempos
        double cpu_time = double(end_cpu - start_cpu) / CLOCKS_PER_SEC;
        double wall_time = std::chrono::duration<double>(end_wall - start_wall).count();

        std::cout << "CPU time used (sum over all threads): " << cpu_time << " seconds\n";
        std::cout << "Total execution time (wall-clock): " << wall_time << " seconds\n";

    } catch (const std::exception &e) {
        std::cerr << "Error during processing: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
