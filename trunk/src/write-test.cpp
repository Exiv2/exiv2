// ***************************************************************** -*- C++ -*-
/*
  Abstract : ExifData write unit tests

  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  Version  : $Name:  $ $Revision: 1.6 $

  Test procedure: 
   $ rm -f test.jpg thumb.jpg iii ttt; 
   $ ./exifprint ../test/img_1771.jpg > iii;
   $ cp ../test/img_1771.jpg ./test.jpg; 
   $ ./makernote-test2 ../test/img_1771.jpg > ttt; 
   $ diff iii ttt

 */
// *****************************************************************************
// included header files
#include "exif.hpp"
#include "makernote.hpp"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <utility>

// *****************************************************************************
// local declarations

using namespace Exiv2;

void testCase(const std::string& file1,
              const std::string& file2,
              const std::string& thumb,
              const std::string& key,
              const std::string& value);
void exifPrint(const ExifData& exifData);

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
{
try {

    if (argc != 3) {
        std::cout << "Usage: write-test file case\n\n"
                  << "where case is an integer between 1 and 6\n";
        return 1;
    }

    std::string testFile = argv[1];
    std::istringstream iss(argv[2]);
    int testNo;
    iss >> testNo;

    int rc = 0;
    switch (testNo) {
    case 1:
        std::cerr << "Case 1: ";
        std::cerr << "Non-intrusive change to the standard Exif metadata\n";
        testCase(testFile, "test1.jpg", "thumb1", 
                 "Image.DateTime.DateTimeOriginal", 
                 "1999:11:22 00:11:22");
        break;
    case 2:
        std::cerr << "Case 2: ";
        std::cerr << "Non-intrusive change to the makernote metadata\n";
        testCase(testFile, "test2.jpg", "thumb2",
                 "Makernote.Canon.OwnerName",
                 "Chan YeeSend");
        break;
    case 3:
        std::cerr << "Case 3: ";
        std::cerr << "Non-intrusive change to the Exif metadata (w/o makernote)\n";
        testCase(testFile, "test3.jpg", "thumb3",
                 "Image.DateTime.DateTimeOriginal", 
                 "1999:11:22 00:11:22");
        break;
    case 4:
        std::cerr << "Case 4: ";
        std::cerr << "Intrusive change to the standard Exif metadata\n";
        testCase(testFile, "test4.jpg", "thumb4",
                 "Image.DateTime.DateTimeOriginal", 
                 "1999:11:22 00:11:22 and twenty seconds");
        break;
    case 5:
        std::cerr << "Case 5: ";
        std::cerr << "Intrusive change to the makernote metadata\n";
        testCase(testFile, "test5.jpg", "thumb5",
                 "Makernote.Canon.OwnerName",
                 "Frau Chan YeeSend und Herr Andreas Huggel");
        break;
    case 6:
        std::cerr << "Case 6: ";
        std::cerr << "Intrusive change to the Exif metadata (w/o makernote)\n";
        testCase(testFile, "test6.jpg", "thumb6",
                 "Image.DateTime.DateTimeOriginal", 
                 "1999:11:22 00:11:22 and twenty seconds");
        break;
    default:
        std::cout << "Usage: exiftest file case\n\n"
                  << "where case is an integer between 1 and 6\n";
        rc = 1;
        break;
    }

    return rc;
}
catch (Error& e) {
    std::cerr << "Caught Exif exception '" << e << "'\n";
    return 1;
}
}

// *****************************************************************************
void testCase(const std::string& file1,
              const std::string& file2,
              const std::string& thumb,
              const std::string& key,
              const std::string& value)
{
    ExifData ed1;

    std::cerr << "---> Reading file " << file1 << "\n";
    int rc = ed1.read(file1);
    if (rc) {
        std::string error = ExifData::strError(rc, file1.c_str());
        throw Error(error);
    }

    std::cerr << "---> Modifying Exif data\n";
    Exiv2::ExifData::iterator pos = ed1.findKey(key);
    if (pos == ed1.end()) {
        throw Error("Metadatum with key = " + key + " not found");
    }
    pos->setValue(value);

    std::cerr << "---> Writing Exif data to file " << file2 << "\n";
    rc = ed1.write(file2);
    if (rc) {
        std::string error = ExifData::strError(rc, file2.c_str());
        throw Error(error);
    }

    ExifData ed2;

    std::cerr << "---> Reading file " << file2 << "\n";
    rc = ed2.read(file2);
    if (rc) {
        std::string error = ExifData::strError(rc, file2.c_str());
        throw Error(error);
    }

    exifPrint(ed2);

    std::cerr << "---> Writing Exif thumbnail to file " << thumb << ".*\n";
    ed2.writeThumbnail(thumb);

}

// *****************************************************************************

void exifPrint(const ExifData& exifData)
{
    ExifData::const_iterator i = exifData.begin();
    for (; i != exifData.end(); ++i) {
        std::cout << std::setw(53) << std::setfill(' ') << std::left
                  << i->key() << " "
                  << "0x" << std::setw(4) << std::setfill('0') << std::right
                  << std::hex << i->tag() << "  " 
                  << std::dec << i->value() 
                  << "\n";
    }
}
