// SPDX-License-Identifier: GPL-2.0-or-later
// Sample program to print value of an exif key in an image

#include <exiv2/exiv2.hpp>

#include <iostream>

int main(int argc, char* const argv[]) {
  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);
#ifdef EXV_ENABLE_BMFF
  Exiv2::enableBMFF();
#endif

  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " file key\n";
    return EXIT_FAILURE;
  }

  const char* file = argv[1];
  const char* key = argv[2];

  auto image = Exiv2::ImageFactory::open(file);
  image->readMetadata();
  Exiv2::ExifData& exifData = image->exifData();

  if (exifData.empty()) {
    std::cerr << "no metadata found in file " << file << std::endl;
    return EXIT_FAILURE;
  }

  try {
    std::cout << exifData[key] << std::endl;
  } catch (Exiv2::Error& e) {
    std::cerr << "Caught Exiv2 exception '" << e << "'" << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "Caught a cold!" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
