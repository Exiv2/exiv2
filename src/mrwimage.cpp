// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "mrwimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "tiffimage.hpp"

#include <iostream>

namespace Exiv2 {
MrwImage::MrwImage(BasicIo::UniquePtr io, bool /*create*/) :
    Image(ImageType::mrw, mdExif | mdIptc | mdXmp, std::move(io)) {
}

std::string MrwImage::mimeType() const {
  return "image/x-minolta-mrw";
}

uint32_t MrwImage::pixelWidth() const {
  auto imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
  if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
    return imageWidth->toUint32();
  }
  return 0;
}

uint32_t MrwImage::pixelHeight() const {
  auto imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
  if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
    return imageHeight->toUint32();
  }
  return 0;
}

void MrwImage::setExifData(const ExifData& /*exifData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Exif metadata", "MRW"));
}

void MrwImage::setIptcData(const IptcData& /*iptcData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "IPTC metadata", "MRW"));
}

void MrwImage::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "MRW"));
}

void MrwImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Reading MRW file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isMrwType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "MRW");
  }
  clearMetadata();

  // Find the TTW block and read it into a buffer
  uint32_t const len = 8;
  byte tmp[len];
  io_->read(tmp, len);
  uint32_t pos = len;
  uint32_t const end = getULong(tmp + 4, bigEndian);

  pos += len;
  Internal::enforce(pos <= end, ErrorCode::kerFailedToReadImageData);
  io_->read(tmp, len);
  if (io_->error() || io_->eof())
    throw Error(ErrorCode::kerFailedToReadImageData);

  while (memcmp(tmp + 1, "TTW", 3) != 0) {
    uint32_t const siz = getULong(tmp + 4, bigEndian);
    Internal::enforce(siz <= end - pos, ErrorCode::kerFailedToReadImageData);
    pos += siz;
    io_->seek(siz, BasicIo::cur);
    Internal::enforce(!io_->error() && !io_->eof(), ErrorCode::kerFailedToReadImageData);

    Internal::enforce(len <= end - pos, ErrorCode::kerFailedToReadImageData);
    pos += len;
    io_->read(tmp, len);
    Internal::enforce(!io_->error() && !io_->eof(), ErrorCode::kerFailedToReadImageData);
  }

  const uint32_t siz = getULong(tmp + 4, bigEndian);
  // First do an approximate bounds check of siz, so that we don't
  // get DOS-ed by a 4GB allocation on the next line. If siz is
  // greater than io_->size() then it is definitely invalid. But the
  // exact bounds checking is done by the call to io_->read, which
  // will fail if there are fewer than siz bytes left to read.
  Internal::enforce(siz <= io_->size(), ErrorCode::kerFailedToReadImageData);
  DataBuf buf(siz);
  io_->read(buf.data(), buf.size());
  Internal::enforce(!io_->error() && !io_->eof(), ErrorCode::kerFailedToReadImageData);

  ByteOrder bo = TiffParser::decode(exifData_, iptcData_, xmpData_, buf.c_data(), buf.size());
  setByteOrder(bo);
}  // MrwImage::readMetadata

void MrwImage::writeMetadata() {
  // Todo: implement me!
  throw(Error(ErrorCode::kerWritingImageFormatUnsupported, "MRW"));
}  // MrwImage::writeMetadata

// *************************************************************************
// free functions
Image::UniquePtr newMrwInstance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<MrwImage>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isMrwType(BasicIo& iIo, bool advance) {
  const int32_t len = 4;
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  int rc = memcmp(buf, "\0MRM", 4);
  if (!advance || rc != 0) {
    iIo.seek(-len, BasicIo::cur);
  }
  return rc == 0;
}

}  // namespace Exiv2
