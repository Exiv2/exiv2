// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "tiffimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "types.hpp"

#include <array>
#include <iostream>

/* --------------------------------------------------------------------------

   Todo:

   + CR2 Makernotes don't seem to have a next pointer but Canon Jpeg Makernotes
     do. What a mess. (That'll become an issue when it comes to writing to CR2)
   + Sony makernotes in RAW files do not seem to have header like those in Jpegs.
     And maybe no next pointer either.

   in crwimage.* :

   + Fix CiffHeader according to TiffHeader
   + Combine Error(ErrorCode::kerNotAJpeg) and Error(kerNotACrwImage), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + rename loadStack to getPath for consistency

   -------------------------------------------------------------------------- */

// *****************************************************************************
// class member definitions
namespace Exiv2 {
using namespace Internal;

TiffImage::TiffImage(BasicIo::UniquePtr io, bool /*create*/) :
    Image(ImageType::tiff, mdExif | mdIptc | mdXmp, std::move(io)) {
}  // TiffImage::TiffImage

//! List of TIFF compression to MIME type mappings

constexpr struct mimeType {
  int comp;
  const char* type;

  bool operator==(int c) const {
    return comp == c;
  }
} mimeTypeList[] = {
    {32767, "image/x-sony-arw"},  {32769, "image/x-epson-erf"}, {32770, "image/x-samsung-srw"},
    {34713, "image/x-nikon-nef"}, {65000, "image/x-kodak-dcr"}, {65535, "image/x-pentax-pef"},
};

std::string TiffImage::mimeType() const {
  if (!mimeType_.empty())
    return mimeType_;

  mimeType_ = std::string("image/tiff");
  std::string key = "Exif." + primaryGroup() + ".Compression";
  auto md = exifData_.findKey(ExifKey(key));
  if (md != exifData_.end() && md->count() > 0) {
    auto mt = Exiv2::find(mimeTypeList, static_cast<int>(md->toInt64()));
    if (mt)
      mimeType_ = mt->type;
  }
  return mimeType_;
}

std::string TiffImage::primaryGroup() const {
  if (!primaryGroup_.empty())
    return primaryGroup_;

  static constexpr auto keys = std::array{
      "Exif.Image.NewSubfileType",     "Exif.SubImage1.NewSubfileType", "Exif.SubImage2.NewSubfileType",
      "Exif.SubImage3.NewSubfileType", "Exif.SubImage4.NewSubfileType", "Exif.SubImage5.NewSubfileType",
      "Exif.SubImage6.NewSubfileType", "Exif.SubImage7.NewSubfileType", "Exif.SubImage8.NewSubfileType",
      "Exif.SubImage9.NewSubfileType",
  };
  // Find the group of the primary image, default to "Image"
  primaryGroup_ = std::string("Image");
  for (auto i : keys) {
    auto md = exifData_.findKey(ExifKey(i));
    // Is it the primary image?
    if (md != exifData_.end() && md->count() > 0 && md->toInt64() == 0) {
      // Sometimes there is a JPEG primary image; that's not our first choice
      primaryGroup_ = md->groupName();
      std::string key = "Exif." + primaryGroup_ + ".JPEGInterchangeFormat";
      if (exifData_.findKey(ExifKey(key)) == exifData_.end())
        break;
    }
  }
  return primaryGroup_;
}

uint32_t TiffImage::pixelWidth() const {
  if (pixelWidthPrimary_ != 0) {
    return pixelWidthPrimary_;
  }

  ExifKey key(std::string("Exif.") + primaryGroup() + std::string(".ImageWidth"));
  auto imageWidth = exifData_.findKey(key);
  if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
    pixelWidthPrimary_ = imageWidth->toUint32();
  }
  return pixelWidthPrimary_;
}

uint32_t TiffImage::pixelHeight() const {
  if (pixelHeightPrimary_ != 0) {
    return pixelHeightPrimary_;
  }

  ExifKey key(std::string("Exif.") + primaryGroup() + std::string(".ImageLength"));
  auto imageHeight = exifData_.findKey(key);
  if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
    pixelHeightPrimary_ = imageHeight->toUint32();
  }
  return pixelHeightPrimary_;
}

void TiffImage::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "TIFF"));
}

void TiffImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Reading TIFF file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }

  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isTiffType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "TIFF");
  }
  clearMetadata();

  ByteOrder bo = TiffParser::decode(exifData_, iptcData_, xmpData_, io_->mmap(), io_->size());
  setByteOrder(bo);

  // read profile from the metadata
  Exiv2::ExifKey key("Exif.Image.InterColorProfile");
  auto pos = exifData_.findKey(key);
  if (pos != exifData_.end()) {
    size_t size = pos->count() * pos->typeSize();
    if (size == 0) {
      throw Error(ErrorCode::kerFailedToReadImageData);
    }
    iccProfile_.alloc(size);
    pos->copy(iccProfile_.data(), bo);
  }
}

void TiffImage::writeMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Writing TIFF file " << io_->path() << "\n";
#endif
  ByteOrder bo = byteOrder();
  byte* pData = nullptr;
  size_t size = 0;
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (io_->open() == 0 && isTiffType(*io_, false)) {
    pData = io_->mmap(true);
    size = io_->size();
    TiffHeader tiffHeader;
    if (0 == tiffHeader.read(pData, 8)) {
      bo = tiffHeader.byteOrder();
    }
  }
  if (bo == invalidByteOrder) {
    bo = littleEndian;
  }
  setByteOrder(bo);

  // fixup ICC profile
  Exiv2::ExifKey key("Exif.Image.InterColorProfile");
  auto pos = exifData_.findKey(key);
  bool found = pos != exifData_.end();
  if (iccProfileDefined()) {
    Exiv2::DataValue value(iccProfile_.c_data(), iccProfile_.size());
    if (found)
      pos->setValue(&value);
    else
      exifData_.add(key, &value);
  } else {
    if (found)
      exifData_.erase(pos);
  }

  // set usePacket to influence TiffEncoder::encodeXmp() called by TiffVisitor.encode()
  xmpData().usePacket(writeXmpFromPacket());

  TiffParser::encode(*io_, pData, size, bo, exifData_, iptcData_, xmpData_);  // may throw
}  // TiffImage::writeMetadata

ByteOrder TiffParser::decode(ExifData& exifData, IptcData& iptcData, XmpData& xmpData, const byte* pData, size_t size) {
  uint32_t root = Tag::root;

  // #1402  Fujifilm RAF. Change root when parsing embedded tiff
  Exiv2::ExifKey key("Exif.Image.Make");
  if (exifData.findKey(key) != exifData.end() && exifData.findKey(key)->toString() == "FUJIFILM") {
    root = Tag::fuji;
  }

  return TiffParserWorker::decode(exifData, iptcData, xmpData, pData, size, root, TiffMapping::findDecoder);
}  // TiffParser::decode

WriteMethod TiffParser::encode(BasicIo& io, const byte* pData, size_t size, ByteOrder byteOrder, ExifData& exifData,
                               IptcData& iptcData, XmpData& xmpData) {
  // Delete IFDs which do not occur in TIFF images
  static constexpr auto filteredIfds = std::array{
      IfdId::panaRawId,
  };
  for (auto filteredIfd : filteredIfds) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Warning: Exif IFD " << filteredIfd << " not encoded\n";
#endif
    exifData.erase(std::remove_if(exifData.begin(), exifData.end(), FindExifdatum(filteredIfd)), exifData.end());
  }

  TiffHeader header(byteOrder);
  return TiffParserWorker::encode(io, pData, size, exifData, iptcData, xmpData, Tag::root, TiffMapping::findEncoder,
                                  &header, nullptr);
}  // TiffParser::encode

// *************************************************************************
// free functions
Image::UniquePtr newTiffInstance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<TiffImage>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isTiffType(BasicIo& iIo, bool advance) {
  const int32_t len = 8;
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  TiffHeader tiffHeader;
  bool rc = tiffHeader.read(buf, len);
  if (!advance || !rc) {
    iIo.seek(-len, BasicIo::cur);
  }
  return rc;
}

void TiffImage::printStructure(std::ostream& out, Exiv2::PrintStructureOption option, size_t depth) {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  // Ensure that this is the correct image type
  if (imageType() == ImageType::none && !isTiffType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAJpeg);
  }

  io_->seek(0, BasicIo::beg);

  printTiffStructure(io(), out, option, depth);
}

}  // namespace Exiv2
