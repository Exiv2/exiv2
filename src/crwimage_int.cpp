// SPDX-License-Identifier: GPL-2.0-or-later

#include "crwimage_int.hpp"
#include "canonmn_int.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "i18n.h"  // NLS support.

#include <algorithm>
#include <ctime>
#include <iostream>

// *****************************************************************************
// local declarations
namespace {
//! Helper class to map Exif orientation values to CRW rotation degrees
class RotationMap {
 public:
  //! Get the orientation number for a degree value
  static uint16_t orientation(int32_t degrees);
  //! Get the degree value for an orientation number
  static int32_t degrees(uint16_t orientation);

 private:
  //! Helper structure for the mapping list
  struct OmList {
    uint16_t orientation;
    int32_t degrees;
  };
  // DATA
  static const OmList omList_[];
};  // class RotationMap
}  // namespace

// *****************************************************************************
// local definitions
namespace {
constexpr RotationMap::OmList RotationMap::omList_[] = {
    {1, 0}, {3, 180}, {3, -180}, {6, 90}, {6, -270}, {8, 270}, {8, -90},
};

uint16_t RotationMap::orientation(int32_t degrees) {
  uint16_t o = 1;
  for (auto&& [orient, deg] : omList_) {
    if (deg == degrees) {
      o = orient;
      break;
    }
  }
  return o;
}

int32_t RotationMap::degrees(uint16_t orientation) {
  int32_t d = 0;
  for (auto&& [orient, deg] : omList_) {
    if (orient == orientation) {
      d = deg;
      break;
    }
  }
  return d;
}
}  // namespace

namespace Exiv2::Internal {
/*
  Mapping table used to decode and encode CIFF tags to/from Exif tags.  Only
  a subset of the Exif tags can be mapped to known tags found in CRW files
  and not all CIFF tags in the CRW files have a corresponding Exif tag. Tags
  which are not mapped in the table below are ignored.

  When decoding, each CIFF tag/directory pair in the CRW image is looked up
  in the table and if it has an entry, the corresponding decode function is
  called (CrwMap::decode). This function may or may not make use of the
  other parameters in the structure (such as the Exif tag and Ifd id).

  Encoding is done in a loop over the mapping table (CrwMap::encode). For
  each entry, the encode function is called, which looks up the (Exif)
  metadata to encode in the image. This function may or may not make use of
  the other parameters in the mapping structure.
*/
const CrwMapping CrwMap::crwMapping_[] = {
    //         CrwTag  CrwDir  Size ExifTag IfdId    decodeFct     encodeFct
    //         ------  ------  ---- ------- -----    ---------     ---------
    {0x0805, 0x300a, 0, 0, IfdId::canonId, decode0x0805, encode0x0805},
    {0x080a, 0x2807, 0, 0, IfdId::canonId, decode0x080a, encode0x080a},
    {0x080b, 0x3004, 0, 0x0007, IfdId::canonId, decodeBasic, encodeBasic},
    {0x0810, 0x2807, 0, 0x0009, IfdId::canonId, decodeBasic, encodeBasic},
    {0x0815, 0x2804, 0, 0x0006, IfdId::canonId, decodeBasic, encodeBasic},
    {0x1029, 0x300b, 0, 0x0002, IfdId::canonId, decodeBasic, encodeBasic},
    {0x102a, 0x300b, 0, 0x0004, IfdId::canonId, decodeArray, encodeArray},
    {0x102d, 0x300b, 0, 0x0001, IfdId::canonId, decodeArray, encodeArray},
    {0x1033, 0x300b, 0, 0x000f, IfdId::canonId, decodeArray, encodeArray},
    {0x1038, 0x300b, 0, 0x0012, IfdId::canonId, decodeArray, encodeArray},
    {0x10a9, 0x300b, 0, 0x00a9, IfdId::canonId, decodeBasic, encodeBasic},
    // Mapped to Exif.Photo.ColorSpace instead (see below)
    // {0x10b4, 0x300b,   0, 0x00b4, IfdId::canonId, decodeBasic,  encodeBasic},
    {0x10b4, 0x300b, 0, 0xa001, IfdId::exifId, decodeBasic, encodeBasic},
    {0x10b5, 0x300b, 0, 0x00b5, IfdId::canonId, decodeBasic, encodeBasic},
    {0x10c0, 0x300b, 0, 0x00c0, IfdId::canonId, decodeBasic, encodeBasic},
    {0x10c1, 0x300b, 0, 0x00c1, IfdId::canonId, decodeBasic, encodeBasic},
    {0x1807, 0x3002, 0, 0x9206, IfdId::exifId, decodeBasic, encodeBasic},
    {0x180b, 0x3004, 0, 0x000c, IfdId::canonId, decodeBasic, encodeBasic},
    {0x180e, 0x300a, 0, 0x9003, IfdId::exifId, decode0x180e, encode0x180e},
    {0x1810, 0x300a, 0, 0xa002, IfdId::exifId, decode0x1810, encode0x1810},
    {0x1817, 0x300a, 4, 0x0008, IfdId::canonId, decodeBasic, encodeBasic},
    // {0x1818, 0x3002,   0, 0x9204, IfdId::exifId, decodeBasic,  encodeBasic},
    {0x183b, 0x300b, 0, 0x0015, IfdId::canonId, decodeBasic, encodeBasic},
    {0x2008, 0x0000, 0, 0, IfdId::ifd1Id, decode0x2008, encode0x2008},
};  // CrwMap::crwMapping_[]

/*
  CIFF directory hierarchy

            root
             |
            300a
             |
   +----+----+----+----+
   |    |    |    |    |
  2804 2807 3002 3003 300b
        |
       3004

  The array is arranged bottom-up so that starting with a directory at the
  bottom, the (unique) path to root can be determined in a single loop.
*/
const CrwSubDir CrwMap::crwSubDir_[] = {
    // dir,   parent
    {0x3004, 0x2807}, {0x300b, 0x300a}, {0x3003, 0x300a}, {0x3002, 0x300a},
    {0x2807, 0x300a}, {0x2804, 0x300a}, {0x300a, 0x0000}, {0x0000, 0xffff},
};

CiffComponent::CiffComponent(uint16_t tag, uint16_t dir) : dir_(dir), tag_(tag) {
}

CiffDirectory::~CiffDirectory() {
  for (auto&& component : components_) {
    delete component;
  }
}

void CiffComponent::add(UniquePtr component) {
  doAdd(std::move(component));
}

void CiffEntry::doAdd(UniquePtr /*component*/) {
  throw Error(ErrorCode::kerFunctionNotSupported, "CiffEntry::add");
}  // CiffEntry::doAdd

void CiffDirectory::doAdd(UniquePtr component) {
  components_.push_back(component.release());
}  // CiffDirectory::doAdd

void CiffHeader::read(const byte* pData, size_t size) {
  if (size < 14)
    throw Error(ErrorCode::kerNotACrwImage);

  if (pData[0] == 'I' && pData[0] == pData[1]) {
    byteOrder_ = littleEndian;
  } else if (pData[0] == 'M' && pData[0] == pData[1]) {
    byteOrder_ = bigEndian;
  } else {
    throw Error(ErrorCode::kerNotACrwImage);
  }
  offset_ = getULong(pData + 2, byteOrder_);
  if (offset_ < 14 || offset_ > size)
    throw Error(ErrorCode::kerNotACrwImage);
  if (std::memcmp(pData + 6, signature(), 8) != 0) {
    throw Error(ErrorCode::kerNotACrwImage);
  }

  pPadding_.clear();
  pPadding_.resize(offset_ - 14);
  padded_ = offset_ - 14;
  std::copy_n(pData + 14, padded_, pPadding_.begin());

  pRootDir_ = std::make_unique<CiffDirectory>();
  pRootDir_->readDirectory(pData + offset_, size - offset_, byteOrder_);
}  // CiffHeader::read

void CiffComponent::read(const byte* pData, size_t size, uint32_t start, ByteOrder byteOrder) {
  doRead(pData, size, start, byteOrder);
}

void CiffComponent::doRead(const byte* pData, size_t size, uint32_t start, ByteOrder byteOrder) {
  // We're going read 10 bytes. Make sure they won't be out-of-bounds.
  enforce(size >= 10 && start <= size - 10, ErrorCode::kerNotACrwImage);
  tag_ = getUShort(pData + start, byteOrder);

  DataLocId dl = dataLocation();

  if (dl == DataLocId::valueData) {
    size_ = getULong(pData + start + 2, byteOrder);
    offset_ = getULong(pData + start + 6, byteOrder);

    // Make sure that the sub-region does not overlap with the 10 bytes
    // that we just read. (Otherwise a malicious file could cause an
    // infinite recursion.) There are two cases two consider because
    // the sub-region is allowed to be either before or after the 10
    // bytes in memory.
    if (offset_ < start) {
      // Sub-region is before in memory.
      enforce(size_ <= start - offset_, ErrorCode::kerOffsetOutOfRange);
    } else {
      // Sub-region is after in memory.
      enforce(offset_ >= start + 10, ErrorCode::kerOffsetOutOfRange);
      enforce(offset_ <= size, ErrorCode::kerOffsetOutOfRange);
      enforce(size_ <= size - offset_, ErrorCode::kerOffsetOutOfRange);
    }
  }
  if (dl == DataLocId::directoryData) {
    size_ = 8;
    offset_ = start + 2;
  }
  pData_ = pData + offset_;
#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "  Entry for tag 0x" << std::hex << tagId() << " (0x" << tag() << "), " << std::dec << size_
            << " Bytes, Offset is " << offset_ << "\n";
#endif

}  // CiffComponent::doRead

void CiffDirectory::doRead(const byte* pData, size_t size, uint32_t start, ByteOrder byteOrder) {
  CiffComponent::doRead(pData, size, start, byteOrder);
#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Reading directory 0x" << std::hex << tag() << "\n";
#endif
  if (this->offset() + this->size() > size)
    throw Error(ErrorCode::kerOffsetOutOfRange);

  readDirectory(pData + offset(), this->size(), byteOrder);
#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "<---- 0x" << std::hex << tag() << "\n";
#endif
}  // CiffDirectory::doRead

void CiffDirectory::readDirectory(const byte* pData, size_t size, ByteOrder byteOrder) {
  if (size < 4)
    throw Error(ErrorCode::kerCorruptedMetadata);
  uint32_t o = getULong(pData + size - 4, byteOrder);
  if (o > size - 2)
    throw Error(ErrorCode::kerCorruptedMetadata);
  uint16_t count = getUShort(pData + o, byteOrder);
#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Directory at offset " << std::dec << o << ", " << count << " entries \n";
#endif
  o += 2;
  if (count * 10u > size - o)
    throw Error(ErrorCode::kerCorruptedMetadata);

  for (uint16_t i = 0; i < count; ++i) {
    uint16_t tag = getUShort(pData + o, byteOrder);
    auto m = [this, tag]() -> std::unique_ptr<CiffComponent> {
      if (this->typeId(tag) == TypeId::directory)
        return std::make_unique<CiffDirectory>();
      return std::make_unique<CiffEntry>();
    }();
    m->setDir(this->tag());
    m->read(pData, size, o, byteOrder);
    add(std::move(m));
    o += 10;
  }
}  // CiffDirectory::readDirectory

void CiffHeader::decode(Image& image) const {
  // Nothing to decode from the header itself, just add correct byte order
  if (pRootDir_)
    pRootDir_->decode(image, byteOrder_);
}

void CiffComponent::decode(Image& image, ByteOrder byteOrder) const {
  doDecode(image, byteOrder);
}

void CiffEntry::doDecode(Image& image, ByteOrder byteOrder) const {
  CrwMap::decode(*this, image, byteOrder);
}  // CiffEntry::doDecode

void CiffDirectory::doDecode(Image& image, ByteOrder byteOrder) const {
  for (auto&& component : components_) {
    component->decode(image, byteOrder);
  }
}  // CiffDirectory::doDecode

void CiffHeader::write(Blob& blob) const {
  if (byteOrder_ == littleEndian) {
    blob.push_back('I');
    blob.push_back('I');
  } else {
    blob.push_back('M');
    blob.push_back('M');
  }
  uint32_t o = 2;
  byte buf[4];
  ul2Data(buf, offset_, byteOrder_);
  append(blob, buf, 4);
  o += 4;
  append(blob, reinterpret_cast<const byte*>(signature_), 8);
  o += 8;
  // Pad as needed
  if (!pPadding_.empty()) {
    append(blob, pPadding_.data(), padded_);
  } else {
    for (uint32_t i = o; i < offset_; ++i) {
      blob.push_back(0);
      ++o;
    }
  }
  if (pRootDir_) {
    pRootDir_->write(blob, byteOrder_, offset_);
  }
}

size_t CiffComponent::write(Blob& blob, ByteOrder byteOrder, size_t offset) {
  return doWrite(blob, byteOrder, offset);
}

size_t CiffEntry::doWrite(Blob& blob, ByteOrder /*byteOrder*/, size_t offset) {
  return writeValueData(blob, offset);
}

size_t CiffComponent::writeValueData(Blob& blob, size_t offset) {
  if (dataLocation() == DataLocId::valueData) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cout << "  Data for tag 0x" << std::hex << tagId() << ", " << std::dec << size_ << " Bytes\n";
#endif
    offset_ = offset;
    append(blob, pData_, size_);
    offset += size_;
    // Pad the value to an even number of bytes
    if (size_ % 2 == 1) {
      blob.push_back(0);
      ++offset;
    }
  }
  return offset;
}

size_t CiffDirectory::doWrite(Blob& blob, ByteOrder byteOrder, size_t offset) {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Writing directory 0x" << std::hex << tag() << "---->\n";
#endif
  // Ciff offsets are relative to the start of the directory
  size_t dirOffset = 0;

  // Value data
  for (auto&& component : components_) {
    dirOffset = component->write(blob, byteOrder, dirOffset);
  }
  const auto dirStart = static_cast<uint32_t>(dirOffset);

  // Number of directory entries
  byte buf[4];
  us2Data(buf, static_cast<uint16_t>(components_.size()), byteOrder);
  append(blob, buf, 2);
  dirOffset += 2;

  // Directory entries
  for (auto&& component : components_) {
    component->writeDirEntry(blob, byteOrder);
    dirOffset += 10;
  }

  // Offset of directory
  ul2Data(buf, dirStart, byteOrder);
  append(blob, buf, 4);
  dirOffset += 4;

  // Update directory entry
  setOffset(offset);
  setSize(dirOffset);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Directory is at offset " << std::dec << dirStart << ", " << components_.size() << " entries\n"
            << "<---- 0x" << std::hex << tag() << "\n";
#endif
  return offset + dirOffset;
}  // CiffDirectory::doWrite

void CiffComponent::writeDirEntry(Blob& blob, ByteOrder byteOrder) const {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "  Directory entry for tag 0x" << std::hex << tagId() << " (0x" << tag() << "), " << std::dec << size_
            << " Bytes, Offset is " << offset_ << "\n";
#endif
  byte buf[4];

  DataLocId dl = dataLocation();

  if (dl == DataLocId::valueData) {
    us2Data(buf, tag_, byteOrder);
    append(blob, buf, 2);

    ul2Data(buf, static_cast<uint32_t>(size_), byteOrder);
    append(blob, buf, 4);

    ul2Data(buf, static_cast<uint32_t>(offset_), byteOrder);
    append(blob, buf, 4);
  }

  if (dl == DataLocId::directoryData) {
    // Only 8 bytes fit in the directory entry

    us2Data(buf, tag_, byteOrder);
    append(blob, buf, 2);
    // Copy value instead of size and offset
    append(blob, pData_, size_);
    // Pad with 0s
    for (size_t i = size_; i < 8; ++i) {
      blob.push_back(0);
    }
  }
}  // CiffComponent::writeDirEntry

void CiffComponent::print(std::ostream& os, ByteOrder byteOrder, const std::string& prefix) const {
  doPrint(os, byteOrder, prefix);
}

void CiffComponent::doPrint(std::ostream& os, ByteOrder byteOrder, const std::string& prefix) const {
  os << prefix << _("tag") << " = 0x" << std::setw(4) << std::setfill('0') << std::hex << std::right << tagId() << ", "
     << _("dir") << " = 0x" << std::setw(4) << std::setfill('0') << std::hex << std::right << dir() << ", " << _("type")
     << " = " << TypeInfo::typeName(typeId()) << ", " << _("size") << " = " << std::dec << size_ << ", " << _("offset")
     << " = " << offset_ << "\n";

  if (typeId() != directory) {
    auto value = Value::create(typeId());
    value->read(pData_, size_, byteOrder);
    if (value->size() < 100) {
      os << prefix << *value << "\n";
    }
  }
}  // CiffComponent::doPrint

void CiffDirectory::doPrint(std::ostream& os, ByteOrder byteOrder, const std::string& prefix) const {
  CiffComponent::doPrint(os, byteOrder, prefix);
  for (auto&& component : components_) {
    component->print(os, byteOrder, prefix + "   ");
  }
}  // CiffDirectory::doPrint

void CiffComponent::setValue(DataBuf&& buf) {
  storage_ = std::move(buf);
  pData_ = storage_.c_data();
  size_ = storage_.size();
  if (size_ > 8 && dataLocation() == DataLocId::directoryData) {
    tag_ &= 0x3fff;
  }
}

TypeId CiffComponent::typeId(uint16_t tag) {
  switch (tag & 0x3800) {
    case 0x0000:
      return unsignedByte;
    case 0x0800:
      return asciiString;
    case 0x1000:
      return unsignedShort;
    case 0x1800:
      return unsignedLong;
    case 0x2000:
      return undefined;
    case 0x2800:
    case 0x3000:
      return directory;
  }
  return invalidTypeId;
}  // CiffComponent::typeId

DataLocId CiffComponent::dataLocation(uint16_t tag) {
  switch (tag & 0xc000) {
    case 0x0000:
      return DataLocId::valueData;
    case 0x4000:
      return DataLocId::directoryData;
    default:
      throw Error(ErrorCode::kerCorruptedMetadata);
  }
}  // CiffComponent::dataLocation

/*!
  @brief Finds \em crwTagId in directory \em crwDir, returning a pointer to
         the component or 0 if not found.

 */
CiffComponent* CiffHeader::findComponent(uint16_t crwTagId, uint16_t crwDir) const {
  if (!pRootDir_)
    return nullptr;
  return pRootDir_->findComponent(crwTagId, crwDir);
}  // CiffHeader::findComponent

CiffComponent* CiffComponent::findComponent(uint16_t crwTagId, uint16_t crwDir) const {
  return doFindComponent(crwTagId, crwDir);
}  // CiffComponent::findComponent

CiffComponent* CiffComponent::doFindComponent(uint16_t crwTagId, uint16_t crwDir) const {
  if (tagId() == crwTagId && dir() == crwDir) {
    return const_cast<CiffComponent*>(this);
  }
  return nullptr;
}  // CiffComponent::doFindComponent

CiffComponent* CiffDirectory::doFindComponent(uint16_t crwTagId, uint16_t crwDir) const {
  for (auto&& component : components_) {
    if (auto cc = component->findComponent(crwTagId, crwDir))
      return cc;
  }
  return nullptr;
}  // CiffDirectory::doFindComponent

void CiffHeader::add(uint16_t crwTagId, uint16_t crwDir, DataBuf&& buf) {
  CrwDirs crwDirs;
  CrwMap::loadStack(crwDirs, crwDir);
  crwDirs.pop();
  if (!pRootDir_) {
    pRootDir_ = std::make_unique<CiffDirectory>();
  }
  if (auto child = pRootDir_->add(crwDirs, crwTagId)) {
    child->setValue(std::move(buf));
  }
}  // CiffHeader::add

CiffComponent* CiffComponent::add(CrwDirs& crwDirs, uint16_t crwTagId) {
  return doAdd(crwDirs, crwTagId);
}  // CiffComponent::add

CiffComponent* CiffComponent::doAdd(CrwDirs& /*crwDirs*/, uint16_t /*crwTagId*/) {
  return nullptr;
}  // CiffComponent::doAdd

CiffComponent* CiffDirectory::doAdd(CrwDirs& crwDirs, uint16_t crwTagId) {
  /*
    add()
      if stack not empty
        pop from stack
        find dir among components
        if not found, create it
        add()
      else
        find tag among components
        if not found, create it
        set value
  */
  if (!crwDirs.empty()) {
    auto dir = crwDirs.top();
    crwDirs.pop();
    // Find the directory
    for (const auto& c : components_)
      if (c->tag() == dir.dir) {
        cc_ = c;
        break;
      }
    if (!cc_) {
      // Directory doesn't exist yet, add it
      m_ = std::make_unique<CiffDirectory>(dir.dir, dir.parent);
      cc_ = m_.get();
      add(std::move(m_));
    }
    // Recursive call to next lower level directory
    cc_ = cc_->add(crwDirs, crwTagId);
  } else {
    // Find the tag
    for (const auto& c : components_)
      if (c->tagId() == crwTagId) {
        cc_ = c;
        break;
      }
    if (!cc_) {
      // Tag doesn't exist yet, add it
      m_ = std::make_unique<CiffEntry>(crwTagId, tag());
      cc_ = m_.get();
      add(std::move(m_));
    }
  }
  return cc_;
}  // CiffDirectory::doAdd

void CiffHeader::remove(uint16_t crwTagId, uint16_t crwDir) const {
  if (pRootDir_) {
    CrwDirs crwDirs;
    CrwMap::loadStack(crwDirs, crwDir);
    crwDirs.pop();
    pRootDir_->remove(crwDirs, crwTagId);
  }
}  // CiffHeader::remove

void CiffComponent::remove(CrwDirs& crwDirs, uint16_t crwTagId) {
  doRemove(crwDirs, crwTagId);
}  // CiffComponent::remove

void CiffComponent::doRemove(CrwDirs& /*crwDirs*/, uint16_t /*crwTagId*/) {
  // do nothing
}  // CiffComponent::doRemove

void CiffDirectory::doRemove(CrwDirs& crwDirs, uint16_t crwTagId) {
  if (!crwDirs.empty()) {
    auto dir = crwDirs.top();
    crwDirs.pop();
    // Find the directory
    auto it = std::find_if(components_.begin(), components_.end(), [=](const auto& c) { return c->tag() == dir.dir; });
    if (it != components_.end()) {
      // Recursive call to next lower level directory
      (*it)->remove(crwDirs, crwTagId);
      if ((*it)->empty())
        components_.erase(it);
    }
  } else {
    // Find the tag
    auto it = std::find_if(components_.begin(), components_.end(), [=](const auto& c) { return c->tag() == crwTagId; });
    if (it != components_.end()) {
      delete *it;
      components_.erase(it);
    }
  }
}  // CiffDirectory::doRemove

bool CiffComponent::empty() const {
  return doEmpty();
}

bool CiffComponent::doEmpty() const {
  return size_ == 0;
}

bool CiffDirectory::doEmpty() const {
  return components_.empty();
}

void CrwMap::decode(const CiffComponent& ciffComponent, Image& image, ByteOrder byteOrder) {
  const CrwMapping* cmi = crwMapping(ciffComponent.dir(), ciffComponent.tagId());
  if (cmi && cmi->toExif_) {
    cmi->toExif_(ciffComponent, cmi, image, byteOrder);
  }
}  // CrwMap::decode

const CrwMapping* CrwMap::crwMapping(uint16_t crwDir, uint16_t crwTagId) {
  for (auto&& crw : crwMapping_) {
    if (crw.crwDir_ == crwDir && crw.crwTagId_ == crwTagId) {
      return &crw;
    }
  }
  return nullptr;
}  // CrwMap::crwMapping

void CrwMap::decode0x0805(const CiffComponent& ciffComponent, const CrwMapping* /*pCrwMapping*/, Image& image,
                          ByteOrder /*byteOrder*/) {
  std::string s(reinterpret_cast<const char*>(ciffComponent.pData()));
  image.setComment(s);
}  // CrwMap::decode0x0805

void CrwMap::decode0x080a(const CiffComponent& ciffComponent, const CrwMapping* /*pCrwMapping*/, Image& image,
                          ByteOrder byteOrder) {
  if (ciffComponent.typeId() != asciiString)
    return;

  // Make
  ExifKey key1("Exif.Image.Make");
  auto value1 = Value::create(ciffComponent.typeId());
  uint32_t i = 0;
  while (i < ciffComponent.size() && ciffComponent.pData()[i++] != '\0') {
    // empty
  }
  value1->read(ciffComponent.pData(), i, byteOrder);
  image.exifData().add(key1, value1.get());

  // Model
  ExifKey key2("Exif.Image.Model");
  auto value2 = Value::create(ciffComponent.typeId());
  uint32_t j = i;
  while (i < ciffComponent.size() && ciffComponent.pData()[i++] != '\0') {
    // empty
  }
  value2->read(ciffComponent.pData() + j, i - j, byteOrder);
  image.exifData().add(key2, value2.get());
}  // CrwMap::decode0x080a

void CrwMap::decodeArray(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                         ByteOrder byteOrder) {
  if (ciffComponent.typeId() != unsignedShort) {
    decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
    return;
  }

  int64_t aperture = 0;
  int64_t shutterSpeed = 0;

  auto ifdId = [pCrwMapping] {
    if (pCrwMapping->tag_ == 0x0001)
      return IfdId::canonCsId;
    if (pCrwMapping->tag_ == 0x0004)
      return IfdId::canonSiId;
    if (pCrwMapping->tag_ == 0x000f)
      return IfdId::canonCfId;
    if (pCrwMapping->tag_ == 0x0012)
      return IfdId::canonPiId;
    return IfdId::ifdIdNotSet;
  }();

  std::string groupName(Internal::groupName(ifdId));
  const size_t component_size = ciffComponent.size();
  enforce(component_size % 2 == 0, ErrorCode::kerCorruptedMetadata);
  enforce(component_size / 2 <= static_cast<size_t>(std::numeric_limits<uint16_t>::max()),
          ErrorCode::kerCorruptedMetadata);
  const auto num_components = static_cast<uint16_t>(component_size / 2);
  uint16_t c = 1;
  while (c < num_components) {
    uint16_t n = 1;
    ExifKey key(c, groupName);
    UShortValue value;
    if (ifdId == IfdId::canonCsId && c == 23 && component_size >= 52)
      n = 3;
    value.read(ciffComponent.pData() + (c * 2), n * 2, byteOrder);
    image.exifData().add(key, &value);
    if (ifdId == IfdId::canonSiId && c == 21)
      aperture = value.toInt64();
    if (ifdId == IfdId::canonSiId && c == 22)
      shutterSpeed = value.toInt64();
    c += n;
  }

  if (ifdId == IfdId::canonSiId) {
    // Exif.Photo.FNumber
    float f = fnumber(canonEv(aperture));
    auto [r, s] = floatToRationalCast(f);
    auto ur = URational(r, s);
    URationalValue fn;
    fn.value_.push_back(ur);
    image.exifData().add(ExifKey("Exif.Photo.FNumber"), &fn);

    // Exif.Photo.ExposureTime
    ur = exposureTime(canonEv(shutterSpeed));
    URationalValue et;
    et.value_.push_back(ur);
    image.exifData().add(ExifKey("Exif.Photo.ExposureTime"), &et);
  }
}  // CrwMap::decodeArray

void CrwMap::decode0x180e(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                          ByteOrder byteOrder) {
  if (ciffComponent.size() < 8 || ciffComponent.typeId() != unsignedLong) {
    decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
    return;
  }
  ULongValue v;
  v.read(ciffComponent.pData(), 8, byteOrder);
  time_t t = v.value_.at(0);
  tm r;
#ifdef _WIN32
  auto tm = localtime_s(&r, &t) ? nullptr : &r;
#else
  auto tm = localtime_r(&t, &r);
#endif
  if (tm) {
    const size_t m = 20;
    char s[m];
    std::strftime(s, m, "%Y:%m:%d %H:%M:%S", tm);

    ExifKey key(pCrwMapping->tag_, Internal::groupName(pCrwMapping->ifdId_));
    AsciiValue value;
    value.read(std::string(s));
    image.exifData().add(key, &value);
  }
}  // CrwMap::decode0x180e

void CrwMap::decode0x1810(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                          ByteOrder byteOrder) {
  if (ciffComponent.typeId() != unsignedLong || ciffComponent.size() < 28) {
    decodeBasic(ciffComponent, pCrwMapping, image, byteOrder);
    return;
  }

  ExifKey key1("Exif.Photo.PixelXDimension");
  ULongValue value1;
  value1.read(ciffComponent.pData(), 4, byteOrder);
  image.exifData().add(key1, &value1);

  ExifKey key2("Exif.Photo.PixelYDimension");
  ULongValue value2;
  value2.read(ciffComponent.pData() + 4, 4, byteOrder);
  image.exifData().add(key2, &value2);

  int32_t r = getLong(ciffComponent.pData() + 12, byteOrder);
  uint16_t o = RotationMap::orientation(r);
  image.exifData()["Exif.Image.Orientation"] = o;

}  // CrwMap::decode0x1810

void CrwMap::decode0x2008(const CiffComponent& ciffComponent, const CrwMapping* /*pCrwMapping*/, Image& image,
                          ByteOrder /*byteOrder*/) {
  ExifThumb exifThumb(image.exifData());
  exifThumb.setJpegThumbnail(ciffComponent.pData(), ciffComponent.size());
}  // CrwMap::decode0x2008

void CrwMap::decodeBasic(const CiffComponent& ciffComponent, const CrwMapping* pCrwMapping, Image& image,
                         ByteOrder byteOrder) {
  // create a key and value pair
  ExifKey key(pCrwMapping->tag_, Internal::groupName(pCrwMapping->ifdId_));
  std::unique_ptr<Value> value;
  if (ciffComponent.typeId() != directory) {
    value = Value::create(ciffComponent.typeId());
    size_t size = 0;
    if (pCrwMapping->size_ != 0) {
      size = pCrwMapping->size_;  // size in the mapping table overrides all
    } else if (ciffComponent.typeId() == asciiString) {
      // determine size from the data, by looking for the first 0
      uint32_t i = 0;
      while (i < ciffComponent.size() && ciffComponent.pData()[i++] != '\0') {
        // empty
      }
      size = i;
    } else {
      // by default, use the size from the directory entry
      size = ciffComponent.size();
    }
    value->read(ciffComponent.pData(), size, byteOrder);
  }
  // Add metadatum to exif data
  image.exifData().add(key, value.get());
}  // CrwMap::decodeBasic

void CrwMap::loadStack(CrwDirs& crwDirs, uint16_t crwDir) {
  for (auto&& crw : crwSubDir_) {
    auto&& [dir, parent] = crw;
    if (dir == crwDir) {
      crwDirs.push(crw);
      crwDir = parent;
    }
  }
}  // CrwMap::loadStack

void CrwMap::encode(CiffHeader* pHead, const Image& image) {
  for (auto&& crw : crwMapping_) {
    if (crw.fromExif_) {
      crw.fromExif_(image, &crw, pHead);
    }
  }
}  // CrwMap::encode

void CrwMap::encodeBasic(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead) {
  // Determine the source Exif metadatum
  ExifKey ek(pCrwMapping->tag_, Internal::groupName(pCrwMapping->ifdId_));
  auto ed = image.exifData().findKey(ek);

  // Set the new value or remove the entry
  if (ed != image.exifData().end() && ed->size() > 0) {
    DataBuf buf(ed->size());
    ed->copy(buf.data(), pHead->byteOrder());
    pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, std::move(buf));
  } else {
    pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
  }
}  // CrwMap::encodeBasic

void CrwMap::encode0x0805(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead) {
  std::string comment = image.comment();

  CiffComponent* cc = pHead->findComponent(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
  if (!comment.empty()) {
    auto size = comment.size();
    if (cc && cc->size() > size)
      size = cc->size();
    DataBuf buf(size);
    std::move(comment.begin(), comment.end(), buf.begin());
    pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, std::move(buf));
  } else {
    if (cc) {
      // Just delete the value, do not remove the tag
      DataBuf buf(cc->size());
      cc->setValue(std::move(buf));
    }
  }
}  // CrwMap::encode0x0805

void CrwMap::encode0x080a(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead) {
  const ExifKey k1("Exif.Image.Make");
  const ExifKey k2("Exif.Image.Model");
  const auto ed1 = image.exifData().findKey(k1);
  const auto ed2 = image.exifData().findKey(k2);
  const auto edEnd = image.exifData().end();

  size_t size{0};
  if (ed1 != edEnd)
    size += ed1->size();
  if (ed2 != edEnd)
    size += ed2->size();
  if (size != 0) {
    DataBuf buf(size);
    size_t pos{0};
    if (ed1 != edEnd) {
      ed1->copy(buf.data(), pHead->byteOrder());
      pos += ed1->size();
    }
    if (ed2 != edEnd) {
      ed2->copy(buf.data(pos), pHead->byteOrder());
      pos += ed2->size();
    }
    pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, std::move(buf));
  } else {
    pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
  }
}

void CrwMap::encodeArray(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead) {
  auto ifdId = [=] {
    switch (pCrwMapping->tag_) {
      case 0x0001:
        return IfdId::canonCsId;
      case 0x0004:
        return IfdId::canonSiId;
      case 0x000f:
        return IfdId::canonCfId;
      case 0x0012:
        return IfdId::canonPiId;
    }
    return IfdId::ifdIdNotSet;
  }();
  DataBuf buf = packIfdId(image.exifData(), ifdId, pHead->byteOrder());
  if (buf.empty()) {
    // Try the undecoded tag
    encodeBasic(image, pCrwMapping, pHead);
  }
  if (!buf.empty()) {
    // Write the number of shorts to the beginning of buf
    buf.write_uint16(0, static_cast<uint16_t>(buf.size()), pHead->byteOrder());
    pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, std::move(buf));
  } else {
    pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
  }
}  // CrwMap::encodeArray

void CrwMap::encode0x180e(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead) {
  time_t t = 0;
  const ExifKey key(pCrwMapping->tag_, Internal::groupName(pCrwMapping->ifdId_));
  if (auto ed = image.exifData().findKey(key); ed != image.exifData().end()) {
    tm tm = {};
    if (exifTime(ed->toString().c_str(), &tm) == 0) {
      t = ::mktime(&tm);
    }
  }
  if (t != 0) {
    DataBuf buf(12);
    buf.write_uint32(0, static_cast<uint32_t>(t), pHead->byteOrder());
    pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, std::move(buf));
  } else {
    pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
  }
}  // CrwMap::encode0x180e

void CrwMap::encode0x1810(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead) {
  const ExifKey kX("Exif.Photo.PixelXDimension");
  const ExifKey kY("Exif.Photo.PixelYDimension");
  const ExifKey kO("Exif.Image.Orientation");
  const ExifData& exivData = image.exifData();
  const auto edX = exivData.findKey(kX);
  const auto edY = exivData.findKey(kY);
  const auto edO = exivData.findKey(kO);
  const auto edEnd = exivData.end();

  auto cc = pHead->findComponent(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
  if (edX != edEnd || edY != edEnd || edO != edEnd) {
    size_t size = 28;
    if (cc) {
      if (cc->size() < size)
        throw Error(ErrorCode::kerCorruptedMetadata);
      size = cc->size();
    }
    DataBuf buf(size);
    if (cc)
      std::copy_n(cc->pData() + 8, cc->size() - 8, buf.begin() + 8);
    if (edX != edEnd && edX->size() == 4) {
      edX->copy(buf.data(), pHead->byteOrder());
    }
    if (edY != edEnd && edY->size() == 4) {
      edY->copy(buf.data(4), pHead->byteOrder());
    }
    int32_t d = 0;
    if (edO != edEnd && edO->count() > 0 && edO->typeId() == unsignedShort) {
      d = RotationMap::degrees(static_cast<uint16_t>(edO->toInt64()));
    }
    buf.write_uint32(12, d, pHead->byteOrder());
    pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, std::move(buf));
  } else {
    pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
  }
}  // CrwMap::encode0x1810

void CrwMap::encode0x2008(const Image& image, const CrwMapping* pCrwMapping, CiffHeader* pHead) {
  ExifThumbC exifThumb(image.exifData());
  DataBuf buf = exifThumb.copy();
  if (!buf.empty()) {
    pHead->add(pCrwMapping->crwTagId_, pCrwMapping->crwDir_, std::move(buf));
  } else {
    pHead->remove(pCrwMapping->crwTagId_, pCrwMapping->crwDir_);
  }
}  // CrwMap::encode0x2008

// *************************************************************************
// free functions
DataBuf packIfdId(const ExifData& exifData, IfdId ifdId, ByteOrder byteOrder) {
  const uint16_t size = 1024;
  DataBuf buf(size);

  uint16_t len = 0;

  for (auto&& exif : exifData) {
    if (exif.ifdId() != ifdId)
      continue;
    const uint16_t s = (exif.tag() * 2) + static_cast<uint16_t>(exif.size());
    if (s <= size) {
      len = std::max(len, s);
      exif.copy(buf.data(exif.tag() * 2), byteOrder);
    } else {
      EXV_ERROR << "packIfdId out-of-bounds error: s = " << std::dec << s << "\n";
    }
  }
  // Round the size to make it even.
  buf.resize(len + (len % 2));
  return buf;
}

}  // namespace Exiv2::Internal
