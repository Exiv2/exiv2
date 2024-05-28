// SPDX-License-Identifier: GPL-2.0-or-later
// Sample program to dump the XMP packet of an image

#include <exiv2/exiv2.hpp>
#include <iostream>

int main(int argc, char* const argv[]) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  try {
    if (argc != 2) {
      std::cout << "Usage: " << argv[0] << " file\n";
      return EXIT_FAILURE;
    }

    auto image = Exiv2::ImageFactory::open(argv[1]);
    image->readMetadata();

    const std::string& xmpPacket = image->xmpPacket();
    if (xmpPacket.empty()) {
      std::string error(argv[1]);
      error += ": No XMP packet found in the file";
      throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, error);
    }
    std::cout << xmpPacket << "\n";

    return EXIT_SUCCESS;
  } catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e << "'\n";
    return EXIT_FAILURE;
  }
}
