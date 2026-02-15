#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>


extern "C" int LLVMFuzzerTestOneInput(const uint8_t * Data, size_t Size)
try {
    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(Data, Size);
    assert(image.get() != 0);
    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        return -1;
    }


    return 0;
}
catch (Exiv2::Error& e) {
    return -1;
}
