// ***************************************************************** -*- C++ -*-
/*
  Abstract : Sample program showing how to set the Exif comment of an image

  File:      exifcomment.cpp
  Version  : $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History  : 10-May-04, ahu: created
 */
// *****************************************************************************
// included header files
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

    Exiv2::ExifData exifData;
    int rc = exifData.read(argv[1]);
    if (rc) {
        std::string error = Exiv2::ExifData::strError(rc, argv[1]);
        throw Exiv2::Error(error);
    }

    /*
      There are two pitfalls that we need to consider when setting the Exif user
      comment (Exif.Photo.UserComment) of an image:

      First, the type of the Exif user comment tag is "undefined" (and not
      ASCII) according to the Exif standard. This means that in Exiv2, we have
      to deal with a DataValue (and not an AsciiValue). DataValue has the usual
      two read methods, however, the one taking a const std::string& buf expects
      the string to contain a series of integers (e.g., "0 1 2") and not a text
      string. Thus, we need to use the read function that reads the value from a
      character buffer of a given length.

      Second, the Exif comment field starts with an eight character area that
      identifies the used character set. For ASCII, these eight characters are
      "ASCII\0\0\0". The actual comment follows after this code.

      Note: There is a more simple Exif tag for the title of an image. It is a
      20 byte string (type ASCII) and does not store two-byte characters.
      (Exif.Image.ImageDescription)
     */

    // Initialise a data value with the character set and comment
    std::string charset("ASCII\0\0\0", 8);
    std::string comment("A comment added to the Exif metadata through Exiv2");
    Exiv2::DataValue value;
    value.read(reinterpret_cast<const Exiv2::byte*>((charset + comment).data()), 
                8 + static_cast<long>(comment.size()));

    // Set the Exif comment
    Exiv2::ExifKey key("Exif.Photo.UserComment");
    Exiv2::ExifData::iterator pos = exifData.findKey(key);
    if (pos != exifData.end()) {
        // Use the existing Exif UserComment metadatum if there is one
        pos->setValue(&value);
    }
    else {
        // Otherwise add a new UserComment metadatum
        exifData.add(key, &value);
        pos = exifData.findKey(key);
    }

    // Now we should have a valid iterator in any case. We use the metadatum
    // output operator to print the formatted value
    std::cout << "Writing user comment '" << *pos << "' back to the image\n";

    rc = exifData.write(argv[1]);
    if (rc) {
        std::string error = Exiv2::ExifData::strError(rc, argv[1]);
        throw Exiv2::Error(error);
    }

   return rc;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
