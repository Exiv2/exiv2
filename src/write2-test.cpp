// ***************************************************************** -*- C++ -*-
/*
  Abstract : ExifData write unit tests for Exif data created from scratch

  File     : write2-test.cpp
  Version  : $Name:  $ $Revision: 1.3 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 26-Jun-04, ahu: created

 */
// *****************************************************************************
// included header files
#include "exif.hpp"
#include <iostream>
#include <iomanip>
#include <string>

void write(const std::string& file, Exiv2::ExifData& ed);
void print(const std::string& file);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
try {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }
    std::string file(argv[1]);

    std::cout <<"----- One IFD0 tag\n";
    Exiv2::ExifData ed1;
    Exiv2::Exifdatum md1("Image.OtherTags.Model");
    md1.setValue("Test 1");
    ed1.add(md1);
    write(file, ed1);
    print(file);

    std::cout <<"\n----- One Exif tag\n";
    Exiv2::ExifData ed2;
    Exiv2::Exifdatum md2("Image.DateTime.DateTimeOriginal");
    md2.setValue("Test 2");
    ed2.add(md2);
    write(file, ed2);
    print(file);

    // Todo: One Makernote tag for each Makernote

    std::cout <<"\n----- One IOP tag\n";
    Exiv2::ExifData ed3;
    Exiv2::Exifdatum md3("Image.Interoperability.InteroperabilityVersion");
    md3.setValue("Test 3");
    ed3.add(md3);
    write(file, ed3);
    print(file);

    std::cout <<"\n----- One GPS tag\n";
    Exiv2::ExifData ed4;
    Exiv2::Exifdatum md4("Image.GPS.GPSVersionID");
    md4.setValue("Test 4");
    ed4.add(md4);
    write(file, ed4);
    print(file);

    // Todo: Fix this
    std::cout <<"\n----- One IFD1 tag\n";
    Exiv2::ExifData ed5;
    Exiv2::Exifdatum md5("Thumbnail.OtherTags.Artist");
    md5.setValue("Test 5");
    ed5.add(md5);

    Exiv2::Exifdatum md6("Image.OtherTags.Model");
    md6.setValue("Test 5 (Fix me!)");
    ed5.add(md6);

    write(file, ed5);
    print(file);

    return 0;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
}

void write(const std::string& file, Exiv2::ExifData& ed)
{
    int rc = ed.write(file);
    if (rc) {
        std::string error = Exiv2::ExifData::strError(rc, file);
        throw Exiv2::Error(error);
    }
}

void print(const std::string& file)
{
    Exiv2::ExifData ed;
    int rc = ed.read(file);
    if (rc) {
        std::string error = Exiv2::ExifData::strError(rc, file);
        throw Exiv2::Error(error);
    }

    Exiv2::ExifData::const_iterator end = ed.end();
    for (Exiv2::ExifData::const_iterator i = ed.begin(); i != end; ++i) {
        std::cout << std::setw(53) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << std::setw(12) << std::setfill(' ') << std::left
                  << i->ifdName() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << "  " 
                  << std::dec << i->value() 
                  << "\n";
    }
}
