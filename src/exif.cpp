// SPDX-License-Identifier: GPL-2.0-or-later

/*
  File:      exif.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
// included header files
#include "exif.hpp"

#include "basicio.hpp"
#include "config.h"
#include "error.hpp"
#include "metadatum.hpp"
#include "safe_op.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "tiffcomposite_int.hpp"  // for Tag::root
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "types.hpp"
#include "value.hpp"

// + standard includes
#include <algorithm>
#include <array>
#include <cstdio>
#include <iostream>
#include <utility>

// *****************************************************************************
namespace {
//! Unary predicate that matches a Exifdatum with a given key
class FindExifdatumByKey {
 public:
  //! Constructor, initializes the object with the key to look for
  explicit FindExifdatumByKey(std::string key) : key_(std::move(key)) {
  }
  /*!
    @brief Returns true if the key of \em exifdatum is equal
           to that of the object.
  */
  bool operator()(const Exiv2::Exifdatum& exifdatum) const {
    return key_ == exifdatum.key();
  }

 private:
  std::string key_;
};  // class FindExifdatumByKey

/*!
  @brief Exif %Thumbnail image. This abstract base class provides the
         interface for the thumbnail image that is optionally embedded in
         the Exif data. This class is used internally by ExifData, it is
         probably not useful for a client as a standalone class.  Instead,
         use an instance of ExifData to access the Exif thumbnail image.
 */
class Thumbnail {
 public:
  //! Shortcut for a %Thumbnail auto pointer.
  using UniquePtr = std::unique_ptr<Thumbnail>;

  //! @name Creators
  //@{
  //! Virtual destructor
  virtual ~Thumbnail() = default;
  //@}

  //! Factory function to create a thumbnail for the Exif metadata provided.
  static UniquePtr create(const Exiv2::ExifData& exifData);

  //! @name Accessors
  //@{
  /*!
    @brief Return the thumbnail image in a %DataBuf. The caller owns the
           data buffer and %DataBuf ensures that it will be deleted.
   */
  [[nodiscard]] virtual Exiv2::DataBuf copy(const Exiv2::ExifData& exifData) const = 0;
  /*!
    @brief Return the MIME type of the thumbnail ("image/tiff" or
           "image/jpeg").
   */
  [[nodiscard]] virtual const char* mimeType() const = 0;
  /*!
    @brief Return the file extension for the format of the thumbnail
           (".tif", ".jpg").
   */
  [[nodiscard]] virtual const char* extension() const = 0;
  //@}

};  // class Thumbnail

//! Exif thumbnail image in TIFF format
class TiffThumbnail : public Thumbnail {
 public:
  //! Shortcut for a %TiffThumbnail auto pointer.
  using UniquePtr = std::unique_ptr<TiffThumbnail>;

  //! @name Accessors
  //@{
  [[nodiscard]] Exiv2::DataBuf copy(const Exiv2::ExifData& exifData) const override;
  [[nodiscard]] const char* mimeType() const override;
  [[nodiscard]] const char* extension() const override;
  //@}

};  // class TiffThumbnail

//! Exif thumbnail image in JPEG format
class JpegThumbnail : public Thumbnail {
 public:
  //! Shortcut for a %JpegThumbnail auto pointer.
  using UniquePtr = std::unique_ptr<JpegThumbnail>;

  //! @name Accessors
  //@{
  [[nodiscard]] Exiv2::DataBuf copy(const Exiv2::ExifData& exifData) const override;
  [[nodiscard]] const char* mimeType() const override;
  [[nodiscard]] const char* extension() const override;
  //@}

};  // class JpegThumbnail

//! Helper function to sum all components of the value of a metadatum
int64_t sumToLong(const Exiv2::Exifdatum& md);

//! Helper function to delete all tags of a specific IFD from the metadata.
void eraseIfd(Exiv2::ExifData& ed, Exiv2::IfdId ifdId);

}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
using namespace Internal;

/*!
  @brief Set the value of \em exifDatum to \em value. If the object already
         has a value, it is replaced. Otherwise a new ValueType\<T\> value
         is created and set to \em value.

  This is a helper function, called from Exifdatum members. It is meant to
  be used with T = (u)int16_t, (u)int32_t or (U)Rational. Do not use directly.
*/
template <typename T>
Exiv2::Exifdatum& setValue(Exiv2::Exifdatum& exifDatum, const T& value) {
  auto v = std::make_unique<Exiv2::ValueType<T>>();
  v->value_.push_back(value);
  exifDatum.value_ = std::move(v);
  return exifDatum;
}

Exifdatum::Exifdatum(const ExifKey& key, const Value* pValue) : key_(key.clone()) {
  if (pValue)
    value_ = pValue->clone();
}

Exifdatum::Exifdatum(const Exifdatum& rhs) : Metadatum(rhs) {
  if (rhs.key_)
    key_ = rhs.key_->clone();  // deep copy
  if (rhs.value_)
    value_ = rhs.value_->clone();  // deep copy
}

std::ostream& Exifdatum::write(std::ostream& os, const ExifData* pMetadata) const {
  if (value().count() == 0)
    return os;

  PrintFct fct = printValue;
  // be careful with comments (User.Photo.UserComment, GPSAreaInfo etc).
  if (auto ti = Internal::tagInfo(tag(), ifdId())) {
    fct = ti->printFct_;
    if (ti->typeId_ == comment) {
      os << value().toString();
      fct = nullptr;
    }
  }
  if (fct) {
    // https://github.com/Exiv2/exiv2/issues/1706
    // Sometimes the type of the value doesn't match what the
    // print function expects. (The expected types are stored
    // in the TagInfo tables, but they are not enforced when the
    // metadata is parsed.) These type mismatches can sometimes
    // cause a std::out_of_range exception to be thrown.
    try {
      fct(os, value(), pMetadata);
    } catch (const std::out_of_range&) {
      os << "Bad value";
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Caught std::out_of_range exception in Exifdatum::write().\n";
#endif
    }
  }
  return os;
}

const Value& Exifdatum::value() const {
  if (!value_)
    throw Error(ErrorCode::kerValueNotSet, key());
  return *value_;
}

Exifdatum& Exifdatum::operator=(const Exifdatum& rhs) {
  if (this == &rhs)
    return *this;
  Metadatum::operator=(rhs);

  key_.reset();
  if (rhs.key_)
    key_ = rhs.key_->clone();  // deep copy

  value_.reset();
  if (rhs.value_)
    value_ = rhs.value_->clone();  // deep copy

  return *this;
}  // Exifdatum::operator=

Exifdatum& Exifdatum::operator=(const std::string& value) {
  setValue(value);
  return *this;
}

Exifdatum& Exifdatum::operator=(const uint16_t& value) {
  return Exiv2::setValue(*this, value);
}

Exifdatum& Exifdatum::operator=(const uint32_t& value) {
  return Exiv2::setValue(*this, value);
}

Exifdatum& Exifdatum::operator=(const URational& value) {
  return Exiv2::setValue(*this, value);
}

Exifdatum& Exifdatum::operator=(const int16_t& value) {
  return Exiv2::setValue(*this, value);
}

Exifdatum& Exifdatum::operator=(const int32_t& value) {
  return Exiv2::setValue(*this, value);
}

Exifdatum& Exifdatum::operator=(const Rational& value) {
  return Exiv2::setValue(*this, value);
}

Exifdatum& Exifdatum::operator=(const Value& value) {
  setValue(&value);
  return *this;
}

void Exifdatum::setValue(const Value* pValue) {
  value_.reset();
  if (pValue)
    value_ = pValue->clone();
}

int Exifdatum::setValue(const std::string& value) {
  if (!value_) {
    TypeId type = key_->defaultTypeId();
    value_ = Value::create(type);
  }
  return value_->read(value);
}

int Exifdatum::setDataArea(const byte* buf, size_t len) const {
  return value_ ? value_->setDataArea(buf, len) : -1;
}

std::string Exifdatum::key() const {
  return key_ ? key_->key() : "";
}

const char* Exifdatum::familyName() const {
  return key_ ? key_->familyName() : "";
}

std::string Exifdatum::groupName() const {
  return key_ ? key_->groupName() : "";
}

std::string Exifdatum::tagName() const {
  return key_ ? key_->tagName() : "";
}

std::string Exifdatum::tagLabel() const {
  return key_ ? key_->tagLabel() : "";
}

std::string Exifdatum::tagDesc() const {
  return key_ ? key_->tagDesc() : "";
}

uint16_t Exifdatum::tag() const {
  return key_ ? key_->tag() : 0xffff;
}

IfdId Exifdatum::ifdId() const {
  return key_ ? key_->ifdId() : IfdId::ifdIdNotSet;
}

const char* Exifdatum::ifdName() const {
  return key_ ? Internal::ifdName(key_->ifdId()) : "";
}

int Exifdatum::idx() const {
  return key_ ? key_->idx() : 0;
}

size_t Exifdatum::copy(byte* buf, ByteOrder byteOrder) const {
  return value_ ? value_->copy(buf, byteOrder) : 0;
}

TypeId Exifdatum::typeId() const {
  return value_ ? value_->typeId() : invalidTypeId;
}

const char* Exifdatum::typeName() const {
  return TypeInfo::typeName(typeId());
}

size_t Exifdatum::typeSize() const {
  return TypeInfo::typeSize(typeId());
}

size_t Exifdatum::count() const {
  return value_ ? value_->count() : 0;
}

size_t Exifdatum::size() const {
  return value_ ? value_->size() : 0;
}

std::string Exifdatum::toString() const {
  return value_ ? value_->toString() : "";
}

std::string Exifdatum::toString(size_t n) const {
  return value_ ? value_->toString(n) : "";
}

int64_t Exifdatum::toInt64(size_t n) const {
  return value_ ? value_->toInt64(n) : -1;
}

float Exifdatum::toFloat(size_t n) const {
  return value_ ? value_->toFloat(n) : -1;
}

Rational Exifdatum::toRational(size_t n) const {
  return value_ ? value_->toRational(n) : Rational(-1, 1);
}

Value::UniquePtr Exifdatum::getValue() const {
  return value_ ? value_->clone() : nullptr;
}

size_t Exifdatum::sizeDataArea() const {
  return value_ ? value_->sizeDataArea() : 0;
}

DataBuf Exifdatum::dataArea() const {
  return value_ ? value_->dataArea() : DataBuf(nullptr, 0);
}

ExifThumbC::ExifThumbC(const ExifData& exifData) : exifData_(exifData) {
}

DataBuf ExifThumbC::copy() const {
  auto thumbnail = Thumbnail::create(exifData_);
  if (!thumbnail)
    return {};
  return thumbnail->copy(exifData_);
}

size_t ExifThumbC::writeFile(const std::string& path) const {
  auto thumbnail = Thumbnail::create(exifData_);
  if (!thumbnail)
    return 0;

  std::string name = path + thumbnail->extension();
  DataBuf buf(thumbnail->copy(exifData_));
  if (buf.empty())
    return 0;

  return Exiv2::writeFile(buf, name);
}

const char* ExifThumbC::mimeType() const {
  auto thumbnail = Thumbnail::create(exifData_);
  if (!thumbnail)
    return "";
  return thumbnail->mimeType();
}

const char* ExifThumbC::extension() const {
  auto thumbnail = Thumbnail::create(exifData_);
  if (!thumbnail)
    return "";
  return thumbnail->extension();
}

ExifThumb::ExifThumb(ExifData& exifData) : ExifThumbC(exifData), exifData_(exifData) {
}

void ExifThumb::setJpegThumbnail(const std::string& path, URational xres, URational yres, uint16_t unit) {
  DataBuf thumb = readFile(path);  // may throw
  setJpegThumbnail(thumb.c_data(), thumb.size(), xres, yres, unit);
}

void ExifThumb::setJpegThumbnail(const byte* buf, size_t size, URational xres, URational yres, uint16_t unit) {
  setJpegThumbnail(buf, size);
  exifData_["Exif.Thumbnail.XResolution"] = xres;
  exifData_["Exif.Thumbnail.YResolution"] = yres;
  exifData_["Exif.Thumbnail.ResolutionUnit"] = unit;
}

void ExifThumb::setJpegThumbnail(const std::string& path) {
  DataBuf thumb = readFile(path);  // may throw
  setJpegThumbnail(thumb.c_data(), thumb.size());
}

void ExifThumb::setJpegThumbnail(const byte* buf, size_t size) {
  exifData_["Exif.Thumbnail.Compression"] = static_cast<uint16_t>(6);
  Exifdatum& format = exifData_["Exif.Thumbnail.JPEGInterchangeFormat"];
  format = static_cast<uint32_t>(0);
  format.setDataArea(buf, size);
  exifData_["Exif.Thumbnail.JPEGInterchangeFormatLength"] = static_cast<uint32_t>(size);
}

void ExifThumb::erase() {
  eraseIfd(exifData_, IfdId::ifd1Id);
}

Exifdatum& ExifData::operator[](const std::string& key) {
  ExifKey exifKey(key);
  auto pos = findKey(exifKey);
  if (pos == end()) {
    exifMetadata_.emplace_back(exifKey);
    return exifMetadata_.back();
  }
  return *pos;
}

void ExifData::add(const ExifKey& key, const Value* pValue) {
  add(Exifdatum(key, pValue));
}

void ExifData::add(const Exifdatum& exifdatum) {
  // allow duplicates
  exifMetadata_.push_back(exifdatum);
}

ExifData::const_iterator ExifData::findKey(const ExifKey& key) const {
  return std::find_if(exifMetadata_.begin(), exifMetadata_.end(), FindExifdatumByKey(key.key()));
}

ExifData::iterator ExifData::findKey(const ExifKey& key) {
  return std::find_if(exifMetadata_.begin(), exifMetadata_.end(), FindExifdatumByKey(key.key()));
}

void ExifData::clear() {
  exifMetadata_.clear();
}

void ExifData::sortByKey() {
  exifMetadata_.sort(cmpMetadataByKey);
}

void ExifData::sortByTag() {
  exifMetadata_.sort(cmpMetadataByTag);
}

ExifData::iterator ExifData::erase(ExifData::iterator beg, ExifData::iterator end) {
  return exifMetadata_.erase(beg, end);
}

ExifData::iterator ExifData::erase(ExifData::iterator pos) {
  return exifMetadata_.erase(pos);
}

ByteOrder ExifParser::decode(ExifData& exifData, const byte* pData, size_t size) {
  IptcData iptcData;
  XmpData xmpData;
  ByteOrder bo = TiffParser::decode(exifData, iptcData, xmpData, pData, size);
#ifndef SUPPRESS_WARNINGS
  if (!iptcData.empty()) {
    EXV_WARNING << "Ignoring IPTC information encoded in the Exif data.\n";
  }
  if (!xmpData.empty()) {
    EXV_WARNING << "Ignoring XMP information encoded in the Exif data.\n";
  }
#endif
  return bo;
}

//! @cond IGNORE
enum Ptt { pttLen, pttTag, pttIfd };
//! @endcond

WriteMethod ExifParser::encode(Blob& blob, const byte* pData, size_t size, ByteOrder byteOrder, ExifData& exifData) {
  // Delete IFD0 tags that are "not recorded" in compressed images
  // Reference: Exif 2.2 specs, 4.6.8 Tag Support Levels, section A
  static constexpr auto filteredIfd0Tags = std::array{
      "Exif.Image.PhotometricInterpretation",
      "Exif.Image.StripOffsets",
      "Exif.Image.RowsPerStrip",
      "Exif.Image.StripByteCounts",
      "Exif.Image.JPEGInterchangeFormat",
      "Exif.Image.JPEGInterchangeFormatLength",
      "Exif.Image.SubIFDs",
      // Issue 981.  Never allow manufactured data to be written
      "Exif.Canon.AFInfoSize",
      "Exif.Canon.AFAreaMode",
      "Exif.Canon.AFNumPoints",
      "Exif.Canon.AFValidPoints",
      "Exif.Canon.AFCanonImageWidth",
      "Exif.Canon.AFCanonImageHeight",
      "Exif.Canon.AFImageWidth",
      "Exif.Canon.AFImageHeight",
      "Exif.Canon.AFAreaWidths",
      "Exif.Canon.AFAreaHeights",
      "Exif.Canon.AFXPositions",
      "Exif.Canon.AFYPositions",
      "Exif.Canon.AFPointsInFocus",
      "Exif.Canon.AFPointsSelected",
      "Exif.Canon.AFPointsUnusable",
      "Exif.Canon.AFFineRotation",
  };
  for (auto&& filteredIfd0Tag : filteredIfd0Tags) {
    auto pos = exifData.findKey(ExifKey(filteredIfd0Tag));
    if (pos != exifData.end()) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Warning: Exif tag " << pos->key() << " not encoded\n";
#endif
      exifData.erase(pos);
    }
  }

  // Delete IFDs which do not occur in JPEGs
  static constexpr auto filteredIfds = std::array{
      IfdId::subImage1Id, IfdId::subImage2Id, IfdId::subImage3Id, IfdId::subImage4Id, IfdId::subImage5Id,
      IfdId::subImage6Id, IfdId::subImage7Id, IfdId::subImage8Id, IfdId::subImage9Id, IfdId::subThumb1Id,
      IfdId::panaRawId,   IfdId::ifd2Id,      IfdId::ifd3Id,
  };
  for (auto&& filteredIfd : filteredIfds) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Warning: Exif IFD " << filteredIfd << " not encoded\n";
#endif
    eraseIfd(exifData, filteredIfd);
  }

  // IPTC and XMP are stored elsewhere, not in the Exif APP1 segment.
  IptcData emptyIptc;
  XmpData emptyXmp;

  // Encode and check if the result fits into a JPEG Exif APP1 segment
  MemIo mio1;
  TiffHeader header(byteOrder, 0x00000008, false);
  WriteMethod wm = TiffParserWorker::encode(mio1, pData, size, exifData, emptyIptc, emptyXmp, Tag::root,
                                            TiffMapping::findEncoder, &header, nullptr);
  if (mio1.size() <= 65527) {
    append(blob, mio1.mmap(), mio1.size());
    return wm;
  }

  // If it doesn't fit, remove additional tags

  // Delete preview tags if the preview is larger than 32kB.
  // Todo: Enhance preview classes to be able to write and delete previews and use that instead.
  // Table must be sorted by preview, the first tag in each group is the size
  static constexpr auto filteredPvTags = std::array{
      std::pair(pttLen, "Exif.Minolta.ThumbnailLength"),
      std::pair(pttTag, "Exif.Minolta.ThumbnailOffset"),
      std::pair(pttLen, "Exif.Minolta.Thumbnail"),
      std::pair(pttLen, "Exif.NikonPreview.JPEGInterchangeFormatLength"),
      std::pair(pttIfd, "NikonPreview"),
      std::pair(pttLen, "Exif.Olympus.ThumbnailLength"),
      std::pair(pttTag, "Exif.Olympus.ThumbnailOffset"),
      std::pair(pttLen, "Exif.Olympus.ThumbnailImage"),
      std::pair(pttLen, "Exif.Olympus.Thumbnail"),
      std::pair(pttLen, "Exif.Olympus2.ThumbnailLength"),
      std::pair(pttTag, "Exif.Olympus2.ThumbnailOffset"),
      std::pair(pttLen, "Exif.Olympus2.ThumbnailImage"),
      std::pair(pttLen, "Exif.Olympus2.Thumbnail"),
      std::pair(pttLen, "Exif.OlympusCs.PreviewImageLength"),
      std::pair(pttTag, "Exif.OlympusCs.PreviewImageStart"),
      std::pair(pttTag, "Exif.OlympusCs.PreviewImageValid"),
      std::pair(pttLen, "Exif.Pentax.PreviewLength"),
      std::pair(pttTag, "Exif.Pentax.PreviewOffset"),
      std::pair(pttTag, "Exif.Pentax.PreviewResolution"),
      std::pair(pttLen, "Exif.PentaxDng.PreviewLength"),
      std::pair(pttTag, "Exif.PentaxDng.PreviewOffset"),
      std::pair(pttTag, "Exif.PentaxDng.PreviewResolution"),
      std::pair(pttLen, "Exif.SamsungPreview.JPEGInterchangeFormatLength"),
      std::pair(pttIfd, "SamsungPreview"),
      std::pair(pttLen, "Exif.Thumbnail.StripByteCounts"),
      std::pair(pttIfd, "Thumbnail"),
      std::pair(pttLen, "Exif.Thumbnail.JPEGInterchangeFormatLength"),
      std::pair(pttIfd, "Thumbnail"),
  };
  bool delTags = false;
  for (auto&& [ptt, key] : filteredPvTags) {
    switch (ptt) {
      case pttLen: {
        delTags = false;
        if (auto pos = exifData.findKey(ExifKey(key)); pos != exifData.end() && sumToLong(*pos) > 32768) {
          delTags = true;
#ifndef SUPPRESS_WARNINGS
          EXV_WARNING << "Exif tag " << pos->key() << " not encoded\n";
#endif
          exifData.erase(pos);
        }
        break;
      }
      case pttTag: {
        if (delTags) {
          if (auto pos = exifData.findKey(ExifKey(key)); pos != exifData.end()) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "Exif tag " << pos->key() << " not encoded\n";
#endif
            exifData.erase(pos);
          }
        }
        break;
      }
      case pttIfd:
        if (delTags) {
#ifndef SUPPRESS_WARNINGS
          EXV_WARNING << "Exif IFD " << key << " not encoded\n";
#endif
          eraseIfd(exifData, Internal::groupId(key));
        }
        break;
    }
  }

  // Delete unknown tags larger than 4kB and known tags larger than 20kB.
  for (auto tag_iter = exifData.begin(); tag_iter != exifData.end();) {
    if ((tag_iter->size() > 4096 && tag_iter->tagName().substr(0, 2) == "0x") || tag_iter->size() > 20480) {
#ifndef SUPPRESS_WARNINGS
      EXV_WARNING << "Exif tag " << tag_iter->key() << " not encoded\n";
#endif
      tag_iter = exifData.erase(tag_iter);
    } else {
      ++tag_iter;
    }
  }

  // Encode the remaining Exif tags again, don't care if it fits this time
  MemIo mio2;
  wm = TiffParserWorker::encode(mio2, pData, size, exifData, emptyIptc, emptyXmp, Tag::root, TiffMapping::findEncoder,
                                &header, nullptr);
  append(blob, mio2.mmap(), mio2.size());
#ifdef EXIV2_DEBUG_MESSAGES
  if (wm == wmIntrusive) {
    std::cerr << "SIZE OF EXIF DATA IS " << std::dec << mio2.size() << " BYTES\n";
  } else {
    std::cerr << "SIZE DOESN'T MATTER, NON-INTRUSIVE WRITING USED\n";
  }
#endif
  return wm;

}  // ExifParser::encode

}  // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {
//! @cond IGNORE
Thumbnail::UniquePtr Thumbnail::create(const Exiv2::ExifData& exifData) {
  const Exiv2::ExifKey k1("Exif.Thumbnail.Compression");
  auto pos = exifData.findKey(k1);
  if (pos != exifData.end()) {
    if (pos->count() == 0)
      return nullptr;
    if (pos->toInt64() == 6)
      return std::make_unique<JpegThumbnail>();
    return std::make_unique<TiffThumbnail>();
  }

  const Exiv2::ExifKey k2("Exif.Thumbnail.JPEGInterchangeFormat");
  pos = exifData.findKey(k2);
  if (pos != exifData.end())
    return std::make_unique<JpegThumbnail>();
  return nullptr;
}

const char* TiffThumbnail::mimeType() const {
  return "image/tiff";
}

const char* TiffThumbnail::extension() const {
  return ".tif";
}

Exiv2::DataBuf TiffThumbnail::copy(const Exiv2::ExifData& exifData) const {
  Exiv2::ExifData thumb;
  // Copy all Thumbnail (IFD1) tags from exifData to Image (IFD0) tags in thumb
  for (auto&& i : exifData) {
    if (i.groupName() == "Thumbnail") {
      std::string key = "Exif.Image." + i.tagName();
      thumb.add(Exiv2::ExifKey(key), &i.value());
    }
  }

  Exiv2::MemIo io;
  Exiv2::IptcData emptyIptc;
  Exiv2::XmpData emptyXmp;
  Exiv2::TiffParser::encode(io, nullptr, 0, Exiv2::littleEndian, thumb, emptyIptc, emptyXmp);
  return io.read(io.size());
}

const char* JpegThumbnail::mimeType() const {
  return "image/jpeg";
}

const char* JpegThumbnail::extension() const {
  return ".jpg";
}

Exiv2::DataBuf JpegThumbnail::copy(const Exiv2::ExifData& exifData) const {
  Exiv2::ExifKey key("Exif.Thumbnail.JPEGInterchangeFormat");
  auto format = exifData.findKey(key);
  if (format == exifData.end())
    return {};
  return format->dataArea();
}

int64_t sumToLong(const Exiv2::Exifdatum& md) {
  int64_t sum = 0;
  for (size_t i = 0; i < md.count(); ++i) {
    sum = Safe::add(sum, md.toInt64(i));
  }
  return sum;
}

void eraseIfd(Exiv2::ExifData& ed, Exiv2::IfdId ifdId) {
  ed.erase(std::remove_if(ed.begin(), ed.end(), Exiv2::FindExifdatum(ifdId)), ed.end());
}
//! @endcond
}  // namespace
