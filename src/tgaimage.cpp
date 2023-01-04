// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "tgaimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"

#include <iostream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
TgaImage::TgaImage(BasicIo::UniquePtr io) : Image(ImageType::tga, mdNone, std::move(io)) {
}

std::string TgaImage::mimeType() const {
  return "image/targa";
}

void TgaImage::setExifData(const ExifData& /*exifData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Exif metadata", "TGA"));
}

void TgaImage::setIptcData(const IptcData& /*iptcData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "IPTC metadata", "TGA"));
}

void TgaImage::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "TGA"));
}

void TgaImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Exiv2::TgaImage::readMetadata: Reading TARGA file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isTgaType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "TGA");
  }
  clearMetadata();

  /*
    The TARGA header goes as follows -- all numbers are in little-endian byte order:

    offset  length   name                     description
    ======  =======  =======================  ===========
     0      1 byte   ID length                length of image ID (0 to 255)
     1      1 byte   color map type           0 = no color map; 1 = color map included
     2      1 byte   image type                0 = no image;
                                               1 = uncompressed color-mapped;
                                               2 = uncompressed true-color;
                                               3 = uncompressed black-and-white;
                                               9 = RLE-encoded color mapped;
                                              10 = RLE-encoded true-color;
                                              11 = RLE-encoded black-and-white
     3      5 bytes  color map specification
     8      2 bytes  x-origin of image
    10      2 bytes  y-origin of image
    12      2 bytes  image width
    14      2 bytes  image height
    16      1 byte   pixel depth
    17      1 byte   image descriptor
  */
  byte buf[18];
  if (io_->read(buf, sizeof(buf)) == sizeof(buf)) {
    pixelWidth_ = getShort(buf + 12, littleEndian);
    pixelHeight_ = getShort(buf + 14, littleEndian);
  }
}  // TgaImage::readMetadata

void TgaImage::writeMetadata() {
  // Todo: implement me!
  throw(Error(ErrorCode::kerWritingImageFormatUnsupported, "TGA"));
}  // TgaImage::writeMetadata

// *************************************************************************
// free functions
Image::UniquePtr newTgaInstance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<TgaImage>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isTgaType(BasicIo& iIo, bool /*advance*/) {
  // not all TARGA files have a signature string, so first just try to match the file name extension
  const std::string& path = iIo.path();
  if (path.rfind(".tga") != std::string::npos || path.rfind(".TGA") != std::string::npos) {
    return true;
  }
  byte buf[26];
  const size_t curPos = iIo.tell();
  if (curPos < 26)
    return false;

  iIo.seek(-26, BasicIo::end);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  iIo.read(buf, sizeof(buf));
  if (iIo.error()) {
    return false;
  }
  // some TARGA files, but not all, have a signature string at the end
  bool matched = (memcmp(buf + 8, "TRUEVISION-XFILE", 16) == 0);
  iIo.seek(curPos, BasicIo::beg);
  return matched;
}
}  // namespace Exiv2
