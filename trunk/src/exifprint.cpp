// ***************************************************************** -*- C++ -*-
// exifprint.cpp, $Rev$
// Sample program to print the Exif metadata of an image

#include "image.hpp"
#include "exif.hpp"
#include <iostream>
#include <iomanip>

int main(int argc, char* const argv[])
try {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);
    if (image.get() == 0) {
        std::string error(argv[1]);
        error += " : Could not read file or unknown image type";
        throw Exiv2::Error(error);
    }

    // Load existing metadata
    int rc = image->readMetadata();
    if (rc) {
        std::string error = Exiv2::Image::strError(rc, argv[1]);
        throw Exiv2::Error(error);
    }
    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
        std::string error(argv[1]);
        error += ": No Exif data found in the file";
        throw Exiv2::Error(error);
    }
    Exiv2::ExifData::const_iterator end = exifData.end();
    for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
        std::cout << std::setw(53) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << "  " 
                  << std::dec << i->value() 
                  << "\n";
    }

    return rc;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
