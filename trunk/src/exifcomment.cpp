// ***************************************************************** -*- C++ -*-
/*
  Abstract : Sample program showing how to set the Exif comment of an image

  File:      exifcomment.cpp
  Version  : $Name:  $ $Revision: 1.1 $
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
// local declarations

std::string readError(int rc, const std::string& path);
std::string writeError(int rc, const std::string& path);

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
        std::string error = readError(rc, argv[1]);
        throw Exiv2::Error(error);
    }

    /*
      There are two pitfalls that we need to consider when setting the Exif user
      comment (Image.UserInfo.UserComment) of an image:

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
      (Image.OtherTags.ImageDescription)
     */

    // Initialise a data value with the character set and comment
    std::string charset("ASCII\0\0\0", 8);
    std::string comment("A comment added to the Exif metadata through Exiv2");
    Exiv2::DataValue value;
    value.read((charset + comment).data(), 8 + comment.size());

    // Set the Exif comment
    std::string key = "Image.UserInfo.UserComment";
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
        std::string error = writeError(rc, argv[1]);
        throw Exiv2::Error(error);
    }

   return rc;
}
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}

// *****************************************************************************
// local definitions

std::string readError(int rc, const std::string& path)
{
    std::string error;
    switch (rc) {
    case -1:
        error = path + ": Failed to open the file";
        break;
    case -2:
        error = path + ": The file contains data of an unknown image type";
        break;
    case 1:
        error = path + ": Couldn't read from the input stream";
        break;
    case 2:
        error = path + ": This does not look like a JPEG image";
        break;
    case 3:
        error = path + ": No Exif data found in the file";
        break;
    case -99:
        error = path + ": Unsupported Exif or GPS data found in IFD 1";
        break;
    default:
        error = path + ": Reading Exif data failed, rc = " + Exiv2::toString(rc);
        break;
    }
    return error;
} // exifReadError

std::string writeError(int rc, const std::string& path)
{
    std::string error;
    switch (rc) {
    case -1:
        error = path + ": Failed to open the file";
        break;
    case -2:
        error = path + ": The file contains data of an unknown image type";
        break;
    case -3:
        error = path + ": Couldn't open temporary file";
        break;
    case -4:
        error = path + ": Renaming temporary file failed";
        break;
    case 1:
        error = path + ": Couldn't read from the input stream";
        break;
    case 2:
        error = path + ": This does not look like a JPEG image";
        break;
    case 3:
        error = path + ": No JFIF APP0 or Exif APP1 segment found in the file";
        break;
    case 4:
        error = path + ": Writing to the output stream failed";
        break;
    default:
        error = path + ": Reading Exif data failed, rc = " + Exiv2::toString(rc);
        break;
    }
    return error;
} // writeError
