#ifndef PGM_FILE_CLASS
#define PGM_FILE_CLASS

#include <string>

#include "../base/imageFile.h"
#include "../base/map.h"

class PGMFile : public ImageFile {
private:
    std::string magic;
    Map gray_data;
public:
    bool load(const std::string& filename) override;
    bool save(const std::string& filename) const override;
    Pixel** getData() const override;
    ImageFile* createEmpty() const override;
    void setData(int w, int h, Pixel** pixels) override;

    void setMagic(std::string magic);
    

    //int getPixel(int x, int y, int channel = 0) const override;
    //void setPixel(int x, int y, int value, int channel = 0) override;
};

#endif // PGM_FILE_CLASS
