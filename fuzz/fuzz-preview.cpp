#include <exiv2/exiv2.hpp>

#include <cassert>
#include <iomanip>
#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // Invalid files generate a lot of warnings, so switch off logging.
  Exiv2::LogMsg::setLevel(Exiv2::LogMsg::mute);

  try {
    Exiv2::DataBuf data_copy(data, size);
    Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(data_copy.c_data(), size);
    assert(image.get() != 0);

    image->readMetadata();

    Exiv2::PreviewManager pm(*image);
    std::ostringstream os;
    Exiv2::PreviewPropertiesList list = pm.getPreviewProperties();
    for (const auto& pos : list) {
      os << pos.mimeType_ << "\n";

      if (pos.width_ != 0 && pos.height_ != 0)
        os << pos.width_ << " " << pos.height_ << " ";

      os << pos.size_ << "\n";
    }

  } catch (...) {
    // Exiv2 throws an exception if the metadata is invalid.
  }

  return 0;
}
