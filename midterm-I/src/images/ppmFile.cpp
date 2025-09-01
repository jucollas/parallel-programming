#include <iostream>
#include <cstdio>   // fopen, fscanf, fclose
#include <cstdlib>  // malloc, free

#include "../../include/images/ppmFile.h"
#include "../../include/pixel/RGBPixel.h"

bool PPMFile::load(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "r");
    if (file == nullptr) {
        std::cerr << "Error: incorrect path or file not found." << std::endl;
        return false;
    }

    char magic_buf[3];
    if (fscanf(file, "%2s", magic_buf) != 1) {
        std::cerr << "Error reading magic number." << std::endl;
        fclose(file);
        return false;
    }
    magic = magic_buf;

    if (fscanf(file, "%d %d", &width, &height) != 2) {
        std::cerr << "Error reading width/height." << std::endl;
        fclose(file);
        return false;
    }

    if (fscanf(file, "%d", &max_value) != 1) {
        std::cerr << "Error reading max value." << std::endl;
        fclose(file);
        return false;
    }

    int pixel_count = width * height;
    int pixel_count_file = pixel_count * 3;
    int* data = (int*) malloc(pixel_count_file * sizeof(int));
    if (!data) {
        std::cerr << "Memory allocation error." << std::endl;
        fclose(file);
        return false;
    }

    for (int i = 0; i < pixel_count_file; i++) {
        if (fscanf(file, "%d", &data[i]) != 1) {
            std::cerr << "Error reading pixels." << std::endl;
            free(data);
            fclose(file);
            return false;
        }
    }

    fclose(file);

    Pixel** pixels = new Pixel*[pixel_count];
    int index = 0;
    for (int i = 0; i < pixel_count_file; i += 3) {
        pixels[index++] = new RGBPixel(data[i], data[i + 1], data[i + 2]);
    }

    rgb_data.setData(width, height, pixels);

    free(data); 
    return true;
}

bool PPMFile::save(const std::string& filename) const {
    FILE* file = fopen(filename.c_str(), "w");
    if (file == nullptr) {
        std::cerr << "Error: could not open file for writing." << std::endl;
        return false;
    }

    fprintf(file, "%s\n%d %d\n%d\n", magic.c_str(), width, height, max_value);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Pixel* p = rgb_data.getPixel(x, y);
            fprintf(file, "%d\n%d\n%d\n", p->getChannel(0), p->getChannel(1), p->getChannel(2));
        }
    }

    fclose(file);
    return true;
}

Pixel** PPMFile::getData() const {
    return rgb_data.getAllPixel();
}

ImageFile* PPMFile::createEmpty() const {
    PPMFile* ans = new PPMFile();
    ans->setMagic(this->magic);
    return (ImageFile*) ans;
}

void PPMFile::setMagic(std::string magic){
    this->magic = magic;
}

void PPMFile::setData(int w, int h, Pixel** pixels) {
    for (int i = 0; i < this->width * this->height; i++) {
        delete rgb_data.getAllPixel()[i];
    }

    width = w;
    height = h;

    rgb_data.setData(w, h, pixels); // tu Map puede almacenar Pixel**
}


