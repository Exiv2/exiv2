// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "cr2image.hpp"

#include "config.h"
#include "cr2header_int.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"

#include <array>
#include <iostream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

Cr2Image::Cr2Image(BasicIo::UniquePtr io, bool /*create*/) :
    Image(ImageType::cr2, mdExif | mdIptc | mdXmp, std::move(io)) {
}  // Cr2Image::Cr2Image

std::string Cr2Image::mimeType() const {
  return "image/x-canon-cr2";
}

uint32_t Cr2Image::pixelWidth() const {
  auto imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
  if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
    return imageWidth->toUint32();
  }
  return 0;
}

uint32_t Cr2Image::pixelHeight() const {
  auto imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
  if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
    return imageHeight->toUint32();
  }
  return 0;
}

void Cr2Image::printStructure(std::ostream& out, Exiv2::PrintStructureOption option, size_t depth) {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  io_->seek(0, BasicIo::beg);
  printTiffStructure(io(), out, option, depth);
}

void Cr2Image::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "CR2"));
}

void Cr2Image::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Reading CR2 file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isCr2Type(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "CR2");
  }
  clearMetadata();
  ByteOrder bo = Cr2Parser::decode(exifData_, iptcData_, xmpData_, io_->mmap(), io_->size());
  setByteOrder(bo);
}  // Cr2Image::readMetadata

void Cr2Image::writeMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Writing CR2 file " << io_->path() << "\n";
#endif
  ByteOrder bo = byteOrder();
  byte* pData = nullptr;
  size_t size = 0;
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (io_->open() == 0 && isCr2Type(*io_, false)) {
    pData = io_->mmap(true);
    size = io_->size();
    Internal::Cr2Header cr2Header;
    if (0 == cr2Header.read(pData, 16)) {
      bo = cr2Header.byteOrder();
    }
  }
  if (bo == invalidByteOrder) {
    bo = littleEndian;
  }
  setByteOrder(bo);
  Cr2Parser::encode(*io_, pData, size, bo, exifData_, iptcData_, xmpData_);  // may throw
}  // Cr2Image::writeMetadata

ByteOrder Cr2Parser::decode(ExifData& exifData, IptcData& iptcData, XmpData& xmpData, const byte* pData, size_t size) {
  Internal::Cr2Header cr2Header;
  return Internal::TiffParserWorker::decode(exifData, iptcData, xmpData, pData, size, Internal::Tag::root,
                                            Internal::TiffMapping::findDecoder, &cr2Header);
}

WriteMethod Cr2Parser::encode(BasicIo& io, const byte* pData, size_t size, ByteOrder byteOrder, ExifData& exifData,
                              IptcData& iptcData, XmpData& xmpData) {
  // Delete IFDs which do not occur in TIFF images
  static constexpr auto filteredIfds = std::array{
      IfdId::panaRawId,
  };
  for (auto&& filteredIfd : filteredIfds) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Warning: Exif IFD " << filteredIfd << " not encoded\n";
#endif
    exifData.erase(std::remove_if(exifData.begin(), exifData.end(), Internal::FindExifdatum(filteredIfd)),
                   exifData.end());
  }

  auto header = Internal::Cr2Header(byteOrder);
  Internal::OffsetWriter offsetWriter;
  offsetWriter.setOrigin(Internal::OffsetWriter::cr2RawIfdOffset, Internal::Cr2Header::offset2addr(), byteOrder);
  return Internal::TiffParserWorker::encode(io, pData, size, exifData, iptcData, xmpData, Internal::Tag::root,
                                            Internal::TiffMapping::findEncoder, &header, &offsetWriter);
}

// *************************************************************************
// free functions
Image::UniquePtr newCr2Instance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<Cr2Image>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isCr2Type(BasicIo& iIo, bool advance) {
  const int32_t len = 16;
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  Internal::Cr2Header header;
  bool rc = header.read(buf, len);
  if (!advance || !rc) {
    iIo.seek(-len, BasicIo::cur);
  }
  return rc;
}

}  // namespace Exiv2
