#include "tags.hpp"
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
                      << i->tag() << " " 
                      << std::setw(27) << std::setfill(' ') << std::left
                      << i->tagName() << " "
                      << std::setw(17) << std::setfill(' ') << std::left
                      << i->typeName() << " "
                      << std::dec << std::setw(3) 
                      << std::setfill(' ') << std::right
                      << i->count() << "   " 
                      << std::dec << i->value() << "\n";
        }
    }

//    std::string tmp = "12 2ddd4. xd35";
    std::string tmp = " 1 2 3";
    Value* val = Value::create(unsignedShort);
    std::cout << "Reading test string \"" << tmp << "\"\n";
    val->read(tmp);
    std::cout << "And the answer is: " << *val << ", size is " << val->size() << "\n";
    Rational r = std::make_pair(1,72);
    URational ur = std::make_pair(2,3);

    std::cout << "Rational  r  = " << r << "\n";
    std::cout << "URational ur = " << ur << "\n";

    ValueType<Rational> vr;
    ValueType<URational> vur;

    std::string str("1/ 2 4 / 5 2 5/3");
    vr.read(str);
    std::cout << "ValueType<Rational> vr  = " << vr 
              << ", size is " << vr.size() << "\n";

    rc = exifData.writeThumbnail("thumb.jpg");

    return rc;
}
