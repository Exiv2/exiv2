#include "exif.h"
#include <iostream>

int main(int argc, char* const argv[])
{
    if (argc != 2) {
        std::cout << "Usage: exiftest path\n";
        return 1;
    }

    Exif::ExifData exifData;
    return exifData.read(argv[1]);
}
