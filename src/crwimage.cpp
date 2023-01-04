// SPDX-License-Identifier: GPL-2.0-or-later
/*
  File:      crwimage.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   28-Aug-05, ahu: created
 */
// included header files
#include "config.h"

#include "crwimage.hpp"
#include "crwimage_int.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "tags.hpp"

#include <iostream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

CrwImage::CrwImage(BasicIo::UniquePtr io, bool /*create*/) : Image(ImageType::crw, mdExif | mdComment, std::move(io)) {
}  // CrwImage::CrwImage

std::string CrwImage::mimeType() const {
  return "image/x-canon-crw";
}

uint32_t CrwImage::pixelWidth() const {
  auto widthIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
  if (widthIter != exifData_.end() && widthIter->count() > 0) {
    return widthIter->toUint32();
  }
  return 0;
}

uint32_t CrwImage::pixelHeight() const {
  auto heightIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
  if (heightIter != exifData_.end() && heightIter->count() > 0) {
    return heightIter->toUint32();
  }
  return 0;
}

void CrwImage::setIptcData(const IptcData& /*iptcData*/) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "IPTC metadata", "CRW"));
}

void CrwImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Reading CRW file " << io_->path() << "\n";
#endif
  if (io_->open()) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isCrwType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotACrwImage);
  }
  clearMetadata();
  DataBuf file(io().size());
  io_->read(file.data(), file.size());

  CrwParser::decode(this, io_->mmap(), io_->size());

}  // CrwImage::readMetadata

void CrwImage::writeMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Writing CRW file " << io_->path() << "\n";
#endif
  // Read existing image
  DataBuf buf;
  if (io_->open() == 0) {
    IoCloser closer(*io_);
    // Ensure that this is the correct image type
    if (isCrwType(*io_, false)) {
      // Read the image into a memory buffer
      buf.alloc(io_->size());
      io_->read(buf.data(), buf.size());
      if (io_->error() || io_->eof()) {
        buf.reset();
      }
    }
  }

  Blob blob;
  CrwParser::encode(blob, buf.c_data(), buf.size(), this);

  // Write new buffer to file
  MemIo tempIo;
  tempIo.write((!blob.empty() ? blob.data() : nullptr), blob.size());
  io_->close();
  io_->transfer(tempIo);  // may throw

}  // CrwImage::writeMetadata

void CrwParser::decode(CrwImage* pCrwImage, const byte* pData, size_t size) {
  // Parse the image, starting with a CIFF header component
  Internal::CiffHeader header;
  header.read(pData, size);
  header.decode(*pCrwImage);

  // a hack to get absolute offset of preview image inside CRW structure
  auto preview = header.findComponent(0x2007, 0x0000);
  if (preview) {
    (pCrwImage->exifData())["Exif.Image2.JPEGInterchangeFormat"] = static_cast<uint32_t>(preview->pData() - pData);
    (pCrwImage->exifData())["Exif.Image2.JPEGInterchangeFormatLength"] = static_cast<uint32_t>(preview->size());
  }
}  // CrwParser::decode

void CrwParser::encode(Blob& blob, const byte* pData, size_t size, const CrwImage* pCrwImage) {
  // Parse image, starting with a CIFF header component
  Internal::CiffHeader header;
  if (size != 0) {
    header.read(pData, size);
  }

  // Encode Exif tags from image into the CRW parse tree and write the
  // structure to the binary image blob
  Internal::CrwMap::encode(&header, *pCrwImage);
  header.write(blob);
}

// *************************************************************************
// free functions
Image::UniquePtr newCrwInstance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<CrwImage>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isCrwType(BasicIo& iIo, bool advance) {
  bool result = true;
  byte tmpBuf[14];
  iIo.read(tmpBuf, 14);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  if (('I' != tmpBuf[0] || 'I' != tmpBuf[1]) && ('M' != tmpBuf[0] || 'M' != tmpBuf[1])) {
    result = false;
  }
  if (result && std::memcmp(tmpBuf + 6, Internal::CiffHeader::signature(), 8) != 0) {
    result = false;
  }
  if (!advance || !result)
    iIo.seek(-14, BasicIo::cur);
  return result;
}

}  // namespace Exiv2
