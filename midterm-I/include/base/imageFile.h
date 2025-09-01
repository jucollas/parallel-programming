#ifndef IMAGE_FILE_CLASS
#define IMAGE_FILE_CLASS

#include <string>
#include "pixel.h"

class ImageFile {
protected:
    int width = 0;
    int height = 0;
    int max_value = 255;

public:
    virtual ~ImageFile();

    virtual bool load(const std::string& filename) = 0;
    virtual bool save(const std::string& filename) const = 0;

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getMaxValue() const { return max_value; }

    virtual Pixel** getData() const = 0;
    virtual ImageFile* createEmpty() const = 0;
    virtual void setData(int w, int h, Pixel** pixels);
};

#endif // IMAGE_FILE_CLASS
