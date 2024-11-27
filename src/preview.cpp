// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "preview.hpp"

#include "config.h"
#include "enforce.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "jpgimage.hpp"
#include "photoshop.hpp"
#include "safe_op.hpp"
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"

#include <algorithm>
#include <climits>

namespace {
using namespace Exiv2;
using Exiv2::byte;

/*!
  @brief Compare two preview images by number of pixels, if width and height
         of both lhs and rhs are available or else by size.
         Return true if lhs is smaller than rhs.
 */
bool cmpPreviewProperties(const PreviewProperties& lhs, const PreviewProperties& rhs) {
  auto l = lhs.width_ * lhs.height_;
  auto r = rhs.width_ * rhs.height_;
  return l < r;
}

/// @brief Decode a Hex string.
DataBuf decodeHex(const byte* src, size_t srcSize);

/// @brief Decode a Base64 string.
DataBuf decodeBase64(const std::string& src);

/// @brief Decode an Illustrator thumbnail that follows after %AI7_Thumbnail.
DataBuf decodeAi7Thumbnail(const DataBuf& src);

/// @brief Create a PNM image from raw RGB data.
DataBuf makePnm(size_t width, size_t height, const DataBuf& rgb);

/*!
  Base class for image loaders. Provides virtual methods for reading properties
  and DataBuf.
 */
class Loader {
 public:
  //! Virtual destructor.
  virtual ~Loader() = default;

  Loader(const Loader&) = delete;
  Loader& operator=(const Loader&) = delete;

  //! Loader auto pointer
  using UniquePtr = std::unique_ptr<Loader>;

  //! Create a Loader subclass for requested id
  static UniquePtr create(PreviewId id, const Image& image);

  //! Check if a preview image with given params exists in the image
  [[nodiscard]] virtual bool valid() const {
    return valid_;
  }

  //! Get properties of a preview image with given params
  [[nodiscard]] virtual PreviewProperties getProperties() const;

  //! Get a buffer that contains the preview image
  [[nodiscard]] virtual DataBuf getData() const = 0;

  //! Read preview image dimensions when they are not available directly
  virtual bool readDimensions() {
    return true;
  }

  //! A number of image loaders configured in the loaderList_ table
  static PreviewId getNumLoaders();

 protected:
  //! Constructor. Sets all image properties to unknown.
  Loader(PreviewId id, const Image& image);

  //! Functions that creates a loader from given parameters
  using CreateFunc = UniquePtr (*)(PreviewId, const Image&, int);

  //! Structure to list possible loaders
  struct LoaderList {
    const char* imageMimeType_;  //!< Image type for which the loader is valid, 0 matches all images
    CreateFunc create_;          //!< Function that creates particular loader instance
    int parIdx_;                 //!< Parameter that is passed into CreateFunc
  };

  //! Table that lists possible loaders.  PreviewId is an index to this table.
  static const LoaderList loaderList_[];

  //! Identifies preview image type
  PreviewId id_;

  //! Source image reference
  const Image& image_;

  //! Preview image width
  size_t width_{0};

  //! Preview image length
  size_t height_{0};

  //! Preview image size in bytes
  size_t size_{0};

  //! True if the source image contains a preview image of given type
  bool valid_{false};
};

//! Loader for native previews
class LoaderNative : public Loader {
 public:
  //! Constructor
  LoaderNative(PreviewId id, const Image& image, int parIdx);

  //! Get properties of a preview image with given params
  [[nodiscard]] PreviewProperties getProperties() const override;

  //! Get a buffer that contains the preview image
  [[nodiscard]] DataBuf getData() const override;

  //! Read preview image dimensions
  bool readDimensions() override;

 protected:
  //! Native preview information
  NativePreview nativePreview_;
};

//! Function to create new LoaderNative
Loader::UniquePtr createLoaderNative(PreviewId id, const Image& image, int parIdx);

//! Loader for Jpeg previews that are not read into ExifData directly
class LoaderExifJpeg : public Loader {
 public:
  //! Constructor
  LoaderExifJpeg(PreviewId id, const Image& image, int parIdx);

  //! Get properties of a preview image with given params
  [[nodiscard]] PreviewProperties getProperties() const override;

  //! Get a buffer that contains the preview image
  [[nodiscard]] DataBuf getData() const override;

  //! Read preview image dimensions
  bool readDimensions() override;

 protected:
  //! Structure that lists offset/size tag pairs
  struct Param {
    const char* offsetKey_;      //!< Offset tag
    const char* sizeKey_;        //!< Size tag
    const char* baseOffsetKey_;  //!< Tag that holds base offset or 0
  };

  //! Table that holds all possible offset/size pairs. parIdx is an index to this table
  static const Param param_[];

  //! Offset value
  size_t offset_{0};
};

//! Function to create new LoaderExifJpeg
Loader::UniquePtr createLoaderExifJpeg(PreviewId id, const Image& image, int parIdx);

//! Loader for Jpeg previews that are read into ExifData
class LoaderExifDataJpeg : public Loader {
 public:
  //! Constructor
  LoaderExifDataJpeg(PreviewId id, const Image& image, int parIdx);

  //! Get properties of a preview image with given params
  [[nodiscard]] PreviewProperties getProperties() const override;

  //! Get a buffer that contains the preview image
  [[nodiscard]] DataBuf getData() const override;

  //! Read preview image dimensions
  bool readDimensions() override;

 protected:
  //! Structure that lists data/size tag pairs
  struct Param {
    const char* dataKey_;  //!< Data tag
    const char* sizeKey_;  //!< Size tag
  };

  //! Table that holds all possible data/size pairs. parIdx is an index to this table
  static const Param param_[];

  //! Key that points to the Value that contains the JPEG preview in data area
  ExifKey dataKey_;
};

//! Function to create new LoaderExifDataJpeg
Loader::UniquePtr createLoaderExifDataJpeg(PreviewId id, const Image& image, int parIdx);

//! Loader for Tiff previews - it can get image data from ExifData or image_.io() as needed
class LoaderTiff : public Loader {
 public:
  //! Constructor
  LoaderTiff(PreviewId id, const Image& image, int parIdx);

  //! Get properties of a preview image with given params
  [[nodiscard]] PreviewProperties getProperties() const override;

  //! Get a buffer that contains the preview image
  [[nodiscard]] DataBuf getData() const override;

 protected:
  //! Name of the group that contains the preview image
  const char* group_;

  //! Tag that contains image data. Possible values are "StripOffsets" or "TileOffsets"
  std::string offsetTag_;

  //! Tag that contains data sizes. Possible values are "StripByteCounts" or "TileByteCounts"
  std::string sizeTag_;

  //! Structure that lists preview groups
  struct Param {
    const char* group_;       //!< Group name
    const char* checkTag_;    //!< Tag to check or NULL
    const char* checkValue_;  //!< The preview image is valid only if the checkTag_ has this value
  };

  //! Table that holds all possible groups. parIdx is an index to this table.
  static const Param param_[];
};

//! Function to create new LoaderTiff
Loader::UniquePtr createLoaderTiff(PreviewId id, const Image& image, int parIdx);

//! Loader for JPEG previews stored in the XMP metadata
class LoaderXmpJpeg : public Loader {
 public:
  //! Constructor
  LoaderXmpJpeg(PreviewId id, const Image& image, int parIdx);

  //! Get properties of a preview image with given params
  [[nodiscard]] PreviewProperties getProperties() const override;

  //! Get a buffer that contains the preview image
  [[nodiscard]] DataBuf getData() const override;

  //! Read preview image dimensions
  bool readDimensions() override;

 protected:
  //! Preview image data
  DataBuf preview_;
};

//! Function to create new LoaderXmpJpeg
Loader::UniquePtr createLoaderXmpJpeg(PreviewId id, const Image& image, int parIdx);

// *****************************************************************************
// class member definitions

const Loader::LoaderList Loader::loaderList_[] = {
    {nullptr, createLoaderNative, 0},        {nullptr, createLoaderNative, 1},
    {nullptr, createLoaderNative, 2},        {nullptr, createLoaderNative, 3},
    {nullptr, createLoaderExifDataJpeg, 0},  {nullptr, createLoaderExifDataJpeg, 1},
    {nullptr, createLoaderExifDataJpeg, 2},  {nullptr, createLoaderExifDataJpeg, 3},
    {nullptr, createLoaderExifDataJpeg, 4},  {nullptr, createLoaderExifDataJpeg, 5},
    {nullptr, createLoaderExifDataJpeg, 6},  {nullptr, createLoaderExifDataJpeg, 7},
    {nullptr, createLoaderExifDataJpeg, 8},  {"image/x-panasonic-rw2", createLoaderExifDataJpeg, 9},
    {nullptr, createLoaderExifDataJpeg, 10}, {nullptr, createLoaderExifDataJpeg, 11},
    {nullptr, createLoaderTiff, 0},          {nullptr, createLoaderTiff, 1},
    {nullptr, createLoaderTiff, 2},          {nullptr, createLoaderTiff, 3},
    {nullptr, createLoaderTiff, 4},          {nullptr, createLoaderTiff, 5},
    {nullptr, createLoaderTiff, 6},          {"image/x-canon-cr2", createLoaderTiff, 7},
    {nullptr, createLoaderExifJpeg, 0},      {nullptr, createLoaderExifJpeg, 1},
    {nullptr, createLoaderExifJpeg, 2},      {nullptr, createLoaderExifJpeg, 3},
    {nullptr, createLoaderExifJpeg, 4},      {nullptr, createLoaderExifJpeg, 5},
    {nullptr, createLoaderExifJpeg, 6},      {"image/x-canon-cr2", createLoaderExifJpeg, 7},
    {nullptr, createLoaderExifJpeg, 8},      {nullptr, createLoaderXmpJpeg, 0},
};

const LoaderExifJpeg::Param LoaderExifJpeg::param_[] = {
    {"Exif.Image.JPEGInterchangeFormat", "Exif.Image.JPEGInterchangeFormatLength", nullptr},            // 0
    {"Exif.SubImage1.JPEGInterchangeFormat", "Exif.SubImage1.JPEGInterchangeFormatLength", nullptr},    // 1
    {"Exif.SubImage2.JPEGInterchangeFormat", "Exif.SubImage2.JPEGInterchangeFormatLength", nullptr},    // 2
    {"Exif.SubImage3.JPEGInterchangeFormat", "Exif.SubImage3.JPEGInterchangeFormatLength", nullptr},    // 3
    {"Exif.SubImage4.JPEGInterchangeFormat", "Exif.SubImage4.JPEGInterchangeFormatLength", nullptr},    // 4
    {"Exif.SubThumb1.JPEGInterchangeFormat", "Exif.SubThumb1.JPEGInterchangeFormatLength", nullptr},    // 5
    {"Exif.Image2.JPEGInterchangeFormat", "Exif.Image2.JPEGInterchangeFormatLength", nullptr},          // 6
    {"Exif.Image.StripOffsets", "Exif.Image.StripByteCounts", nullptr},                                 // 7
    {"Exif.OlympusCs.PreviewImageStart", "Exif.OlympusCs.PreviewImageLength", "Exif.MakerNote.Offset"}  // 8
};

const LoaderExifDataJpeg::Param LoaderExifDataJpeg::param_[] = {
    {"Exif.Thumbnail.JPEGInterchangeFormat", "Exif.Thumbnail.JPEGInterchangeFormatLength"},            //  0
    {"Exif.NikonPreview.JPEGInterchangeFormat", "Exif.NikonPreview.JPEGInterchangeFormatLength"},      //  1
    {"Exif.Pentax.PreviewOffset", "Exif.Pentax.PreviewLength"},                                        //  2
    {"Exif.PentaxDng.PreviewOffset", "Exif.PentaxDng.PreviewLength"},                                  //  3
    {"Exif.Minolta.ThumbnailOffset", "Exif.Minolta.ThumbnailLength"},                                  //  4
    {"Exif.SonyMinolta.ThumbnailOffset", "Exif.SonyMinolta.ThumbnailLength"},                          //  5
    {"Exif.Olympus.ThumbnailImage", nullptr},                                                          //  6
    {"Exif.Olympus2.ThumbnailImage", nullptr},                                                         //  7
    {"Exif.Minolta.Thumbnail", nullptr},                                                               //  8
    {"Exif.PanasonicRaw.PreviewImage", nullptr},                                                       //  9
    {"Exif.SamsungPreview.JPEGInterchangeFormat", "Exif.SamsungPreview.JPEGInterchangeFormatLength"},  // 10
    {"Exif.Casio2.PreviewImage", nullptr}                                                              // 11
};

const LoaderTiff::Param LoaderTiff::param_[] = {
    {"Image", "Exif.Image.NewSubfileType", "1"},          // 0
    {"SubImage1", "Exif.SubImage1.NewSubfileType", "1"},  // 1
    {"SubImage2", "Exif.SubImage2.NewSubfileType", "1"},  // 2
    {"SubImage3", "Exif.SubImage3.NewSubfileType", "1"},  // 3
    {"SubImage4", "Exif.SubImage4.NewSubfileType", "1"},  // 4
    {"SubThumb1", "Exif.SubThumb1.NewSubfileType", "1"},  // 5
    {"Thumbnail", nullptr, nullptr},                      // 6
    {"Image2", nullptr, nullptr}                          // 7
};

Loader::UniquePtr Loader::create(PreviewId id, const Image& image) {
  Loader::UniquePtr loader;
  if (id < 0 || id >= Loader::getNumLoaders())
    return loader;

  if (loaderList_[id].imageMimeType_ && std::string(loaderList_[id].imageMimeType_) != image.mimeType())
    return loader;

  loader = loaderList_[id].create_(id, image, loaderList_[id].parIdx_);
  if (!loader->valid())
    loader = nullptr;

  return loader;
}

Loader::Loader(PreviewId id, const Image& image) : id_(id), image_(image) {
}

PreviewProperties Loader::getProperties() const {
  return {"", "", size_, width_, height_, id_};
}

PreviewId Loader::getNumLoaders() {
  return static_cast<PreviewId>(std::size(loaderList_));
}

LoaderNative::LoaderNative(PreviewId id, const Image& image, int parIdx) : Loader(id, image) {
  if (0 > parIdx || static_cast<size_t>(parIdx) >= image.nativePreviews().size())
    return;
  nativePreview_ = image.nativePreviews()[parIdx];
  width_ = nativePreview_.width_;
  height_ = nativePreview_.height_;
  valid_ = true;
  if (nativePreview_.filter_.empty()) {
    size_ = nativePreview_.size_;
  } else {
    size_ = getData().size();
  }
}

Loader::UniquePtr createLoaderNative(PreviewId id, const Image& image, int parIdx) {
  return std::make_unique<LoaderNative>(id, image, parIdx);
}

PreviewProperties LoaderNative::getProperties() const {
  PreviewProperties prop = Loader::getProperties();
  prop.mimeType_ = nativePreview_.mimeType_;
  if (nativePreview_.mimeType_ == "image/jpeg") {
    prop.extension_ = ".jpg";
  } else if (nativePreview_.mimeType_ == "image/tiff") {
    prop.extension_ = ".tif";
  } else if (nativePreview_.mimeType_ == "image/x-wmf") {
    prop.extension_ = ".wmf";
  } else if (nativePreview_.mimeType_ == "image/x-portable-anymap") {
    prop.extension_ = ".pnm";
  } else {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Unknown native preview format: " << nativePreview_.mimeType_ << "\n";
#endif
    prop.extension_ = ".dat";
  }
  return prop;
}

DataBuf LoaderNative::getData() const {
  if (!valid())
    return {};

  BasicIo& io = image_.io();
  if (io.open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io.path(), strError());
  }
  IoCloser closer(io);
  const byte* data = io.mmap();
  if (io.size() < nativePreview_.position_ + nativePreview_.size_) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Invalid native preview position or size.\n";
#endif
    return {};
  }
  if (nativePreview_.filter_.empty()) {
    return {data + nativePreview_.position_, nativePreview_.size_};
  }
  if (nativePreview_.filter_ == "hex-ai7thumbnail-pnm") {
    const DataBuf ai7thumbnail = decodeHex(data + nativePreview_.position_, nativePreview_.size_);
    const DataBuf rgb = decodeAi7Thumbnail(ai7thumbnail);
    return makePnm(width_, height_, rgb);
  }
  if (nativePreview_.filter_ == "hex-irb") {
    const DataBuf psData = decodeHex(data + nativePreview_.position_, nativePreview_.size_);
    const byte* record;
    uint32_t sizeHdr = 0;
    uint32_t sizeData = 0;
    if (Photoshop::locatePreviewIrb(psData.c_data(), psData.size(), &record, sizeHdr, sizeData) != 0) {
#ifndef SUPPRESS_WARNINGS
      EXV_WARNING << "Missing preview IRB in Photoshop EPS preview.\n";
#endif
      return {};
    }
    return {record + sizeHdr + 28, sizeData - 28};
  }
  throw Error(ErrorCode::kerErrorMessage, "Invalid native preview filter: ", nativePreview_.filter_);
}

bool LoaderNative::readDimensions() {
  if (!valid())
    return false;
  if (width_ != 0 || height_ != 0)
    return true;

  const DataBuf data = getData();
  if (data.empty())
    return false;

  try {
    auto image = ImageFactory::open(data.c_data(), data.size());
    if (!image)
      return false;
    image->readMetadata();

    width_ = image->pixelWidth();
    height_ = image->pixelHeight();
  } catch (const Error& /* error */) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Invalid native preview image.\n";
#endif
    return false;
  }
  return true;
}

LoaderExifJpeg::LoaderExifJpeg(PreviewId id, const Image& image, int parIdx) : Loader(id, image) {
  const ExifData& exifData = image_.exifData();
  auto pos = exifData.findKey(ExifKey(param_[parIdx].offsetKey_));
  if (pos != exifData.end() && pos->count() > 0) {
    offset_ = pos->toUint32();
  }

  size_ = 0;
  pos = exifData.findKey(ExifKey(param_[parIdx].sizeKey_));
  if (pos != exifData.end() && pos->count() > 0) {
    size_ = pos->toUint32();
  }

  if (offset_ == 0 || size_ == 0)
    return;

  if (param_[parIdx].baseOffsetKey_) {
    pos = exifData.findKey(ExifKey(param_[parIdx].baseOffsetKey_));
    if (pos != exifData.end() && pos->count() > 0) {
      offset_ += pos->toUint32();
    }
  }

  if (Safe::add(offset_, size_) > image_.io().size())
    return;

  valid_ = true;
}

Loader::UniquePtr createLoaderExifJpeg(PreviewId id, const Image& image, int parIdx) {
  return std::make_unique<LoaderExifJpeg>(id, image, parIdx);
}

PreviewProperties LoaderExifJpeg::getProperties() const {
  PreviewProperties prop = Loader::getProperties();
  prop.mimeType_ = "image/jpeg";
  prop.extension_ = ".jpg";
  return prop;
}

DataBuf LoaderExifJpeg::getData() const {
  if (!valid())
    return {};
  BasicIo& io = image_.io();

  if (io.open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io.path(), strError());
  }
  IoCloser closer(io);

  const Exiv2::byte* base = io.mmap();

  return {base + offset_, size_};
}

bool LoaderExifJpeg::readDimensions() {
  if (!valid())
    return false;
  if (width_ || height_)
    return true;

  BasicIo& io = image_.io();

  if (io.open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io.path(), strError());
  }
  IoCloser closer(io);
  const Exiv2::byte* base = io.mmap();

  try {
    auto image = ImageFactory::open(base + offset_, size_);
    if (!image)
      return false;
    image->readMetadata();

    width_ = image->pixelWidth();
    height_ = image->pixelHeight();
  } catch (const Error& /* error */) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Invalid JPEG preview image.\n";
#endif
    return false;
  }

  return true;
}

LoaderExifDataJpeg::LoaderExifDataJpeg(PreviewId id, const Image& image, int parIdx) :
    Loader(id, image), dataKey_(param_[parIdx].dataKey_) {
  if (auto pos = image_.exifData().findKey(dataKey_); pos != image_.exifData().end()) {
    size_ = pos->sizeDataArea();  // indirect data
    if (size_ == 0 && pos->typeId() == undefined)
      size_ = pos->size();  // direct data
  }

  if (size_ == 0)
    return;

  valid_ = true;
}

Loader::UniquePtr createLoaderExifDataJpeg(PreviewId id, const Image& image, int parIdx) {
  return std::make_unique<LoaderExifDataJpeg>(id, image, parIdx);
}

PreviewProperties LoaderExifDataJpeg::getProperties() const {
  PreviewProperties prop = Loader::getProperties();
  prop.mimeType_ = "image/jpeg";
  prop.extension_ = ".jpg";
  return prop;
}

DataBuf LoaderExifDataJpeg::getData() const {
  DataBuf buf;

  if (!valid())
    return buf;

  if (auto pos = image_.exifData().findKey(dataKey_); pos != image_.exifData().end()) {
    buf = pos->dataArea();  // indirect data

    if (buf.empty()) {  // direct data
      buf = DataBuf(pos->size());
      pos->copy(buf.data(), invalidByteOrder);
    }

    buf.write_uint8(0, 0xff);  // fix Minolta thumbnails with invalid jpeg header
    return buf;
  }

  return buf;
}

bool LoaderExifDataJpeg::readDimensions() {
  if (!valid())
    return false;

  DataBuf buf = getData();
  if (buf.empty())
    return false;

  try {
    auto image = ImageFactory::open(buf.c_data(), buf.size());
    if (!image)
      return false;
    image->readMetadata();

    width_ = image->pixelWidth();
    height_ = image->pixelHeight();
  } catch (const Error& /* error */) {
    return false;
  }

  return true;
}

LoaderTiff::LoaderTiff(PreviewId id, const Image& image, int parIdx) :
    Loader(id, image), group_(param_[parIdx].group_) {
  const ExifData& exifData = image_.exifData();

  size_t offsetCount = 0;
  ExifData::const_iterator pos;

  // check if the group_ contains a preview image
  if (param_[parIdx].checkTag_) {
    pos = exifData.findKey(ExifKey(param_[parIdx].checkTag_));
    if (pos == exifData.end())
      return;
    if (param_[parIdx].checkValue_ && pos->toString() != param_[parIdx].checkValue_)
      return;
  }

  pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".StripOffsets"));
  if (pos != exifData.end()) {
    offsetTag_ = "StripOffsets";
    sizeTag_ = "StripByteCounts";
    offsetCount = pos->value().count();
  } else {
    pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".TileOffsets"));
    if (pos == exifData.end())
      return;
    offsetTag_ = "TileOffsets";
    sizeTag_ = "TileByteCounts";
    offsetCount = pos->value().count();
  }

  pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + '.' + sizeTag_));
  if (pos == exifData.end())
    return;
  if (offsetCount != pos->value().count())
    return;
  for (size_t i = 0; i < offsetCount; i++) {
    size_ += pos->toUint32(i);
  }

  if (size_ == 0)
    return;

  pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".ImageWidth"));
  if (pos != exifData.end() && pos->count() > 0) {
    width_ = pos->toUint32();
  }

  pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".ImageLength"));
  if (pos != exifData.end() && pos->count() > 0) {
    height_ = pos->toUint32();
  }

  if (width_ == 0 || height_ == 0)
    return;

  valid_ = true;
}

Loader::UniquePtr createLoaderTiff(PreviewId id, const Image& image, int parIdx) {
  return std::make_unique<LoaderTiff>(id, image, parIdx);
}

PreviewProperties LoaderTiff::getProperties() const {
  PreviewProperties prop = Loader::getProperties();
  prop.mimeType_ = "image/tiff";
  prop.extension_ = ".tif";
  return prop;
}

DataBuf LoaderTiff::getData() const {
  const ExifData& exifData = image_.exifData();

  ExifData preview;

  // copy tags
  for (auto&& pos : exifData) {
    if (pos.groupName() == group_) {
      /*
         Write only the necessary TIFF image tags
         tags that especially could cause problems are:
         "NewSubfileType" - the result is no longer a thumbnail, it is a standalone image
         "Orientation" - this tag typically appears only in the "Image" group. Deleting it ensures
                         consistent result for all previews, including JPEG
      */
      uint16_t tag = pos.tag();
      if (tag != 0x00fe && tag != 0x00ff && Internal::isTiffImageTag(tag, IfdId::ifd0Id)) {
        preview.add(ExifKey(tag, "Image"), &pos.value());
      }
    }
  }

  auto& dataValue = const_cast<Value&>(preview["Exif.Image." + offsetTag_].value());

  if (dataValue.sizeDataArea() == 0) {
    // image data are not available via exifData, read them from image_.io()
    BasicIo& io = image_.io();

    if (io.open() != 0) {
      throw Error(ErrorCode::kerDataSourceOpenFailed, io.path(), strError());
    }
    IoCloser closer(io);

    const Exiv2::byte* base = io.mmap();

    const Value& sizes = preview["Exif.Image." + sizeTag_].value();

    if (sizes.count() == dataValue.count()) {
      if (sizes.count() == 1) {
        // this saves one copying of the buffer
        uint32_t offset = dataValue.toUint32(0);
        uint32_t size = sizes.toUint32(0);
        if (Safe::add(offset, size) <= static_cast<uint32_t>(io.size()))
          dataValue.setDataArea(base + offset, size);
      } else {
        // FIXME: the buffer is probably copied twice, it should be optimized
        Internal::enforce(size_ <= io.size(), ErrorCode::kerCorruptedMetadata);
        DataBuf buf(size_);
        uint32_t idxBuf = 0;
        for (size_t i = 0; i < sizes.count(); i++) {
          uint32_t offset = dataValue.toUint32(i);
          uint32_t size = sizes.toUint32(i);

          // the size_ parameter is originally computed by summing all values inside sizes
          // see the constructor of LoaderTiff
          // But e.g in malicious files some of these values could be negative
          // That's why we check again for each step here to really make sure we don't overstep
          Internal::enforce(Safe::add(idxBuf, size) <= size_, ErrorCode::kerCorruptedMetadata);
          if (size != 0 && Safe::add(offset, size) <= static_cast<uint32_t>(io.size())) {
            std::copy_n(base + offset, size, buf.begin() + idxBuf);
          }

          idxBuf += size;
        }
        dataValue.setDataArea(buf.c_data(), buf.size());
      }
    }
  }

  // Fix compression value in the CR2 IFD2 image
  if (0 == strcmp(group_, "Image2") && image_.mimeType() == "image/x-canon-cr2") {
    preview["Exif.Image.Compression"] = static_cast<uint16_t>(1);
  }

  // write new image
  MemIo mio;
  IptcData emptyIptc;
  XmpData emptyXmp;
  TiffParser::encode(mio, nullptr, 0, Exiv2::littleEndian, preview, emptyIptc, emptyXmp);
  return {mio.mmap(), mio.size()};
}

LoaderXmpJpeg::LoaderXmpJpeg(PreviewId id, const Image& image, int parIdx) : Loader(id, image) {
  (void)parIdx;

  const XmpData& xmpData = image_.xmpData();

  std::string prefix = "xmpGImg";
  if (xmpData.findKey(XmpKey("Xmp.xmp.Thumbnails[1]/xapGImg:image")) != xmpData.end()) {
    prefix = "xapGImg";
  }

  auto imageDatum = xmpData.findKey(XmpKey("Xmp.xmp.Thumbnails[1]/" + prefix + ":image"));
  if (imageDatum == xmpData.end())
    return;
  auto formatDatum = xmpData.findKey(XmpKey("Xmp.xmp.Thumbnails[1]/" + prefix + ":format"));
  if (formatDatum == xmpData.end())
    return;
  auto widthDatum = xmpData.findKey(XmpKey("Xmp.xmp.Thumbnails[1]/" + prefix + ":width"));
  if (widthDatum == xmpData.end())
    return;
  auto heightDatum = xmpData.findKey(XmpKey("Xmp.xmp.Thumbnails[1]/" + prefix + ":height"));
  if (heightDatum == xmpData.end())
    return;

  if (formatDatum->toString() != "JPEG")
    return;

  width_ = widthDatum->toUint32();
  height_ = heightDatum->toUint32();
  preview_ = decodeBase64(imageDatum->toString());
  size_ = preview_.size();
  valid_ = true;
}

Loader::UniquePtr createLoaderXmpJpeg(PreviewId id, const Image& image, int parIdx) {
  return std::make_unique<LoaderXmpJpeg>(id, image, parIdx);
}

PreviewProperties LoaderXmpJpeg::getProperties() const {
  PreviewProperties prop = Loader::getProperties();
  prop.mimeType_ = "image/jpeg";
  prop.extension_ = ".jpg";
  return prop;
}

DataBuf LoaderXmpJpeg::getData() const {
  if (!valid())
    return {};
  return {preview_.c_data(), preview_.size()};
}

bool LoaderXmpJpeg::readDimensions() {
  return valid();
}

DataBuf decodeHex(const byte* src, size_t srcSize) {
  // create decoding table
  byte invalid = 16;
  auto decodeHexTable = std::vector<byte>(256, invalid);
  for (byte i = 0; i < 10; i++)
    decodeHexTable[static_cast<byte>('0') + i] = i;
  for (byte i = 0; i < 6; i++)
    decodeHexTable[static_cast<byte>('A') + i] = i + 10;
  for (byte i = 0; i < 6; i++)
    decodeHexTable[static_cast<byte>('a') + i] = i + 10;

  // calculate dest size
  long validSrcSize = 0;
  for (size_t srcPos = 0; srcPos < srcSize; srcPos++) {
    if (decodeHexTable[src[srcPos]] != invalid)
      validSrcSize++;
  }
  const size_t destSize = validSrcSize / 2;

  // allocate dest buffer
  DataBuf dest(destSize);

  // decode
  for (size_t srcPos = 0, destPos = 0; destPos < destSize; destPos++) {
    byte buffer = 0;
    for (int bufferPos = 1; bufferPos >= 0 && srcPos < srcSize; srcPos++) {
      byte srcValue = decodeHexTable[src[srcPos]];
      if (srcValue == invalid)
        continue;
      buffer |= srcValue << (bufferPos * 4);
      bufferPos--;
    }
    dest.write_uint8(destPos, buffer);
  }
  return dest;
}

const char encodeBase64Table[64 + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

DataBuf decodeBase64(const std::string& src) {
  DataBuf dest;
  // create decoding table
  unsigned long invalid = 64;
  auto decodeBase64Table = std::vector<unsigned long>(256, invalid);
  for (unsigned long i = 0; i < 64; i++)
    decodeBase64Table[static_cast<unsigned char>(encodeBase64Table[i])] = i;

  // calculate dest size
  auto validSrcSize = static_cast<unsigned long>(
      std::count_if(src.begin(), src.end(), [&](unsigned char c) { return decodeBase64Table.at(c) != invalid; }));
  if (validSrcSize > ULONG_MAX / 3)
    return dest;  // avoid integer overflow
  const unsigned long destSize = (validSrcSize * 3) / 4;

  // allocate dest buffer
  dest = DataBuf(destSize);

  // decode
  for (unsigned long srcPos = 0, destPos = 0; destPos < destSize;) {
    unsigned long buffer = 0;
    for (int bufferPos = 3; bufferPos >= 0 && srcPos < src.size(); srcPos++) {
      unsigned long srcValue = decodeBase64Table[static_cast<unsigned char>(src[srcPos])];
      if (srcValue == invalid)
        continue;
      buffer |= srcValue << (bufferPos * 6);
      bufferPos--;
    }
    for (int bufferPos = 2; bufferPos >= 0 && destPos < destSize; bufferPos--, destPos++) {
      dest.write_uint8(destPos, static_cast<byte>((buffer >> (bufferPos * 8)) & 0xFF));
    }
  }
  return dest;
}

DataBuf decodeAi7Thumbnail(const DataBuf& src) {
  const byte* colorTable = src.c_data();
  const size_t colorTableSize = 256 * 3;
  if (src.size() < colorTableSize) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Invalid size of AI7 thumbnail: " << src.size() << "\n";
#endif
    return {};
  }
  const byte* imageData = src.c_data(colorTableSize);
  const size_t imageDataSize = src.size() - colorTableSize;
  const bool rle = (imageDataSize >= 3 && imageData[0] == 'R' && imageData[1] == 'L' && imageData[2] == 'E');
  std::string dest;
  for (size_t i = rle ? 3 : 0; i < imageDataSize;) {
    byte num = 1;
    byte value = imageData[i++];
    if (rle && value == 0xFD) {
      if (i >= imageDataSize) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Unexpected end of image data at AI7 thumbnail.\n";
#endif
        return {};
      }
      value = imageData[i++];
      if (value != 0xFD) {
        if (i >= imageDataSize) {
#ifndef SUPPRESS_WARNINGS
          EXV_WARNING << "Unexpected end of image data at AI7 thumbnail.\n";
#endif
          return {};
        }
        num = value;
        value = imageData[i++];
      }
    }
    for (; num != 0; num--) {
      dest.append(reinterpret_cast<const char*>(colorTable + (3 * value)), 3);
    }
  }
  return {reinterpret_cast<const byte*>(dest.data()), dest.size()};
}

DataBuf makePnm(size_t width, size_t height, const DataBuf& rgb) {
  DataBuf dest;
  if (size_t expectedSize = width * height * 3UL; rgb.size() != expectedSize) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Invalid size of preview data. Expected " << expectedSize << " bytes, got " << rgb.size()
                << " bytes.\n";
#endif
    return dest;
  }

  const std::string header = "P6\n" + std::to_string(width) + " " + std::to_string(height) + "\n255\n";
  const auto headerBytes = reinterpret_cast<const byte*>(header.data());

  dest = DataBuf(header.size() + rgb.size());
  std::copy_n(headerBytes, header.size(), dest.begin());
  std::copy_n(rgb.c_data(), rgb.size(), dest.begin() + header.size());
  return dest;
}

}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
PreviewImage::PreviewImage(PreviewProperties properties, DataBuf&& data) :
    properties_(std::move(properties)), preview_(std::move(data)) {
}

PreviewImage::PreviewImage(const PreviewImage& rhs) : properties_(rhs.properties_), preview_(rhs.pData(), rhs.size()) {
}

PreviewImage& PreviewImage::operator=(const PreviewImage& rhs) {
  if (this == &rhs)
    return *this;
  properties_ = rhs.properties_;
  preview_ = DataBuf(rhs.pData(), rhs.size());
  return *this;
}

#ifdef EXV_ENABLE_FILESYSTEM
size_t PreviewImage::writeFile(const std::string& path) const {
  std::string name = path + extension();
  // Todo: Creating a DataBuf here unnecessarily copies the memory
  DataBuf buf(pData(), size());
  return Exiv2::writeFile(buf, name);
}
#endif

DataBuf PreviewImage::copy() const {
  return {pData(), size()};
}

const byte* PreviewImage::pData() const {
  return preview_.c_data();
}

uint32_t PreviewImage::size() const {
  return static_cast<uint32_t>(preview_.size());
}

std::string PreviewImage::mimeType() const {
  return properties_.mimeType_;
}

std::string PreviewImage::extension() const {
  return properties_.extension_;
}

size_t PreviewImage::width() const {
  return properties_.width_;
}

size_t PreviewImage::height() const {
  return properties_.height_;
}

PreviewId PreviewImage::id() const {
  return properties_.id_;
}

PreviewManager::PreviewManager(const Image& image) : image_(image) {
}

PreviewPropertiesList PreviewManager::getPreviewProperties() const {
  PreviewPropertiesList list;
  // go through the loader table and store all successfully created loaders in the list
  for (PreviewId id = 0; id < Loader::getNumLoaders(); ++id) {
    auto loader = Loader::create(id, image_);
    if (loader && loader->readDimensions()) {
      PreviewProperties props = loader->getProperties();
      DataBuf buf = loader->getData();  // #16 getPreviewImage()
      props.size_ = buf.size();         //     update the size
      list.push_back(std::move(props));
    }
  }
  std::sort(list.begin(), list.end(), cmpPreviewProperties);
  return list;
}

PreviewImage PreviewManager::getPreviewImage(const PreviewProperties& properties) const {
  auto loader = Loader::create(properties.id_, image_);
  DataBuf buf;
  if (loader) {
    buf = loader->getData();
  }

  return {properties, std::move(buf)};
}
}  // namespace Exiv2
