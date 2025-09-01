#ifndef PPM_FILE_CLASS
#define PPM_FILE_CLASS

#include <string>

#include "../base/imageFile.h"
#include "../base/map.h"

class PPMFile : public ImageFile {
private:
    std::string magic;
    Map rgb_data;
public:
    PPMFile() {this->type = "PPM";}


    bool load(const std::string& filename) override;
    bool save(const std::string& filename) const override;
    Pixel** getData() const override;
    ImageFile* createEmpty() const override;
    void setData(int w, int h, Pixel** pixels) override;

    void setMagic(std::string magic);
    


    //int getPixel(int x, int y, int channel = 0) const override;
    //void setPixel(int x, int y, int value, int channel = 0) override;
};

#endif // PPM_FILE_CLASS
