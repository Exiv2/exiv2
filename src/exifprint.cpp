// ***************************************************************** -*- C++ -*-
/*
  Abstract : Sample program to print the Exif metadata of an image

  File:      exifprint.cpp
  Version  : $Name:  $ $Revision: 1.11 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 26-Jan-04, ahu: created
 */
// *****************************************************************************
// included header files
#include "exif.hpp"
#include <iostream>
#include <iomanip>
#include <cstring>

// *****************************************************************************
// local declarations

std::string readError(int rc, const char* file);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
try {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exif::ExifData exifData;
    int rc = exifData.read(argv[1]);
    if (rc) {
        std::string error = readError(rc, argv[1]);
        throw Exif::Error(error);
    }

    Exif::ExifData::const_iterator end = exifData.end();
    for (Exif::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
        std::cout << std::setw(53) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << "  " 
                  << std::dec << i->value() 
                  << "\n";
    }

    return rc;
}
catch (Exif::Error& e) {
    std::cout << "Caught Exif exception '" << e << "'\n";
    return -1;
}

// *****************************************************************************
// local definitions

std::string readError(int rc, const char* file)
{
    std::string error;
    switch (rc) {
    case -1:
        error = "Couldn't open file `" + std::string(file) + "'";
        break;
    case 1:
        error = "Couldn't read from the input stream";
        break;
    case 2:
        error = "This does not look like a JPEG image";
        break;
    case 3:
        error = "No Exif data found in the file";
        break;
    case -99:
        error = "Unsupported Exif or GPS data found in IFD 1";
        break;
    default:
        error = "Reading Exif data failed, rc = " + Exif::toString(rc);
        break;
    }
    return error;
}
