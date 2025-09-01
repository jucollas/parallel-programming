#include <iostream>
#include <cstdio>   // fopen, fscanf, fclose
#include <cstdlib>  // malloc, free

#include "../../include/images/pgmFile.h"
#include "../../include/pixel/grayPixel.h"

bool PGMFile::load(const std::string& filename) {
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
    int* data = (int*) malloc(pixel_count * sizeof(int));
    if (!data) {
        std::cerr << "Memory allocation error." << std::endl;
        fclose(file);
        return false;
    }

    for (int i = 0; i < pixel_count; i++) {
        if (fscanf(file, "%d", &data[i]) != 1) {
            std::cerr << "Error reading pixels." << std::endl;
            free(data);
            fclose(file);
            return false;
        }
    }

    fclose(file);

    Pixel** pixels = new Pixel*[pixel_count];
    for (int i = 0; i < pixel_count; ++i) {
        pixels[i] = new GrayPixel(data[i]);
    }
    gray_data.setData(width, height, pixels);
    free(data); 
    return true;
}

bool PGMFile::save(const std::string& filename) const {
    FILE* file = fopen(filename.c_str(), "w");
    if (file == nullptr) {
        std::cerr << "Error: could not open file for writing." << std::endl;
        return false;
    }

    fprintf(file, "%s\n%d %d\n%d\n", magic.c_str(), width, height, max_value);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Pixel* p = gray_data.getPixel(x, y);
            fprintf(file, "%d\n", p->getChannel(0));
        }
    }

    fclose(file);
    return true;
}

Pixel** PGMFile::getData() const {
    return gray_data.getAllPixel();
}

ImageFile* PGMFile::createEmpty() const {
    PGMFile* ans = new PGMFile();
    ans->setMagic(this->magic);
    return (ImageFile*) ans;
}

void PGMFile::setMagic(std::string magic){
    this->magic = magic;
}



void PGMFile::setData(int w, int h, Pixel** pixels) {
    for (int i = 0; i < this->width * this->height; i++) {
        delete gray_data.getAllPixel()[i];
    }

    width = w;
    height = h;

    gray_data.setData(w, h, pixels); // tu Map puede almacenar Pixel**
}

