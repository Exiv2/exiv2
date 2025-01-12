// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "tags.hpp"

#include "canonmn_int.hpp"
#include "casiomn_int.hpp"
#include "convert.hpp"
#include "error.hpp"
#include "i18n.h"  // NLS support.
#include "tags_int.hpp"
#include "types.hpp"

// *****************************************************************************
// class member definitions
namespace Exiv2 {
using namespace Internal;

//! List of all defined Exif sections.
constexpr SectionInfo sectionInfo[] = {
    {SectionId::sectionIdNotSet, "(UnknownSection)", N_("Unknown section")},
    {SectionId::imgStruct, "ImageStructure", N_("Image data structure")},
    {SectionId::recOffset, "RecordingOffset", N_("Recording offset")},
    {SectionId::imgCharacter, "ImageCharacteristics", N_("Image data characteristics")},
    {SectionId::otherTags, "OtherTags", N_("Other data")},
    {SectionId::exifFormat, "ExifFormat", N_("Exif data structure")},
    {SectionId::exifVersion, "ExifVersion", N_("Exif version")},
    {SectionId::imgConfig, "ImageConfig", N_("Image configuration")},
    {SectionId::userInfo, "UserInfo", N_("User information")},
    {SectionId::relatedFile, "RelatedFile", N_("Related file")},
    {SectionId::dateTime, "DateTime", N_("Date and time")},
    {SectionId::captureCond, "CaptureConditions", N_("Picture taking conditions")},
    {SectionId::gpsTags, "GPS", N_("GPS information")},
    {SectionId::iopTags, "Interoperability", N_("Interoperability information")},
    {SectionId::mpfTags, "MPF", N_("CIPA Multi-Picture Format")},
    {SectionId::makerTags, "Makernote", N_("Vendor specific information")},
    {SectionId::dngTags, "DngTags", N_("Adobe DNG tags")},
    {SectionId::panaRaw, "PanasonicRaw", N_("Panasonic RAW tags")},
    {SectionId::tiffEp, "TIFF/EP", N_("TIFF/EP tags")},
    {SectionId::tiffPm6, "TIFF&PM6", N_("TIFF PageMaker 6.0 tags")},
    {SectionId::adobeOpi, "AdobeOPI", N_("Adobe OPI tags")},
    {SectionId::lastSectionId, "(LastSection)", N_("Last section")},
};

}  // namespace Exiv2

namespace Exiv2::Internal {
bool TagVocabulary::operator==(const std::string& key) const {
  return key.rfind(voc_) != std::string::npos;
}

// Unknown Tag
static constexpr TagInfo unknownTag{
    0xffff,
    "Unknown tag",
    N_("Unknown tag"),
    N_("Unknown tag"),
    IfdId::ifdIdNotSet,
    SectionId::sectionIdNotSet,
    asciiString,
    -1,
    printValue,
};

}  // namespace Exiv2::Internal

namespace Exiv2 {
bool GroupInfo::operator==(IfdId ifdId) const {
  return ifdId_ == ifdId;
}

bool GroupInfo::operator==(const GroupName& groupName) const {
  return groupName == groupName_;
}

const char* ExifTags::sectionName(const ExifKey& key) {
  const TagInfo* ti = tagInfo(key.tag(), key.ifdId());
  if (!ti)
    return sectionInfo[static_cast<int>(unknownTag.sectionId_)].name_;
  return sectionInfo[static_cast<int>(ti->sectionId_)].name_;
}

/// \todo not used internally. At least we should test it
uint16_t ExifTags::defaultCount(const ExifKey& key) {
  const TagInfo* ti = tagInfo(key.tag(), key.ifdId());
  if (!ti)
    return unknownTag.count_;
  return ti->count_;
}

const char* ExifTags::ifdName(const std::string& groupName) {
  IfdId ifdId = Internal::groupId(groupName);
  return Internal::ifdName(ifdId);
}

bool ExifTags::isMakerGroup(const std::string& groupName) {
  IfdId ifdId = Internal::groupId(groupName);
  return Internal::isMakerIfd(ifdId);
}

bool ExifTags::isExifGroup(const std::string& groupName) {
  IfdId ifdId = Internal::groupId(groupName);
  return Internal::isExifIfd(ifdId);
}

const GroupInfo* ExifTags::groupList() {
  return Internal::groupList();
}

const TagInfo* ExifTags::tagList(const std::string& groupName) {
  return Internal::tagList(groupName);
}

void ExifTags::taglist(std::ostream& os) {
  const TagInfo* ifd = ifdTagList();
  const TagInfo* exif = exifTagList();
  const TagInfo* gps = gpsTagList();
  const TagInfo* iop = iopTagList();
  const TagInfo* mpf = mpfTagList();

  for (int i = 0; ifd[i].tag_ != 0xffff; ++i) {
    os << ifd[i] << "\n";
  }
  for (int i = 0; exif[i].tag_ != 0xffff; ++i) {
    os << exif[i] << "\n";
  }
  for (int i = 0; iop[i].tag_ != 0xffff; ++i) {
    os << iop[i] << "\n";
  }
  for (int i = 0; gps[i].tag_ != 0xffff; ++i) {
    os << gps[i] << "\n";
  }
  for (int i = 0; mpf[i].tag_ != 0xffff; ++i) {
    os << mpf[i] << "\n";
  }
}  // ExifTags::taglist

void ExifTags::taglist(std::ostream& os, const std::string& groupName) {
  IfdId ifdId = Internal::groupId(groupName);
  Internal::taglist(os, ifdId);
}

//! %Internal Pimpl structure with private members and data of class ExifKey.
struct ExifKey::Impl {
  //! @name Manipulators
  //@{
  /*!
    @brief Set the key corresponding to the \em tag, \em ifdId and \em tagInfo.
           The key is of the form '<b>Exif</b>.groupName.tagName'.
   */
  void makeKey(uint16_t tag, IfdId ifdId, const TagInfo* tagInfo);
  /*!
    @brief Parse and convert the key string into tag and IFD Id.
           Updates data members if the string can be decomposed,
           or throws \em Error .

    @throw Error if the key cannot be decomposed.
   */
  void decomposeKey(const std::string& key);
  //@}

  //! @name Accessors
  //@{
  //! Return the name of the tag
  [[nodiscard]] std::string tagName() const;
  //@}

  // DATA
  static constexpr auto familyName_ = "Exif";  //!< "Exif"

  const TagInfo* tagInfo_{};         //!< Tag info
  uint16_t tag_{0};                  //!< Tag value
  IfdId ifdId_{IfdId::ifdIdNotSet};  //!< The IFD associated with this tag
  int idx_{0};                       //!< Unique id of the Exif key in the image
  std::string groupName_;            //!< The group name
  std::string key_;                  //!< %Key
};

std::string ExifKey::Impl::tagName() const {
  if (tagInfo_ && tagInfo_->tag_ != 0xffff) {
    return tagInfo_->name_;
  }
  std::ostringstream os;
  os << "0x" << std::setw(4) << std::setfill('0') << std::right << std::hex << tag_;
  return os.str();
}

void ExifKey::Impl::decomposeKey(const std::string& key) {
  // Get the family name, IFD name and tag name parts of the key
  std::string::size_type pos1 = key.find('.');
  if (pos1 == std::string::npos)
    throw Error(ErrorCode::kerInvalidKey, key);
  std::string familyName = key.substr(0, pos1);
  if (familyName != familyName_)
    throw Error(ErrorCode::kerInvalidKey, key);
  std::string::size_type pos0 = pos1 + 1;
  pos1 = key.find('.', pos0);
  if (pos1 == std::string::npos)
    throw Error(ErrorCode::kerInvalidKey, key);
  std::string groupName = key.substr(pos0, pos1 - pos0);
  if (groupName.empty())
    throw Error(ErrorCode::kerInvalidKey, key);
  std::string tn = key.substr(pos1 + 1);
  if (tn.empty())
    throw Error(ErrorCode::kerInvalidKey, key);

  // Find IfdId
  IfdId ifdId = groupId(groupName);
  if (ifdId == IfdId::ifdIdNotSet)
    throw Error(ErrorCode::kerInvalidKey, key);
  if (!Internal::isExifIfd(ifdId) && !Internal::isMakerIfd(ifdId)) {
    throw Error(ErrorCode::kerInvalidKey, key);
  }
  // Convert tag
  uint16_t tag = tagNumber(tn, ifdId);
  // Get tag info
  tagInfo_ = tagInfo(tag, ifdId);
  if (!tagInfo_)
    throw Error(ErrorCode::kerInvalidKey, key);

  tag_ = tag;
  ifdId_ = ifdId;
  groupName_ = groupName;
  // tagName() translates hex tag name (0xabcd) to a real tag name if there is one
  key_ = familyName + "." + groupName + "." + tagName();
}

void ExifKey::Impl::makeKey(uint16_t tag, IfdId ifdId, const TagInfo* tagInfo) {
  tagInfo_ = tagInfo;
  tag_ = tag;
  ifdId_ = ifdId;
  key_ = std::string(familyName_) + "." + groupName_ + "." + tagName();
}

ExifKey::ExifKey(uint16_t tag, const std::string& groupName) : p_(std::make_unique<Impl>()) {
  IfdId ifdId = groupId(groupName);
  // Todo: Test if this condition can be removed
  if (!Internal::isExifIfd(ifdId) && !Internal::isMakerIfd(ifdId)) {
    throw Error(ErrorCode::kerInvalidIfdId, ifdId);
  }
  const TagInfo* ti = tagInfo(tag, ifdId);
  if (!ti) {
    throw Error(ErrorCode::kerInvalidIfdId, ifdId);
  }
  p_->groupName_ = groupName;
  p_->makeKey(tag, ifdId, ti);
}

ExifKey::ExifKey(const TagInfo& ti) : p_(std::make_unique<Impl>()) {
  auto ifdId = ti.ifdId_;
  if (!Internal::isExifIfd(ifdId) && !Internal::isMakerIfd(ifdId)) {
    throw Error(ErrorCode::kerInvalidIfdId, ifdId);
  }
  p_->groupName_ = Exiv2::groupName(ifdId);
  p_->makeKey(ti.tag_, ifdId, &ti);
}

ExifKey::ExifKey(const std::string& key) : p_(std::make_unique<Impl>()) {
  p_->decomposeKey(key);
}

ExifKey::ExifKey(const ExifKey& rhs) : Key(rhs), p_(std::make_unique<Impl>(*rhs.p_)) {
}

ExifKey::~ExifKey() = default;

ExifKey& ExifKey::operator=(const ExifKey& rhs) {
  if (this == &rhs)
    return *this;
  Key::operator=(rhs);
  *p_ = *rhs.p_;
  return *this;
}

void ExifKey::setIdx(int idx) const {
  p_->idx_ = idx;
}

std::string ExifKey::key() const {
  return p_->key_;
}

const char* ExifKey::familyName() const {
  return Exiv2::ExifKey::Impl::familyName_;
}

std::string ExifKey::groupName() const {
  return p_->groupName_;
}

std::string ExifKey::tagName() const {
  return p_->tagName();
}

std::string ExifKey::tagLabel() const {
  if (!p_->tagInfo_ || p_->tagInfo_->tag_ == 0xffff)
    return "";
  return _(p_->tagInfo_->title_);
}

std::string ExifKey::tagDesc() const {
  if (!p_->tagInfo_ || p_->tagInfo_->tag_ == 0xffff)
    return {};
  return _(p_->tagInfo_->desc_);
}

TypeId ExifKey::defaultTypeId() const {
  if (!p_->tagInfo_)
    return unknownTag.typeId_;
  return p_->tagInfo_->typeId_;
}

uint16_t ExifKey::tag() const {
  return p_->tag_;
}

ExifKey::UniquePtr ExifKey::clone() const {
  return UniquePtr(clone_());
}

ExifKey* ExifKey::clone_() const {
  return new ExifKey(*this);
}

IfdId ExifKey::ifdId() const {
  return p_->ifdId_;
}

int ExifKey::idx() const {
  return p_->idx_;
}

// *************************************************************************
// free functions

std::ostream& operator<<(std::ostream& os, const TagInfo& ti) {
  std::ios::fmtflags f(os.flags());
  ExifKey exifKey(ti);
  os << exifKey.tagName() << "," << std::dec << exifKey.tag() << ","
     << "0x" << std::setw(4) << std::setfill('0') << std::right << std::hex << exifKey.tag() << ","
     << exifKey.groupName() << "," << exifKey.key() << "," << TypeInfo::typeName(exifKey.defaultTypeId()) << ",";
  // CSV encoded I am \"dead\" beat" => "I am ""dead"" beat"
  char Q = '"';
  os << Q;
  for (char c : exifKey.tagDesc()) {
    if (c == Q)
      os << Q;
    os << c;
  }
  os << Q;
  os.flags(f);
  return os;
}

}  // namespace Exiv2
