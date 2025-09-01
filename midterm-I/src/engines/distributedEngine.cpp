#include "../../include/engines/distributedEngine.h"
#include "../../include/pixel/grayPixel.h"
#include "../../include/pixel/RGBPixel.h"

std::vector<ImageFile*> DistributedEngine::applyFilters(ImageFile* file, const std::vector<const Filter*>& filters) const  {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::vector<ImageFile*> localResults;
    SequentialEngine seqEngine;

    // Cada proceso aplica filtros según su rank
    for (size_t i = rank; i < filters.size(); i += size) {
        const Filter* filter = filters[i];
        ImageFile* filteredImage = seqEngine.applyFilter(file, filter);
        localResults.push_back(filteredImage);
    }

    // Serializar cada imagen local para enviar al proceso 0
    std::vector<std::vector<int>> serializedImages;
    std::vector<int> imageSizes;

    for (auto img : localResults) {
        int width = img->getWidth();
        int height = img->getHeight();
        Pixel** data = img->getData();

        bool isGray = (img->getType() == "PGM"); // ejemplo, tu ImageFile puede tener getType
        std::vector<int> buffer(width * height * (isGray ? 1 : 3));

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Pixel* p = data[y * width + x];
                int base = (y*width + x)*(isGray ? 1 : 3);
                for (int c = 0; c < (isGray ? 1 : 3); ++c) {
                    buffer[base + c] = p->getChannel(c);
                }
            }
        }

        serializedImages.push_back(buffer);
        imageSizes.push_back(buffer.size());
    }

    // Enviar al rank 0
    if (rank != 0) {
        int nImages = serializedImages.size();
        MPI_Send(&nImages, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Send(imageSizes.data(), nImages, MPI_INT, 0, 1, MPI_COMM_WORLD);
        for (int i = 0; i < nImages; ++i) {
            MPI_Send(serializedImages[i].data(), imageSizes[i], MPI_INT, 0, 2+i, MPI_COMM_WORLD);
        }
    }

    std::vector<ImageFile*> allResults = localResults;

    if (rank == 0) {
        for (int src = 1; src < size; ++src) {
            int nImages;
            MPI_Recv(&nImages, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            std::vector<int> recvSizes(nImages);
            MPI_Recv(recvSizes.data(), nImages, MPI_INT, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int i = 0; i < nImages; ++i) {
                std::vector<int> buffer(recvSizes[i]);
                MPI_Recv(buffer.data(), recvSizes[i], MPI_INT, src, 2+i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                ImageFile* img = file->createEmpty();
                int width = file->getWidth();
                int height = file->getHeight();
                Pixel** data = new Pixel*[width * height];
                bool isGray = (img->getType() == "PGM");

                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        Pixel* p = nullptr;
                        if (isGray) {
                            p = new GrayPixel();
                            p->setChannel(0, buffer[y*width + x]);
                        } else {
                            p = new RGBPixel();
                            for (int c = 0; c < 3; ++c) {
                                p->setChannel(c, buffer[(y*width + x)*3 + c]);
                            }
                        }
                        data[y*width + x] = p;
                    }
                }
                img->setData(width, height, data);
                allResults.push_back(img);
            }
        }
    }

    return allResults;
}

ImageFile* DistributedEngine::applyFilter(ImageFile* file, const Filter* filter) {
    // Podrías llamar a SequentialEngine directamente
    SequentialEngine seqEngine;
    return seqEngine.applyFilter(file, filter);
}


