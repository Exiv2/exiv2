// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "orfimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "orfimage_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"

#include <array>
#include <iostream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
using namespace Internal;

OrfImage::OrfImage(BasicIo::UniquePtr io, bool create) :
    TiffImage(/*ImageType::orf, mdExif | mdIptc | mdXmp,*/ std::move(io), create) {
  setTypeSupported(ImageType::orf, mdExif | mdIptc | mdXmp);
}  // OrfImage::OrfImage

std::string OrfImage::mimeType() const {
  return "image/x-olympus-orf";
}

uint32_t OrfImage::pixelWidth() const {
  auto imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
  if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
    return imageWidth->toUint32();
  }
  return 0;
}

uint32_t OrfImage::pixelHeight() const {
  auto imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
  if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
    return imageHeight->toUint32();
  }
  return 0;
}

void OrfImage::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "ORF"));
}

void OrfImage::printStructure(std::ostream& out, PrintStructureOption option, size_t depth) {
  out << "ORF IMAGE" << '\n';
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  // Ensure that this is the correct image type
  if (imageType() == ImageType::none && !isOrfType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAJpeg);
  }

  io_->seek(0, BasicIo::beg);

  printTiffStructure(io(), out, option, depth);
}  // OrfImage::printStructure

void OrfImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Reading ORF file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isOrfType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "ORF");
  }
  clearMetadata();
  ByteOrder bo = OrfParser::decode(exifData_, iptcData_, xmpData_, io_->mmap(), io_->size());
  setByteOrder(bo);
}

void OrfImage::writeMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Writing ORF file " << io_->path() << "\n";
#endif
  ByteOrder bo = byteOrder();
  byte* pData = nullptr;
  size_t size = 0;
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (io_->open() == 0 && isOrfType(*io_, false)) {
    pData = io_->mmap(true);
    size = io_->size();
    OrfHeader orfHeader;
    if (0 == orfHeader.read(pData, 8)) {
      bo = orfHeader.byteOrder();
    }
  }
  if (bo == invalidByteOrder) {
    bo = littleEndian;
  }
  setByteOrder(bo);
  OrfParser::encode(*io_, pData, size, bo, exifData_, iptcData_, xmpData_);  // may throw
}  // OrfImage::writeMetadata

ByteOrder OrfParser::decode(ExifData& exifData, IptcData& iptcData, XmpData& xmpData, const byte* pData, size_t size) {
  OrfHeader orfHeader;
  return TiffParserWorker::decode(exifData, iptcData, xmpData, pData, size, Tag::root, TiffMapping::findDecoder,
                                  &orfHeader);
}

WriteMethod OrfParser::encode(BasicIo& io, const byte* pData, size_t size, ByteOrder byteOrder, ExifData& exifData,
                              IptcData& iptcData, XmpData& xmpData) {
  // Delete IFDs which do not occur in TIFF images
  static constexpr auto filteredIfds = {
      IfdId::panaRawId,
  };
  for (auto&& filteredIfd : filteredIfds) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Warning: Exif IFD " << filteredIfd << " not encoded\n";
#endif
    exifData.erase(std::remove_if(exifData.begin(), exifData.end(), FindExifdatum(filteredIfd)), exifData.end());
  }

  OrfHeader header(byteOrder);
  return TiffParserWorker::encode(io, pData, size, exifData, iptcData, xmpData, Tag::root, TiffMapping::findEncoder,
                                  &header, nullptr);
}

// *************************************************************************
// free functions
Image::UniquePtr newOrfInstance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<OrfImage>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isOrfType(BasicIo& iIo, bool advance) {
  const int32_t len = 8;
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  OrfHeader orfHeader;
  bool rc = orfHeader.read(buf, len);
  if (!advance || !rc) {
    iIo.seek(-len, BasicIo::cur);
  }
  return rc;
}

}  // namespace Exiv2
