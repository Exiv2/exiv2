#include <exiv2/exiv2.hpp>

#include <cassert>
#include <iomanip>
#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // Invalid files generate a lot of warnings, so switch off logging.
  Exiv2::LogMsg::setLevel(Exiv2::LogMsg::mute);

  Exiv2::XmpParser::initialize();
  ::atexit(Exiv2::XmpParser::terminate);

  try {
    Exiv2::DataBuf data_copy(data, size);
    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(data_copy.c_data(), size);
    assert(image.get() != 0);

    image->readMetadata();
    image->writeMetadata();

  } catch (...) {
    // Exiv2 throws an exception if the metadata is invalid.
  }

  return 0;
}
