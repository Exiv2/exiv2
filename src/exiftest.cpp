// ***************************************************************** -*- C++ -*-
/*
  Abstract : This is playground code, do what you want with it.

  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  Version  : $Name:  $ $Revision: 1.16 $
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.16 $ $RCSfile: exiftest.cpp,v $")

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
std::string readError(int rc, const char* file);
std::string writeError(int rc, const char* file);

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
        std::string error = readError(rc, argv[1]);
        throw Error(error);
    }

    exifPrint(exifData);
    exifData.writeThumbnail("thumb");

    std::string key = "Image.UserInfo.UserComment";
    Value* value = Value::create(undefined);
    value->read("ASCII\0\0\0Chan Yee Send", 21, bigEndian);
    exifData.add(key, value);
    delete value;
    key = "Image.OtherTags.Copyright";
    value = Value::create(asciiString);
    value->read(std::string("Photographer\0", 13));
//    value->read(std::string(" \0Editor\0", 9));
//    value->read(std::string("Photographer\0Editor\0", 20));
    std::cout << "value size is (20) " << value->size() << "\n";
    std::cout << "value is " << *value << "\n";

    exifData.add(key, value);

    delete value;

    char* buf = new char[1024*128];
    long siz = exifData.copy(buf);

    std::cout << siz << " Bytes written.\n" 
              << "=======================\n";

    ExifData e2;
    e2.read(buf, siz);
    
    exifPrint(e2);
    e2.writeThumbnail("t2");

    // Assumes that such a file exists...
    rc = e2.write("test.jpg");
    if (rc) {
        std::string error = writeError(rc, "test.jpg");
        throw Error(error);
    }

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
                  << std::dec << *i
//                  << std::dec << i->value() 

//                  << " | " << i->key() 
                  << " | " << i->ifdName()
                  << " | " << i->ifdIdx()

                  << "\n";
    }
}

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
        error = "Reading Exif data failed, rc = " + toString(rc);
        break;
    }
    return error;
}

std::string writeError(int rc, const char* file)
{
    std::string error;
    switch (rc) {
    case -1:
        error = "Couldn't open file `" + std::string(file) + "'";
        break;
    case -2:
        error = "Couldn't open temporary file";
        break;
    case -3:
        error = "Renaming temporary file failed";
        break;
    case 1:
        error = "Couldn't read from the input stream";
        break;
    case 2:
        error = "This does not look like a JPEG image";
        break;
    case 3:
        error = "No JFIF APP0 or Exif APP1 segment found in the file";
        break;
    case 4:
        error = "Writing to the output stream failed";
        break;
    default:
        error = "Reading Exif data failed, rc = " + toString(rc);
        break;
    }
    return error;
}
