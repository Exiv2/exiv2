// ***************************************************************** -*- C++ -*-
/*
  Abstract : This is playground code, do what you want with it.

  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  Version  : $Name:  $ $Revision: 1.11 $
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.11 $ $RCSfile: exiftest.cpp,v $")

// *****************************************************************************
// included header files
#include "exif.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

// *****************************************************************************
// local declarations

using namespace Exif;

void exifPrint(const ExifData& exifData);
template<typename T> 
std::string toString(T arg);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
try {

    if (argc != 2) {
        std::cout << "Usage: exiftest file\n";
        return 1;
    }

    ExifData exifData;
    int rc = exifData.read(argv[1]);
    if (rc) {
        std::string error = "Reading Exif data failed: ";
        switch (rc) {
        case -1:
            error += "Couldn't open file `" + std::string(argv[1]) + "'";
            break;
        case 1:
            error += "Couldn't read from the input stream";
            break;
        case 2:
            error += "This does not look like a JPEG image";
            break;
        case 3:
        case 4:
            error += "No Exif data found in the file";
            break;
        case -99:
            error += "Unsupported Exif or GPS data found in IFD 1";
            break;
        default:
            error += "rc = " + toString(rc);
            break;
        }
        throw Error(error);
    }

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

template<typename T> 
std::string toString(T arg)
{
    std::ostringstream os;
    os << arg;
    return os.str();
}
