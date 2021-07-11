#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * Data, size_t Size) {
  // Invalid files generate a lot of warnings, so switch off logging.
  Exiv2::LogMsg::setLevel(Exiv2::LogMsg::mute);

  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  try {
    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(Data, Size);
    assert(image.get() != 0);

    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
      return -1;
    }
  } catch(...) {
    // Exiv2 throws an exception if the metadata is invalid.
  }

  return 0;
}
