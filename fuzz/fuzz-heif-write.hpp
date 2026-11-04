// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FUZZ_HEIF_WRITE_HPP
#define FUZZ_HEIF_WRITE_HPP

#include <exiv2/exiv2.hpp>

/*!
  @brief Exercise HEIF serialization and compaction after the drivers' unchanged write.
  @param image Image backed by the fuzz driver's owned MemIo; other image types are ignored.
  @param selector Select an Exif update or metadata removal deterministically from input size.

  A successful write is read back to exercise the emitted container. Exceptions
  propagate to the fuzz driver's existing invalid-input handler.
 */
inline void fuzzHeifWrite(Exiv2::Image& image, size_t selector) {
  if (image.imageType() != Exiv2::ImageType::heif)
    return;

  // Mutate metadata so valid seeds reach the rewrite path rather than only a no-op.
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

  // Parse the result through the public API as well as exercising its serialization.
  image.writeMetadata();
  image.readMetadata();
}

#endif
