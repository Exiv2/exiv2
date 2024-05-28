// SPDX-License-Identifier: GPL-2.0-or-later
// Conversion test driver - make sure you have a copy of the input file around!

#include <exiv2/exiv2.hpp>

#include <iostream>

int main(int argc, char* const argv[]) {
  try {
    Exiv2::XmpParser::initialize();
    ::atexit(Exiv2::XmpParser::terminate);

    if (argc != 2) {
      std::cout << "Usage: " << argv[0] << " file\n";
      return EXIT_FAILURE;
    }

    auto image = Exiv2::ImageFactory::open(argv[1]);
    image->readMetadata();

    Exiv2::XmpData xmpData;
    Exiv2::copyExifToXmp(image->exifData(), xmpData);

    Exiv2::ExifData exifData;
    Exiv2::copyXmpToExif(xmpData, exifData);

    image->setXmpData(xmpData);
    image->setExifData(exifData);
    image->writeMetadata();

    return EXIT_SUCCESS;
  } catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return EXIT_FAILURE;
  }
}
