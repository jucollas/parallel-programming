#include <iostream>
#include <string>

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
        Processor p(input_file, output_file, filter, new MultithreadEngine());
        p.execute();
    } catch (const std::exception &e) {
        std::cerr << "Error during processing: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
