// ***************************************************************** -*- C++ -*-
/*
  Abstract : Sample program showing how to set the Exif comment of an image,
             Exif.Photo.UserComment

  File:      exifcomment.cpp
  Version  : $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 10-May-04, ahu: created
             16-Jan-05, ahu: updated using CommentValue and operator trickery
 */
// *****************************************************************************
// included header files
#include "image.hpp"
#include "exif.hpp"
#include <iostream>
#include <iomanip>
#include <cstring>

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
try {

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);
    if (image.get() == 0) {
        std::string error(argv[1]);
        error += " : Could not read file or unknown image type";
        throw Exiv2::Error(error);
    }

    // Load existing metadata
    int rc = image->readMetadata();
    if (rc) {
        std::string error = Exiv2::Image::strError(rc, argv[1]);
        throw Exiv2::Error(error);
    }

    Exiv2::ExifData &exifData = image->exifData();

    /*
      Exiv2 uses a CommentValue for Exif user comments. The format of the 
      comment string includes an optional charset specification at the beginning:

      [charset=["]Ascii|Jis|Unicode|Undefined["] ]comment

      Undefined is used as a default if the comment doesn't start with a charset
      definition.

      Following are a few examples of valid comments. The last one is written to
      the file.
     */
    exifData["Exif.Photo.UserComment"] 
        = "charset=\"Unicode\" An Unicode Exif comment added with Exiv2";
    exifData["Exif.Photo.UserComment"] 
        = "charset=\"Undefined\" An undefined Exif comment added with Exiv2";
    exifData["Exif.Photo.UserComment"] 
        = "Another undefined Exif comment added with Exiv2";
    exifData["Exif.Photo.UserComment"] 
        = "charset=Ascii An ASCII Exif comment added with Exiv2";

    std::cout << "Writing user comment '"
              << exifData["Exif.Photo.UserComment"]
              << "' back to the image\n";

    rc = image->writeMetadata();
    if (rc) {
        std::string error = Exiv2::Image::strError(rc, argv[1]);
        throw Exiv2::Error(error);
    }

    return rc;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
