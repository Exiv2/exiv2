// SPDX-License-Identifier: GPL-2.0-or-later
// Read an XMP from a video or graphic file, parse it and print  all (known) properties.

#include <exiv2/exiv2.hpp>
#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  try {
    if (argc != 2) {
      std::cout << "Usage: " << argv[0] << " file\n";
      return EXIT_FAILURE;
    }

    auto image = Exiv2::ImageFactory::open(argv[1]);
    image->readMetadata();

    Exiv2::XmpData& xmpData = image->xmpData();
    if (xmpData.empty()) {
      std::string error(argv[1]);
      error += ": No XMP data found in the file";
      throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, error);
    }
    if (xmpData.empty()) {
      std::string error(argv[1]);
      error += ": No XMP properties found in the XMP packet";
      throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, error);
    }

    for (auto&& md : xmpData) {
      std::cout << std::setfill(' ') << std::left << std::setw(44) << md.key() << " " << std::setw(9)
                << std::setfill(' ') << std::left << md.typeName() << " " << std::dec << std::setw(3)
                << std::setfill(' ') << std::right << md.count() << "  " << std::dec << md.toString() << '\n';
    }

    Exiv2::XmpParser::terminate();

    return EXIT_SUCCESS;
  } catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return EXIT_FAILURE;
  }
}
