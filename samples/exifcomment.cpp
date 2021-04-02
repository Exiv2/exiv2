// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  Abstract : Sample program showing how to set the Exif comment of an image,
             Exif.Photo.UserComment
 */

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <cassert>

// *****************************************************************************
// Main
int main(int argc, char* const argv[])
try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXIV2_ENABLE_BMFF
    Exiv2::enableBMFF();
#endif

    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " file\n";
        return 1;
    }

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(argv[1]);
    assert (image.get() != 0);
    image->readMetadata();
    Exiv2::ExifData &exifData = image->exifData();

    /*
      Exiv2 uses a CommentValue for Exif user comments. The format of the
      comment string includes an optional charset specification at the beginning:

      [charset=[Ascii|Jis|Unicode|Undefined]] comment

      Undefined is used as a default if the comment doesn't start with a charset
      definition.

      Following are a few examples of valid comments. The last one is written to
      the file.
     */
    exifData["Exif.Photo.UserComment"]
        = "charset=Unicode A Unicode Exif comment added with Exiv2";
    exifData["Exif.Photo.UserComment"]
        = "charset=Undefined An undefined Exif comment added with Exiv2";
    exifData["Exif.Photo.UserComment"]
        = "Another undefined Exif comment added with Exiv2";
    exifData["Exif.Photo.UserComment"]
        = "charset=Ascii An ASCII Exif comment added with Exiv2";

    std::cout << "Writing user comment '"
              << exifData["Exif.Photo.UserComment"]
              << "' back to the image\n";

    image->writeMetadata();

    return 0;
}
catch (Exiv2::AnyError& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return -1;
}
