#include "exif.hpp"
#include <iostream>
#include <iomanip>

int main(int argc, char* const argv[])
{
    if (argc != 2) {
        std::cout << "Usage: exiftest path\n";
        return 1;
    }

    Exif::ExifData exifData;
    int rc = exifData.read(argv[1]);

    if (rc == 0) {

        Exif::ExifData::const_iterator beg = exifData.begin();
        Exif::ExifData::const_iterator end = exifData.end();
        Exif::ExifData::const_iterator i = beg;
        for (; i != end; ++i) {

            std::cout << "0x" 
                      << std::hex << std::setw(4) << std::setfill('0') << std::right
                      << i->tag_ << " " 
                      << std::setw(50) << std::setfill(' ') << std::left
                      << i->key() << " "
                      << std::setw(17) << std::setfill(' ') << std::left
                      << i->typeName() << " "
                      << std::dec << std::setw(3) << std::setfill(' ') << std::right
                      << i->count_ << " " 
                      << std::dec << std::setw(3) << std::setfill(' ') << std::right
                      << i->typeSize() * i->count_ << "\n";

        }
    }

    return rc;

}
