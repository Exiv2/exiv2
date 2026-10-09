// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FUZZ_HEIF_WRITE_HPP
#define FUZZ_HEIF_WRITE_HPP

#include <exiv2/exiv2.hpp>

// The ordinary write keeps metadata unchanged and may take the HEIF no-op
// path. Exercise serialization and compaction as well, on the owned MemIo.
inline void fuzzHeifWrite(Exiv2::Image& image, size_t selector) {
  if (image.imageType() != Exiv2::ImageType::heif)
    return;
  switch (selector % 4) {
    case 0:
      image.exifData()["Exif.Image.ImageDescription"] = "HEIF fuzz metadata";
      break;
    case 1:
      image.clearExifData();
      break;
    case 2:
      image.clearXmpData();
      break;
    case 3:
      image.clearMetadata();
      break;
  }
  image.writeMetadata();
  image.readMetadata();
}

#endif
