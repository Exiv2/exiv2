#include "exif.hpp"
#include <iostream>
#include <iomanip>

using namespace Exif;

int main(int argc, char* const argv[])
{
    if (argc != 2) {
        std::cout << "Usage: exiftest path\n";
        return 1;
    }

    ExifData exifData;
    int rc = exifData.read(argv[1]);

    if (rc == 0) {
        ExifData::const_iterator beg = exifData.begin();
        ExifData::const_iterator end = exifData.end();
        ExifData::const_iterator i = beg;
        for (; i != end; ++i) {

            std::cout << "0x" 
                      << std::hex << std::setw(4) << std::setfill('0') << std::right
                      << i->tag_ << " " 
                      << std::setw(50) << std::setfill(' ') << std::left
                      << i->key() << " ";

            if (i->type_ == 2 || i->type_ == 3) {
                std::cout << std::dec << i->value() << "\n";
            }
            else {
                std::cout << std::setw(17) << std::setfill(' ') << std::left
                          << i->typeName() << " "
                          << std::dec << std::setw(3) 
                          << std::setfill(' ') << std::right
                          << i->count_ << " " 
                          << std::dec << std::setw(3)
                          << std::setfill(' ') << std::right
                          << i->typeSize() * i->count_ << "\n";
            }

        }
    }

    std::string tmp = "12";
    Value* val = Value::create(unsignedShort, littleEndian);
    val->read(tmp);
    std::cout << "And the answer is: " << *val << "\n";

    return rc;

}
