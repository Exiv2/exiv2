#include "tags.hpp"
#include "exif.hpp"
#include <iostream>
#include <iomanip>

using namespace Exif;

void exifPrint(const ExifData& exifData);

int main(int argc, char* const argv[])
{
    if (argc != 2) {
        std::cout << "Usage: exiftest path\n";
        return 1;
    }

    int rc;

    try {
        // Add usecase
        ExifData ed;

        Value* v1 = Value::create(signedRational);
        v1->read("1/2 1/3");
        RationalValue* rv = dynamic_cast<RationalValue*>(v1);
        if (rv == 0) return -1;
        rv->value_.push_back(std::make_pair(2,3));
        rv->value_.push_back(std::make_pair(-5,4));
        std::string key = "Thumbnail.ImageStructure.Compression";

        std::cout << "add(\"" << key << "\", " << *rv << ")\n";
        ed.add(key, rv);
        delete rv;

        key = "Image.DateTime.DateTimeOriginal";
        Value* v2 = Value::create(asciiString);
        v2->read("1999:12:31 23:59:59\0");
        ed.add(key, v2);
        std::cout << "add(\"" << key << "\", " << *v2 << ")\n";
        delete v2;

        // Modify usecase
        key = "Image.DateTime.DateTimeOriginal";
        ExifData::iterator md = ed.findKey(key);
        if (md == ed.end()) return -2;

        md->setValue("abcd"); 


        exifPrint(ed);
    }
    catch (Error& e) {
        std::cout << "Caught Exif exception '" << e << "'\n";
    }

    return rc;
}

void exifPrint(const ExifData& exifData)
{
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
                  << std::dec << i->value() 

                  << " | " << i->key() 
                  << " | " << i->ifdName()
                  << " | " << i->ifdIdx()

                  << "\n";
    }
}
