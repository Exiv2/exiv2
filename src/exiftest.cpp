// ***************************************************************** -*- C++ -*-
/*
  Abstract : This is playground code, do what you want with it.

  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  Version  : $Name:  $ $Revision: 1.9 $
 */
// *****************************************************************************
// included header files
#include "exif.hpp"
#include <iostream>
#include <iomanip>

// *****************************************************************************
// local declarations

using namespace Exif;

void exifPrint(const ExifData& exifData);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
try {
    ExifData exifData;

    if (argc != 2) {
        std::cout << "Usage: exiftest file\n";
        return 1;
    }

    int rc = exifData.read(argv[1]);
    if (rc) throw Error("Reading Exif data failed");
    
    exifPrint(exifData);
    
    exifData.writeThumbnail("thumb");

    return rc;
}
catch (Error& e) {
    std::cout << "Caught Exif exception '" << e << "'\n";
    return 1;
}

// *****************************************************************************
// local definitions

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

//                  << " | " << i->key() 
                  << " | " << i->ifdName()
                  << " | " << i->ifdIdx()

                  << "\n";
    }
}
