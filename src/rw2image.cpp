// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "rw2image.hpp"

#include "config.h"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "preview.hpp"
#include "rw2image_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"

// + standard includes
#include <array>
#ifdef EXIV2_DEBUG_MESSAGES
#include <iostream>
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {
using namespace Internal;

Rw2Image::Rw2Image(BasicIo::UniquePtr io) : Image(ImageType::rw2, mdExif | mdIptc | mdXmp, std::move(io)) {
}  // Rw2Image::Rw2Image

std::string Rw2Image::mimeType() const {
  return "image/x-panasonic-rw2";
}

uint32_t Rw2Image::pixelWidth() const {
  auto imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.PanasonicRaw.SensorWidth"));
  if (imageWidth == exifData_.end() || imageWidth->count() == 0)
    return 0;
  return imageWidth->toUint32();
}

uint32_t Rw2Image::pixelHeight() const {
  auto imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.PanasonicRaw.SensorHeight"));
  if (imageHeight == exifData_.end() || imageHeight->count() == 0)
    return 0;
  return imageHeight->toUint32();
}

void Rw2Image::setExifData(const ExifData& /*exifData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Exif metadata", "RW2"));
}

void Rw2Image::setIptcData(const IptcData& /*iptcData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "IPTC metadata", "RW2"));
}

void Rw2Image::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "RW2"));
}

void Rw2Image::printStructure(std::ostream& out, PrintStructureOption option, size_t depth) {
  out << "RW2 IMAGE" << '\n';
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  // Ensure that this is the correct image type
  if (imageType() == ImageType::none && !isRw2Type(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAJpeg);
  }

  io_->seek(0, BasicIo::beg);

  printTiffStructure(io(), out, option, depth);
}  // Rw2Image::printStructure

void Rw2Image::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Reading RW2 file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isRw2Type(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "RW2");
  }
  clearMetadata();
  ByteOrder bo = Rw2Parser::decode(exifData_, iptcData_, xmpData_, io_->mmap(), io_->size());
  setByteOrder(bo);

  // A lot more metadata is hidden in the embedded preview image
  // Todo: This should go into the Rw2Parser, but for that it needs the Image
  PreviewManager loader(*this);
  PreviewPropertiesList list = loader.getPreviewProperties();
  // Todo: What if there are more preview images?
  if (list.size() > 1) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "RW2 image contains more than one preview. None used.\n";
#endif
  }
  if (list.size() != 1)
    return;
  ExifData exifData;
  PreviewImage preview = loader.getPreviewImage(*list.begin());
  auto image = ImageFactory::open(preview.pData(), preview.size());
  if (!image) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Failed to open RW2 preview image.\n";
#endif
    return;
  }
  image->readMetadata();
  ExifData& prevData = image->exifData();
  if (!prevData.empty()) {
    // Filter duplicate tags
    for (const auto& pos : exifData_) {
      if (pos.ifdId() == IfdId::panaRawId)
        continue;
      auto dup = prevData.findKey(ExifKey(pos.key()));
      if (dup != prevData.end()) {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Filtering duplicate tag " << pos.key() << " (values '" << pos.value() << "' and '" << dup->value()
                  << "')\n";
#endif
        prevData.erase(dup);
      }
    }
  }
  // Remove tags not applicable for raw images
  static constexpr auto filteredTags = std::array{
      "Exif.Photo.ComponentsConfiguration",
      "Exif.Photo.CompressedBitsPerPixel",
      "Exif.Panasonic.ColorEffect",
      "Exif.Panasonic.Contrast",
      "Exif.Panasonic.NoiseReduction",
      "Exif.Panasonic.ColorMode",
      "Exif.Panasonic.OpticalZoomMode",
      "Exif.Panasonic.Contrast",
      "Exif.Panasonic.Saturation",
      "Exif.Panasonic.Sharpness",
      "Exif.Panasonic.FilmMode",
      "Exif.Panasonic.SceneMode",
      "Exif.Panasonic.WBRedLevel",
      "Exif.Panasonic.WBGreenLevel",
      "Exif.Panasonic.WBBlueLevel",
      "Exif.Photo.ColorSpace",
      "Exif.Photo.PixelXDimension",
      "Exif.Photo.PixelYDimension",
      "Exif.Photo.SceneType",
      "Exif.Photo.CustomRendered",
      "Exif.Photo.DigitalZoomRatio",
      "Exif.Photo.SceneCaptureType",
      "Exif.Photo.GainControl",
      "Exif.Photo.Contrast",
      "Exif.Photo.Saturation",
      "Exif.Photo.Sharpness",
      "Exif.Image.PrintImageMatching",
      "Exif.Image.YCbCrPositioning",
  };
  for (auto&& filteredTag : filteredTags) {
    auto pos = prevData.findKey(ExifKey(filteredTag));
    if (pos != prevData.end()) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Exif tag " << pos->key() << " removed\n";
#endif
      prevData.erase(pos);
    }
  }

  // Add the remaining tags
  for (const auto& pos : prevData) {
    exifData_.add(pos);
  }

}  // Rw2Image::readMetadata

void Rw2Image::writeMetadata() {
  // Todo: implement me!
  throw(Error(ErrorCode::kerWritingImageFormatUnsupported, "RW2"));
}  // Rw2Image::writeMetadata

ByteOrder Rw2Parser::decode(ExifData& exifData, IptcData& iptcData, XmpData& xmpData, const byte* pData, size_t size) {
  Rw2Header rw2Header;
  return TiffParserWorker::decode(exifData, iptcData, xmpData, pData, size, Tag::pana, TiffMapping::findDecoder,
                                  &rw2Header);
}

// *************************************************************************
// free functions
Image::UniquePtr newRw2Instance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<Rw2Image>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isRw2Type(BasicIo& iIo, bool advance) {
  const int32_t len = 24;
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  Rw2Header header;
  bool rc = header.read(buf, len);
  if (!advance || !rc) {
    iIo.seek(-len, BasicIo::cur);
  }
  return rc;
}

}  // namespace Exiv2
