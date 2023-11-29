// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "gifimage.hpp"

#include "config.h"
#include "error.hpp"
#include "futils.hpp"

#include <iostream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
GifImage::GifImage(BasicIo::UniquePtr io) : Image(ImageType::gif, mdNone, std::move(io)) {
}

std::string GifImage::mimeType() const {
  return "image/gif";
}

void GifImage::setExifData(const ExifData& /*exifData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Exif metadata", "GIF"));
}

void GifImage::setIptcData(const IptcData& /*iptcData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "IPTC metadata", "GIF"));
}

void GifImage::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "GIF"));
}

void GifImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Exiv2::GifImage::readMetadata: Reading GIF file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isGifType(*io_, true)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "GIF");
  }
  clearMetadata();

  byte buf[4];
  if (io_->read(buf, sizeof(buf)) == sizeof(buf)) {
    pixelWidth_ = getShort(buf, littleEndian);
    pixelHeight_ = getShort(buf + 2, littleEndian);
  }
}  // GifImage::readMetadata

void GifImage::writeMetadata() {
  // Todo: implement me!
  throw(Error(ErrorCode::kerWritingImageFormatUnsupported, "GIF"));
}  // GifImage::writeMetadata

// *************************************************************************
// free functions
Image::UniquePtr newGifInstance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<GifImage>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isGifType(BasicIo& iIo, bool advance) {
  const int32_t len = 6;
  const unsigned char Gif87aId[8] = {'G', 'I', 'F', '8', '7', 'a'};
  const unsigned char Gif89aId[8] = {'G', 'I', 'F', '8', '9', 'a'};
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  bool matched = (memcmp(buf, Gif87aId, len) == 0) || (memcmp(buf, Gif89aId, len) == 0);
  if (!advance || !matched) {
    iIo.seek(-len, BasicIo::cur);
  }
  return matched;
}
}  // namespace Exiv2
