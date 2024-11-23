// SPDX-License-Identifier: GPL-2.0-or-later
// included header files
#include "config.h"

#include "enforce.hpp"
#include "error.hpp"
#include "makernote_int.hpp"
#include "safe_op.hpp"
#include "sonymn_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "tiffvisitor_int.hpp"
#include "utils.hpp"
#include "value.hpp"

#include <iostream>

// *****************************************************************************
namespace {
//! Add \em tobe - \em curr 0x00 filler bytes if necessary
size_t fillGap(Exiv2::Internal::IoWrapper& ioWrapper, size_t curr, size_t tobe);
}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
bool TiffMappingInfo::operator==(const TiffMappingInfo::Key& key) const {
  return (0 == strcmp("*", make_) || startsWith(key.m_, make_)) &&
         (Tag::all == extendedTag_ || key.e_ == extendedTag_) && key.g_ == group_;
}

IoWrapper::IoWrapper(BasicIo& io, const byte* pHeader, size_t size, OffsetWriter* pow) :
    io_(io), pHeader_(pHeader), size_(size), pow_(pow) {
  if (!pHeader_ || size_ == 0)
    wroteHeader_ = true;
}

size_t IoWrapper::write(const byte* pData, size_t wcount) {
  if (!wroteHeader_ && wcount > 0) {
    io_.write(pHeader_, size_);
    wroteHeader_ = true;
  }
  return io_.write(pData, wcount);
}

int IoWrapper::putb(byte data) {
  if (!wroteHeader_) {
    io_.write(pHeader_, size_);
    wroteHeader_ = true;
  }
  return io_.putb(data);
}

void IoWrapper::setTarget(int id, size_t target) {
  if (target > std::numeric_limits<uint32_t>::max()) {
    throw Error(ErrorCode::kerOffsetOutOfRange);
  }
  if (pow_)
    pow_->setTarget(static_cast<OffsetWriter::OffsetId>(id), static_cast<uint32_t>(target));
}

TiffDirectory::TiffDirectory(uint16_t tag, IfdId group, bool hasNext) : TiffComponent(tag, group), hasNext_(hasNext) {
}

TiffSubIfd::TiffSubIfd(uint16_t tag, IfdId group, IfdId newGroup) :
    TiffEntryBase(tag, group, ttUnsignedLong), newGroup_(newGroup) {
}

TiffIfdMakernote::TiffIfdMakernote(uint16_t tag, IfdId group, IfdId mnGroup, MnHeader* pHeader, bool hasNext) :
    TiffComponent(tag, group), pHeader_(pHeader), ifd_(tag, mnGroup, hasNext) {
}

TiffBinaryArray::TiffBinaryArray(uint16_t tag, IfdId group, const ArrayCfg& arrayCfg, const ArrayDef* arrayDef,
                                 size_t defSize) :
    TiffEntryBase(tag, group, arrayCfg.elTiffType_), arrayCfg_(&arrayCfg), arrayDef_(arrayDef), defSize_(defSize) {
}

TiffBinaryArray::TiffBinaryArray(uint16_t tag, IfdId group, const ArraySet* arraySet, size_t setSize,
                                 CfgSelFct cfgSelFct) :
    TiffEntryBase(tag, group),  // Todo: Does it make a difference that there is no type?
    cfgSelFct_(cfgSelFct),
    arraySet_(arraySet),
    setSize_(setSize) {
  // We'll figure out the correct cfg later
}

TiffDirectory::~TiffDirectory() {
  for (auto&& component : components_) {
    delete component;
  }
  delete pNext_;
}

TiffSubIfd::~TiffSubIfd() {
  for (auto&& ifd : ifds_) {
    delete ifd;
  }
}

TiffEntryBase::~TiffEntryBase() {
  delete pValue_;
}

TiffMnEntry::~TiffMnEntry() {
  delete mn_;
}

TiffIfdMakernote::~TiffIfdMakernote() {
  delete pHeader_;
}

TiffBinaryArray::~TiffBinaryArray() {
  for (auto&& element : elements_) {
    delete element;
  }
}

TiffEntryBase::TiffEntryBase(const TiffEntryBase& rhs) :
    TiffComponent(rhs),
    tiffType_(rhs.tiffType_),
    count_(rhs.count_),
    offset_(rhs.offset_),
    size_(rhs.size_),
    pData_(rhs.pData_),
    idx_(rhs.idx_),
    pValue_(rhs.pValue_ ? rhs.pValue_->clone().release() : nullptr),
    storage_(rhs.storage_) {
}

TiffComponent::UniquePtr TiffComponent::clone() const {
  return UniquePtr(doClone());
}

TiffEntry* TiffEntry::doClone() const {
  return new TiffEntry(*this);
}

TiffDataEntry* TiffDataEntry::doClone() const {
  return new TiffDataEntry(*this);
}

TiffImageEntry* TiffImageEntry::doClone() const {
  return new TiffImageEntry(*this);
}

TiffSizeEntry* TiffSizeEntry::doClone() const {
  return new TiffSizeEntry(*this);
}

TiffDirectory* TiffDirectory::doClone() const {
  return new TiffDirectory(*this);
}

TiffSubIfd* TiffSubIfd::doClone() const {
  return new TiffSubIfd(*this);
}

TiffMnEntry* TiffMnEntry::doClone() const {
  return nullptr;
}

TiffIfdMakernote* TiffIfdMakernote::doClone() const {
  return nullptr;
}

TiffBinaryArray* TiffBinaryArray::doClone() const {
  return new TiffBinaryArray(*this);
}

TiffBinaryElement* TiffBinaryElement::doClone() const {
  return new TiffBinaryElement(*this);
}

int TiffComponent::idx() const {
  return 0;
}

int TiffEntryBase::idx() const {
  return idx_;
}

void TiffEntryBase::setData(std::shared_ptr<DataBuf> buf) {
  storage_ = std::move(buf);
  pData_ = storage_->data();
  size_ = storage_->size();
}

void TiffEntryBase::setData(byte* pData, size_t size, std::shared_ptr<DataBuf> storage) {
  pData_ = pData;
  size_ = size;
  storage_ = std::move(storage);
  if (!pData_)
    size_ = 0;
}

void TiffEntryBase::updateValue(Value::UniquePtr value, ByteOrder byteOrder) {
  if (!value)
    return;
  if (size_t newSize = value->size(); newSize > size_) {
    auto d = std::make_shared<DataBuf>(newSize);
    setData(std::move(d));
  }
  if (pData_) {
    memset(pData_, 0x0, size_);
  }
  size_ = value->copy(pData_, byteOrder);
  setValue(std::move(value));
}

void TiffEntryBase::setValue(Value::UniquePtr value) {
  if (!value)
    return;
  tiffType_ = toTiffType(value->typeId());
  count_ = value->count();
  delete pValue_;
  pValue_ = value.release();
}

void TiffDataEntry::setStrips(const Value* pSize, const byte* pData, size_t sizeData, size_t baseOffset) {
  if (!pValue() || !pSize) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0') << std::hex
                << tag() << ": Size or data offset value not set, ignoring them.\n";
#endif
    return;
  }
  if (pValue()->count() == 0) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0') << std::hex
                << tag() << ": Data offset entry value is empty, ignoring it.\n";
#endif
    return;
  }
  if (pValue()->count() != pSize->count()) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0') << std::hex
                << tag() << ": Size and data offset entries have different"
                << " number of components, ignoring them.\n";
#endif
    return;
  }
  size_t size = 0;
  for (size_t i = 0; i < pSize->count(); ++i) {
    size = Safe::add<size_t>(size, pSize->toUint32(i));
  }
  const size_t offset = pValue()->toUint32(0);
  if (size > sizeData || offset > sizeData - size || baseOffset > sizeData - size - offset) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0') << std::hex
                << tag() << ": Data area exceeds data buffer, ignoring it.\n";
#endif
    return;
  }
  // Todo: Remove limitation of JPEG writer: strips must be contiguous
  // Until then we check: last offset + last size - first offset == size?
  if (pValue()->toUint32(pValue()->count() - 1) + pSize->toUint32(pSize->count() - 1) != size + offset) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0') << std::hex
                << tag() << ": Data area is not contiguous, ignoring it.\n";
#endif
    return;
  }
  pDataArea_ = const_cast<byte*>(pData) + baseOffset + offset;
  sizeDataArea_ = size;
  const_cast<Value*>(pValue())->setDataArea(pDataArea_, sizeDataArea_);
}  // TiffDataEntry::setStrips

void TiffImageEntry::setStrips(const Value* pSize, const byte* pData, size_t sizeData, size_t baseOffset) {
  if (!pValue() || !pSize) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0') << std::hex
                << tag() << ": Size or data offset value not set, ignoring them.\n";
#endif
    return;
  }
  if (pValue()->count() != pSize->count()) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0') << std::hex
                << tag() << ": Size and data offset entries have different"
                << " number of components, ignoring them.\n";
#endif
    return;
  }
  for (size_t i = 0; i < pValue()->count(); ++i) {
    const size_t offset = pValue()->toUint32(i);
    const size_t size = pSize->toUint32(i);

    if (size > sizeData || offset > sizeData - size || baseOffset > sizeData - size - offset) {
#ifndef SUPPRESS_WARNINGS
      EXV_WARNING << "Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0') << std::hex
                  << tag() << ": Strip " << std::dec << i << " is outside of the data area; ignored.\n";
#endif
    } else if (size != 0) {
      const byte* pStrip = pData + baseOffset + offset;
      strips_.emplace_back(pStrip, size);
    }
  }
}  // TiffImageEntry::setStrips

size_t TiffIfdMakernote::ifdOffset() const {
  if (!pHeader_)
    return 0;
  return pHeader_->ifdOffset();
}

ByteOrder TiffIfdMakernote::byteOrder() const {
  if (!pHeader_ || pHeader_->byteOrder() == invalidByteOrder) {
    return imageByteOrder_;
  }
  return pHeader_->byteOrder();
}

size_t TiffIfdMakernote::mnOffset() const {
  return mnOffset_;
}

size_t TiffIfdMakernote::baseOffset() const {
  if (!pHeader_)
    return 0;
  return pHeader_->baseOffset(mnOffset_);
}

bool TiffIfdMakernote::readHeader(const byte* pData, size_t size, ByteOrder byteOrder) {
  if (!pHeader_)
    return true;
  return pHeader_->read(pData, size, byteOrder);
}

void TiffIfdMakernote::setByteOrder(ByteOrder byteOrder) {
  if (pHeader_)
    pHeader_->setByteOrder(byteOrder);
}

size_t TiffIfdMakernote::sizeHeader() const {
  if (!pHeader_)
    return 0;
  return pHeader_->size();
}

size_t TiffIfdMakernote::writeHeader(IoWrapper& ioWrapper, ByteOrder byteOrder) const {
  if (!pHeader_)
    return 0;
  return pHeader_->write(ioWrapper, byteOrder);
}

size_t ArrayDef::size(uint16_t tag, IfdId group) const {
  TypeId typeId = toTypeId(tiffType_, tag, group);
  return count_ * TypeInfo::typeSize(typeId);
}

bool TiffBinaryArray::initialize(IfdId group) {
  if (arrayCfg_)
    return true;  // Not a complex array or already initialized

  for (size_t idx = 0; idx < setSize_; ++idx) {
    if (arraySet_[idx].cfg_.group_ == group) {
      arrayCfg_ = &arraySet_[idx].cfg_;
      arrayDef_ = arraySet_[idx].def_;
      defSize_ = arraySet_[idx].defSize_;
      return true;
    }
  }
  return false;
}

bool TiffBinaryArray::initialize(TiffComponent* pRoot) {
  if (!cfgSelFct_)
    return true;  // Not a complex array

  int idx = cfgSelFct_(tag(), pData(), TiffEntryBase::doSize(), pRoot);
  if (idx > -1) {
    arrayCfg_ = &arraySet_[idx].cfg_;
    arrayDef_ = arraySet_[idx].def_;
    defSize_ = arraySet_[idx].defSize_;
  }
  return idx > -1;
}

void TiffBinaryArray::iniOrigDataBuf() {
  origData_ = const_cast<byte*>(pData());
  origSize_ = TiffEntryBase::doSize();
}

bool TiffBinaryArray::updOrigDataBuf(const byte* pData, size_t size) {
  if (origSize_ != size)
    return false;
  if (origData_ == pData)
    return true;
  std::copy_n(pData, origSize_, origData_);
  return true;
}

size_t TiffBinaryArray::addElement(size_t idx, const ArrayDef& def) {
  auto tag = static_cast<uint16_t>(idx / cfg()->tagStep());
  auto sz = std::min<size_t>(def.size(tag, cfg()->group_), TiffEntryBase::doSize() - idx);
  auto tc = TiffCreator::create(tag, cfg()->group_);
  auto tp = dynamic_cast<TiffBinaryElement*>(tc.get());
  if (!tp)
    return 0;
  // The assertion typically fails if a component is not configured in
  // the TIFF structure table (TiffCreator::tiffTreeStruct_)
  tp->setStart(pData() + idx);
  auto s = storage();
  tp->setData(const_cast<byte*>(pData() + idx), sz, std::move(s));
  tp->setElDef(def);
  tp->setElByteOrder(cfg()->byteOrder_);
  addChild(std::move(tc));
  return sz;
}  // TiffBinaryArray::addElement

TiffComponent* TiffComponent::addPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                                      TiffComponent::UniquePtr object) {
  return doAddPath(tag, tiffPath, pRoot, std::move(object));
}  // TiffComponent::addPath

TiffComponent* TiffComponent::doAddPath(uint16_t /*tag*/, TiffPath& /*tiffPath*/, TiffComponent* /*pRoot*/,
                                        TiffComponent::UniquePtr /*object*/) {
  return this;
}  // TiffComponent::doAddPath

TiffComponent* TiffDirectory::doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                                        TiffComponent::UniquePtr object) {
  tiffPath.pop();
  const TiffPathItem tpi = tiffPath.top();

  TiffComponent* tc = nullptr;
  // Try to use an existing component if there is still at least one
  // composite tag on the stack or the tag to add is the MakerNote tag.
  // This is used to prevent duplicate entries. Sub-IFDs also, but the > 1
  // condition takes care of them, see below.
  if (tiffPath.size() > 1 || (tpi.extendedTag() == 0x927c && tpi.group() == IfdId::exifId)) {
    if (tpi.extendedTag() == Tag::next) {
      tc = pNext_;
    } else {
      for (auto&& component : components_) {
        if (component->tag() == tpi.tag() && component->group() == tpi.group()) {
          tc = component;
          break;
        }
      }
    }
  }

  if (tc)
    return tc->addPath(tag, tiffPath, pRoot, std::move(object));

  auto atc = [&] {
    if (tiffPath.size() == 1 && object) {
      TiffComponent::UniquePtr tempObject;
      std::swap(object, tempObject);
      return tempObject;
    }
    return TiffCreator::create(tpi.extendedTag(), tpi.group());
  }();

  // Prevent dangling sub-IFD tags: Do not add a sub-IFD component without children.
  // Todo: How to check before creating the component?
  if (tiffPath.size() == 1 && dynamic_cast<TiffSubIfd*>(atc.get()))
    return nullptr;

  tc = [&] {
    if (tpi.extendedTag() == Tag::next)
      return this->addNext(std::move(atc));
    return this->addChild(std::move(atc));
  }();
  return tc->addPath(tag, tiffPath, pRoot, std::move(object));
}  // TiffDirectory::doAddPath

TiffComponent* TiffSubIfd::doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                                     TiffComponent::UniquePtr object) {
  const TiffPathItem tpi1 = tiffPath.top();
  tiffPath.pop();
  if (tiffPath.empty()) {
    // If the last element in the path is the sub-IFD tag itself we're done.
    // But that shouldn't happen - see TiffDirectory::doAddPath
    return this;
  }
  const TiffPathItem tpi2 = tiffPath.top();
  tiffPath.push(tpi1);
  auto it = std::find_if(ifds_.begin(), ifds_.end(), [&](auto&& ifd) { return ifd->group() == tpi2.group(); });
  if (it == ifds_.end()) {
    auto tc = [&] {
      if (tiffPath.size() == 1 && object) {
        TiffComponent::UniquePtr tempObject;
        std::swap(object, tempObject);
        return addChild(std::move(tempObject));
      }
      return addChild(std::make_unique<TiffDirectory>(tpi1.tag(), tpi2.group()));
    }();
    setCount(ifds_.size());
    return tc->addPath(tag, tiffPath, pRoot, std::move(object));
  }
  return (*it)->addPath(tag, tiffPath, pRoot, std::move(object));
}  // TiffSubIfd::doAddPath

TiffComponent* TiffMnEntry::doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                                      TiffComponent::UniquePtr object) {
  const TiffPathItem tpi1 = tiffPath.top();
  tiffPath.pop();
  if (tiffPath.empty()) {
    // If the last element in the path is the makernote tag itself we're done
    return this;
  }
  const TiffPathItem tpi2 = tiffPath.top();
  tiffPath.push(tpi1);
  if (!mn_) {
    mnGroup_ = tpi2.group();
    mn_ = TiffMnCreator::create(tpi1.tag(), tpi1.group(), mnGroup_);
  }
  return mn_->addPath(tag, tiffPath, pRoot, std::move(object));
}  // TiffMnEntry::doAddPath

TiffComponent* TiffIfdMakernote::doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                                           TiffComponent::UniquePtr object) {
  return ifd_.addPath(tag, tiffPath, pRoot, std::move(object));
}

TiffComponent* TiffBinaryArray::doAddPath(uint16_t tag, TiffPath& tiffPath, TiffComponent* pRoot,
                                          TiffComponent::UniquePtr object) {
  pRoot_ = pRoot;
  if (tiffPath.size() == 1) {
    // An unknown complex binary array has no children and acts like a standard TIFF entry
    return this;
  }
  tiffPath.pop();
  const TiffPathItem tpi = tiffPath.top();
  // Initialize the binary array (if it is a complex array)
  initialize(tpi.group());
  auto it = elements_.end();
  // Todo: Duplicates are not allowed!
  // To allow duplicate entries, we only check if the new component already
  // exists if there is still at least one composite tag on the stack
  if (tiffPath.size() > 1) {
    it = std::find_if(elements_.begin(), it,
                      [&](auto&& element) { return element->tag() == tpi.tag() && element->group() == tpi.group(); });
  }

  if (it != elements_.end())
    return (*it)->addPath(tag, tiffPath, pRoot, std::move(object));

  auto atc = [&] {
    if (tiffPath.size() == 1 && object) {
      TiffComponent::UniquePtr tempObject;
      std::swap(object, tempObject);
      return tempObject;
    }
    return TiffCreator::create(tpi.extendedTag(), tpi.group());
  }();
  auto tc = addChild(std::move(atc));
  setCount(elements_.size());
  return tc->addPath(tag, tiffPath, pRoot, std::move(object));
}  // TiffBinaryArray::doAddPath

TiffComponent* TiffComponent::addChild(TiffComponent::UniquePtr tiffComponent) {
  return doAddChild(std::move(tiffComponent));
}  // TiffComponent::addChild

TiffComponent* TiffComponent::doAddChild(UniquePtr /*tiffComponent*/) {
  return nullptr;
}  // TiffComponent::doAddChild

TiffComponent* TiffDirectory::doAddChild(TiffComponent::UniquePtr tiffComponent) {
  TiffComponent* tc = tiffComponent.release();
  components_.push_back(tc);
  return tc;
}  // TiffDirectory::doAddChild

TiffComponent* TiffSubIfd::doAddChild(TiffComponent::UniquePtr tiffComponent) {
  auto d = dynamic_cast<TiffDirectory*>(tiffComponent.release());
  ifds_.push_back(d);
  return d;
}  // TiffSubIfd::doAddChild

TiffComponent* TiffMnEntry::doAddChild(TiffComponent::UniquePtr tiffComponent) {
  TiffComponent* tc = nullptr;
  if (mn_) {
    tc = mn_->addChild(std::move(tiffComponent));
  }
  return tc;
}  // TiffMnEntry::doAddChild

TiffComponent* TiffIfdMakernote::doAddChild(TiffComponent::UniquePtr tiffComponent) {
  return ifd_.addChild(std::move(tiffComponent));
}

TiffComponent* TiffBinaryArray::doAddChild(TiffComponent::UniquePtr tiffComponent) {
  TiffComponent* tc = tiffComponent.release();
  elements_.push_back(tc);
  setDecoded(true);
  return tc;
}  // TiffBinaryArray::doAddChild

TiffComponent* TiffComponent::addNext(TiffComponent::UniquePtr tiffComponent) {
  return doAddNext(std::move(tiffComponent));
}  // TiffComponent::addNext

TiffComponent* TiffComponent::doAddNext(UniquePtr /*tiffComponent*/) {
  return nullptr;
}  // TiffComponent::doAddNext

TiffComponent* TiffDirectory::doAddNext(TiffComponent::UniquePtr tiffComponent) {
  TiffComponent* tc = nullptr;
  if (hasNext_) {
    tc = tiffComponent.release();
    pNext_ = tc;
  }
  return tc;
}  // TiffDirectory::doAddNext

TiffComponent* TiffMnEntry::doAddNext(TiffComponent::UniquePtr tiffComponent) {
  TiffComponent* tc = nullptr;
  if (mn_) {
    tc = mn_->addNext(std::move(tiffComponent));
  }
  return tc;
}  // TiffMnEntry::doAddNext

TiffComponent* TiffIfdMakernote::doAddNext(TiffComponent::UniquePtr tiffComponent) {
  return ifd_.addNext(std::move(tiffComponent));
}

void TiffComponent::accept(TiffVisitor& visitor) {
  if (visitor.go(TiffVisitor::geTraverse))
    doAccept(visitor);  // one for NVI :)
}  // TiffComponent::accept

void TiffEntry::doAccept(TiffVisitor& visitor) {
  visitor.visitEntry(this);
}  // TiffEntry::doAccept

void TiffDataEntry::doAccept(TiffVisitor& visitor) {
  visitor.visitDataEntry(this);
}  // TiffDataEntry::doAccept

void TiffImageEntry::doAccept(TiffVisitor& visitor) {
  visitor.visitImageEntry(this);
}  // TiffImageEntry::doAccept

void TiffSizeEntry::doAccept(TiffVisitor& visitor) {
  visitor.visitSizeEntry(this);
}  // TiffSizeEntry::doAccept

void TiffDirectory::doAccept(TiffVisitor& visitor) {
  visitor.visitDirectory(this);
  for (auto&& component : components_) {
    if (!visitor.go(TiffVisitor::geTraverse))
      break;
    component->accept(visitor);
  }
  if (visitor.go(TiffVisitor::geTraverse))
    visitor.visitDirectoryNext(this);
  if (pNext_)
    pNext_->accept(visitor);
  if (visitor.go(TiffVisitor::geTraverse))
    visitor.visitDirectoryEnd(this);
}  // TiffDirectory::doAccept

void TiffSubIfd::doAccept(TiffVisitor& visitor) {
  visitor.visitSubIfd(this);
  for (auto&& ifd : ifds_) {
    if (!visitor.go(TiffVisitor::geTraverse))
      break;
    ifd->accept(visitor);
  }
}  // TiffSubIfd::doAccept

void TiffMnEntry::doAccept(TiffVisitor& visitor) {
  visitor.visitMnEntry(this);
  if (mn_)
    mn_->accept(visitor);
  if (!visitor.go(TiffVisitor::geKnownMakernote)) {
    delete mn_;
    mn_ = nullptr;
  }

}  // TiffMnEntry::doAccept

void TiffIfdMakernote::doAccept(TiffVisitor& visitor) {
  if (visitor.go(TiffVisitor::geTraverse))
    visitor.visitIfdMakernote(this);
  if (visitor.go(TiffVisitor::geKnownMakernote))
    ifd_.accept(visitor);
  if (visitor.go(TiffVisitor::geKnownMakernote) && visitor.go(TiffVisitor::geTraverse))
    visitor.visitIfdMakernoteEnd(this);
}

void TiffBinaryArray::doAccept(TiffVisitor& visitor) {
  visitor.visitBinaryArray(this);
  for (auto&& element : elements_) {
    if (!visitor.go(TiffVisitor::geTraverse))
      break;
    element->accept(visitor);
  }
  if (visitor.go(TiffVisitor::geTraverse))
    visitor.visitBinaryArrayEnd(this);
}

void TiffBinaryElement::doAccept(TiffVisitor& visitor) {
  visitor.visitBinaryElement(this);
}

void TiffEntryBase::encode(TiffEncoder& encoder, const Exifdatum* datum) {
  doEncode(encoder, datum);
}

void TiffBinaryElement::doEncode(TiffEncoder& encoder, const Exifdatum* datum) {
  encoder.encodeBinaryElement(this, datum);
}

void TiffBinaryArray::doEncode(TiffEncoder& encoder, const Exifdatum* datum) {
  encoder.encodeBinaryArray(this, datum);
}

void TiffDataEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum) {
  encoder.encodeDataEntry(this, datum);
}

void TiffEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum) {
  encoder.encodeTiffEntry(this, datum);
}

void TiffImageEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum) {
  encoder.encodeImageEntry(this, datum);
}

void TiffMnEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum) {
  encoder.encodeMnEntry(this, datum);
}

void TiffSizeEntry::doEncode(TiffEncoder& encoder, const Exifdatum* datum) {
  encoder.encodeSizeEntry(this, datum);
}

void TiffSubIfd::doEncode(TiffEncoder& encoder, const Exifdatum* datum) {
  encoder.encodeSubIfd(this, datum);
}

size_t TiffComponent::count() const {
  return doCount();
}

size_t TiffDirectory::doCount() const {
  return components_.size();
}

size_t TiffEntryBase::doCount() const {
  return count_;
}

size_t TiffMnEntry::doCount() const {
  if (!mn_) {
    return TiffEntryBase::doCount();
  }
#ifndef SUPPRESS_WARNINGS
  // Count of IFD makernote in tag Exif.Photo.MakerNote is the size of the
  // Makernote in bytes
  if (tiffType() != ttUndefined && tiffType() != ttUnsignedByte && tiffType() != ttSignedByte) {
    EXV_ERROR << "Makernote entry 0x" << std::setw(4) << std::setfill('0') << std::hex << tag()
              << " has incorrect Exif (TIFF) type " << std::dec << tiffType()
              << ". (Expected signed or unsigned byte.)\n";
  }
#endif
  return mn_->size();
}

size_t TiffIfdMakernote::doCount() const {
  return ifd_.count();
}  // TiffIfdMakernote::doCount

size_t TiffBinaryArray::doCount() const {
  if (!cfg() || !decoded())
    return TiffEntryBase::doCount();

  if (elements_.empty())
    return 0;

  TypeId typeId = toTypeId(tiffType(), tag(), group());
  size_t typeSize = TypeInfo::typeSize(typeId);
  if (0 == typeSize) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0') << std::hex
                << tag() << " has unknown Exif (TIFF) type " << std::dec << tiffType() << "; setting type size 1.\n";
#endif
    typeSize = 1;
  }

  return std::lround(static_cast<double>(size()) / typeSize);
}

size_t TiffBinaryElement::doCount() const {
  return elDef_.count_;
}

size_t TiffComponent::write(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                            size_t& imageIdx) {
  return doWrite(ioWrapper, byteOrder, offset, valueIdx, dataIdx, imageIdx);
}  // TiffComponent::write

size_t TiffDirectory::doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                              size_t& imageIdx) {
  bool isRootDir = (imageIdx == std::string::npos);

  // Number of components to write
  const size_t compCount = count();
  if (compCount > 0xffff)
    throw Error(ErrorCode::kerTooManyTiffDirectoryEntries, groupName(group()));

  // Size of next IFD, if any
  size_t sizeNext = 0;
  if (pNext_)
    sizeNext = pNext_->size();

  // Nothing to do if there are no entries and the size of the next IFD is 0
  if (compCount == 0 && sizeNext == 0)
    return 0;

  // Remember the offset of the CR2 RAW IFD
  if (group() == IfdId::ifd3Id) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Directory " << groupName(group()) << " offset is 0x" << std::setw(8) << std::setfill('0') << std::hex
              << offset << std::dec << "\n";
#endif
    ioWrapper.setTarget(OffsetWriter::cr2RawIfdOffset, offset);
  }
  // Size of all directory entries, without values and additional data
  const size_t sizeDir = 2 + (12 * compCount) + (hasNext_ ? 4 : 0);

  // TIFF standard requires IFD entries to be sorted in ascending order by tag.
  // Not sorting makernote directories sometimes preserves them better.
  if (group() < IfdId::mnId) {
    std::sort(components_.begin(), components_.end(), cmpTagLt);
  }
  // Size of IFD values and additional data
  size_t sizeValue = 0;
  size_t sizeData = 0;
  for (auto&& component : components_) {
    if (size_t sv = component->size(); sv > 4) {
      sv += sv & 1;  // Align value to word boundary
      sizeValue += sv;
    }
    // Also add the size of data, but only if needed
    if (isRootDir) {
      auto sd = component->sizeData();
      sd += sd & 1;  // Align data to word boundary
      sizeData += sd;
    }
  }

  size_t idx = 0;                 // Current IFD index / bytes written
  valueIdx = sizeDir;             // Offset to the current IFD value
  dataIdx = sizeDir + sizeValue;  // Offset to the entry's data area
  if (isRootDir) {                // Absolute offset to the image data
    imageIdx = offset + dataIdx + sizeData + sizeNext;
    imageIdx += imageIdx & 1;  // Align image data to word boundary
  }

  // 1st: Write the IFD, a) Number of directory entries
  byte buf[4];
  us2Data(buf, static_cast<uint16_t>(compCount), byteOrder);
  ioWrapper.write(buf, 2);
  idx += 2;
  // b) Directory entries - may contain pointers to the value or data
  for (auto&& component : components_) {
    idx += writeDirEntry(ioWrapper, byteOrder, offset, component, valueIdx, dataIdx, imageIdx);
    if (size_t sv = component->size(); sv > 4) {
      sv += sv & 1;  // Align value to word boundary
      valueIdx += sv;
    }
    auto sd = component->sizeData();
    sd += sd & 1;  // Align data to word boundary
    dataIdx += sd;
  }
  // c) Pointer to the next IFD
  if (hasNext_) {
    memset(buf, 0x0, 4);
    if (pNext_ && sizeNext) {
      l2Data(buf, static_cast<uint32_t>(offset + dataIdx), byteOrder);
    }
    ioWrapper.write(buf, 4);
    idx += 4;
  }

  // 2nd: Write IFD values - may contain pointers to additional data
  valueIdx = sizeDir;
  dataIdx = sizeDir + sizeValue;
  for (auto&& component : components_) {
    if (size_t sv = component->size(); sv > 4) {
      size_t d = component->write(ioWrapper, byteOrder, offset, valueIdx, dataIdx, imageIdx);
      enforce(sv == d, ErrorCode::kerImageWriteFailed);
      if ((sv & 1) == 1) {
        ioWrapper.putb(0x0);  // Align value to word boundary
        sv += 1;
      }
      idx += sv;
      valueIdx += sv;
    }
    auto sd = component->sizeData();
    sd += sd & 1;  // Align data to word boundary
    dataIdx += sd;
  }

  // 3rd: Write data - may contain offsets too (eg sub-IFD)
  dataIdx = sizeDir + sizeValue;
  idx += writeData(ioWrapper, byteOrder, offset, dataIdx, imageIdx);

  // 4th: Write next-IFD
  if (pNext_ && sizeNext) {
    idx += pNext_->write(ioWrapper, byteOrder, offset + idx, std::string::npos, std::string::npos, imageIdx);
  }

  // 5th, at the root directory level only: write image data
  if (isRootDir) {
    idx += writeImage(ioWrapper, byteOrder);
  }

  return idx;
}

size_t TiffDirectory::writeDirEntry(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset,
                                    TiffComponent* pTiffComponent, size_t valueIdx, size_t dataIdx, size_t& imageIdx) {
  auto pDirEntry = dynamic_cast<TiffEntryBase*>(pTiffComponent);
  if (!pDirEntry)
    return 0;
  byte buf[8];
  us2Data(buf, pDirEntry->tag(), byteOrder);
  us2Data(buf + 2, pDirEntry->tiffType(), byteOrder);
  ul2Data(buf + 4, static_cast<uint32_t>(pDirEntry->count()), byteOrder);
  ioWrapper.write(buf, 8);
  if (pDirEntry->size() > 4) {
    pDirEntry->setOffset(Safe::add<size_t>(offset, valueIdx));
    ul2Data(buf, static_cast<uint32_t>(pDirEntry->offset()), byteOrder);
    ioWrapper.write(buf, 4);
  } else {
    const size_t len = pDirEntry->write(ioWrapper, byteOrder, offset, valueIdx, dataIdx, imageIdx);
#ifndef SUPPRESS_WARNINGS
    if (len > 4) {
      EXV_ERROR << "Unexpected length in TiffDirectory::writeDirEntry(): len == " << len << ".\n";
    }
#endif
    if (len < 4) {
      memset(buf, 0x0, 4);
      ioWrapper.write(buf, 4 - len);
    }
  }
  return 12;
}  // TiffDirectory::writeDirEntry

size_t TiffEntryBase::doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t /*offset*/, size_t /*valueIdx*/,
                              size_t /*dataIdx*/, size_t& /*imageIdx*/) {
  if (!pValue_ || pValue_->size() == 0)
    return 0;

  DataBuf buf(pValue_->size());
  pValue_->copy(buf.data(), byteOrder);
  ioWrapper.write(buf.c_data(), buf.size());
  return buf.size();
}  // TiffEntryBase::doWrite

size_t TiffEntryBase::writeOffset(byte* buf, size_t offset, TiffType tiffType, ByteOrder byteOrder) {
  size_t rc = 0;
  switch (tiffType) {
    case ttUnsignedShort:
    case ttSignedShort:
      if (offset > std::numeric_limits<uint16_t>::max())
        throw Error(ErrorCode::kerOffsetOutOfRange);
      rc = us2Data(buf, static_cast<uint16_t>(offset), byteOrder);
      break;
    case ttUnsignedLong:
    case ttSignedLong:
      rc = l2Data(buf, static_cast<uint32_t>(offset), byteOrder);
      break;
    default:
      throw Error(ErrorCode::kerUnsupportedDataAreaOffsetType);
  }
  return rc;
}  // TiffEntryBase::writeOffset

size_t TiffDataEntry::doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t /*valueIdx*/,
                              size_t dataIdx, size_t& /*imageIdx*/) {
  if (!pValue() || pValue()->count() == 0)
    return 0;

  DataBuf buf(pValue()->size());
  size_t idx = 0;
  const size_t prevOffset = pValue()->toUint32(0);
  for (size_t i = 0; i < count(); ++i) {
    const size_t iOffset = pValue()->toUint32(i);
    enforce(prevOffset <= iOffset, ErrorCode::kerOffsetOutOfRange);
    const auto newDataIdx = Safe::add<size_t>(iOffset - prevOffset, dataIdx);
    idx += writeOffset(buf.data(idx), Safe::add(offset, newDataIdx), tiffType(), byteOrder);
  }
  ioWrapper.write(buf.c_data(), buf.size());
  return buf.size();
}

size_t TiffImageEntry::doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t /*valueIdx*/,
                               size_t dataIdx, size_t& imageIdx) {
  size_t o2 = imageIdx;
  // For makernotes, write TIFF image data to the data area
  if (group() > IfdId::mnId)
    o2 = Safe::add<size_t>(offset, dataIdx);
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "TiffImageEntry, Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0')
            << std::hex << tag() << std::dec << ": Writing offset " << o2 << "\n";
#endif
  DataBuf buf(strips_.size() * 4);
  size_t idx = 0;
  for (const auto& [_, off] : strips_) {
    idx += writeOffset(buf.data(idx), o2, tiffType(), byteOrder);
    // Align strip data to word boundary
    const auto sz = Safe::add(off, off & 1);
    o2 = Safe::add(o2, sz);
    if (group() <= IfdId::mnId)
      imageIdx = Safe::add(imageIdx, sz);
  }
  ioWrapper.write(buf.c_data(), buf.size());
  return buf.size();
}  // TiffImageEntry::doWrite

size_t TiffSubIfd::doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t /*valueIdx*/,
                           size_t dataIdx, size_t& /*imageIdx*/) {
  DataBuf buf(ifds_.size() * 4);
  size_t idx = 0;
  // Sort IFDs by group, needed if image data tags were copied first
  std::sort(ifds_.begin(), ifds_.end(), cmpGroupLt);
  for (auto&& ifd : ifds_) {
    idx += writeOffset(buf.data(idx), offset + dataIdx, tiffType(), byteOrder);
    dataIdx += ifd->size();
  }
  ioWrapper.write(buf.c_data(), buf.size());
  return buf.size();
}  // TiffSubIfd::doWrite

size_t TiffMnEntry::doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx, size_t dataIdx,
                            size_t& imageIdx) {
  if (!mn_) {
    return TiffEntryBase::doWrite(ioWrapper, byteOrder, offset, valueIdx, dataIdx, imageIdx);
  }
  return mn_->write(ioWrapper, byteOrder, offset + valueIdx, std::string::npos, std::string::npos, imageIdx);
}  // TiffMnEntry::doWrite

size_t TiffIfdMakernote::doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t /*valueIdx*/,
                                 size_t /*dataIdx*/, size_t& imageIdx) {
  mnOffset_ = offset;
  setImageByteOrder(byteOrder);
  auto len = writeHeader(ioWrapper, this->byteOrder());
  len += ifd_.write(ioWrapper, this->byteOrder(), offset - baseOffset() + len, std::string::npos, std::string::npos,
                    imageIdx);
  return len;
}  // TiffIfdMakernote::doWrite

size_t TiffBinaryArray::doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t valueIdx,
                                size_t dataIdx, size_t& imageIdx) {
  if (!cfg() || !decoded())
    return TiffEntryBase::doWrite(ioWrapper, byteOrder, offset, valueIdx, dataIdx, imageIdx);
  if (cfg()->byteOrder_ != invalidByteOrder)
    byteOrder = cfg()->byteOrder_;
  // Tags must be sorted in ascending order
  std::sort(elements_.begin(), elements_.end(), cmpTagLt);
  size_t idx = 0;
  MemIo mio;  // memory stream in which to store data
  IoWrapper mioWrapper(mio, nullptr, 0, nullptr);
  // Some array entries need to have the size in the first element
  if (cfg()->hasSize_) {
    byte buf[4];
    size_t elSize = TypeInfo::typeSize(toTypeId(cfg()->elTiffType_, 0, cfg()->group_));
    switch (elSize) {
      case 2:
        idx += us2Data(buf, static_cast<uint16_t>(size()), byteOrder);
        break;
      case 4:
        idx += ul2Data(buf, static_cast<uint32_t>(size()), byteOrder);
        break;
      default:
        break;
    }
    mioWrapper.write(buf, elSize);
  }
  // write all tags of the array (Todo: assumes that there are no duplicates, need check)
  for (auto&& element : elements_) {
    // Skip the manufactured tag, if it exists
    if (cfg()->hasSize_ && element->tag() == 0)
      continue;
    size_t newIdx = element->tag() * cfg()->tagStep();
    idx += fillGap(mioWrapper, idx, newIdx);
    idx += element->write(mioWrapper, byteOrder, offset + newIdx, valueIdx, dataIdx, imageIdx);
  }
  if (cfg()->hasFillers_ && def()) {
    const ArrayDef* lastDef = def() + defSize() - 1;
    auto lastTag = static_cast<uint16_t>(lastDef->idx_ / cfg()->tagStep());
    idx += fillGap(mioWrapper, idx, lastDef->idx_ + lastDef->size(lastTag, cfg()->group_));
  }

  if (cfg()->cryptFct_) {
    // Select sonyTagEncipher
    CryptFct cryptFct = cfg()->cryptFct_;
    if (cryptFct == &sonyTagDecipher) {
      cryptFct = sonyTagEncipher;
    }
    DataBuf buf = cryptFct(tag(), mio.mmap(), mio.size(), pRoot_);
    if (!buf.empty()) {
      mio.seek(0, Exiv2::BasicIo::beg);
      mio.write(buf.c_data(), buf.size());
    }
  }
  ioWrapper.write(mio.mmap(), mio.size());

  return idx;
}  // TiffBinaryArray::doWrite

size_t TiffBinaryElement::doWrite(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t /*offset*/, size_t /*valueIdx*/,
                                  size_t /*dataIdx*/, size_t& /*imageIdx*/) {
  auto pv = pValue();
  if (!pv || pv->count() == 0)
    return 0;
  DataBuf buf(pv->size());
  pv->copy(buf.data(), byteOrder);
  ioWrapper.write(buf.c_data(), buf.size());
  return buf.size();
}  // TiffBinaryElement::doWrite

size_t TiffComponent::writeData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                                size_t& imageIdx) const {
  return doWriteData(ioWrapper, byteOrder, offset, dataIdx, imageIdx);
}  // TiffComponent::writeData

size_t TiffDirectory::doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                                  size_t& imageIdx) const {
  size_t len = 0;
  for (auto&& component : components_) {
    len += component->writeData(ioWrapper, byteOrder, offset, dataIdx + len, imageIdx);
  }
  return len;
}  // TiffDirectory::doWriteData

size_t TiffEntryBase::doWriteData(IoWrapper& /*ioWrapper*/, ByteOrder /*byteOrder*/, size_t /*offset*/,
                                  size_t /*dataIdx*/, size_t& /*imageIdx*/) const {
  return 0;
}  // TiffEntryBase::doWriteData

size_t TiffImageEntry::doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t /*offset*/, size_t /*dataIdx*/,
                                   size_t& /*imageIdx*/) const {
  size_t len = 0;
  // For makernotes, write TIFF image data to the data area
  if (group() > IfdId::mnId) {  // Todo: FIX THIS HACK!!!
    len = writeImage(ioWrapper, byteOrder);
  }
  return len;
}  // TiffImageEntry::doWriteData

size_t TiffDataEntry::doWriteData(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/, size_t /*offset*/, size_t /*dataIdx*/,
                                  size_t& /*imageIdx*/) const {
  if (!pValue())
    return 0;

  DataBuf buf = pValue()->dataArea();
  if (!buf.empty())
    ioWrapper.write(buf.c_data(), buf.size());
  // Align data to word boundary
  size_t align = (buf.size() & 1);
  if (align)
    ioWrapper.putb(0x0);

  return buf.size() + align;
}  // TiffDataEntry::doWriteData

size_t TiffSubIfd::doWriteData(IoWrapper& ioWrapper, ByteOrder byteOrder, size_t offset, size_t dataIdx,
                               size_t& imageIdx) const {
  size_t len = 0;
  for (auto&& ifd : ifds_) {
    len += ifd->write(ioWrapper, byteOrder, offset + dataIdx + len, std::string::npos, std::string::npos, imageIdx);
  }
  // Align data to word boundary
  size_t align = (len & 1);
  if (align)
    ioWrapper.putb(0x0);

  return len + align;
}  // TiffSubIfd::doWriteData

size_t TiffIfdMakernote::doWriteData(IoWrapper& /*ioWrapper*/, ByteOrder /*byteOrder*/, size_t /*offset*/,
                                     size_t /*dataIdx*/, size_t& /*imageIdx*/) const {
  return 0;
}

size_t TiffComponent::writeImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const {
  return doWriteImage(ioWrapper, byteOrder);
}  // TiffComponent::writeImage

size_t TiffDirectory::doWriteImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const {
  size_t len = 0;
  TiffComponent* pSubIfd = nullptr;
  for (auto component : components_) {
    if (component->tag() == 0x014a) {
      // Hack: delay writing of sub-IFD image data to get the order correct
#ifndef SUPPRESS_WARNINGS
      if (pSubIfd) {
        EXV_ERROR << "Multiple sub-IFD image data tags found\n";
      }
#endif
      pSubIfd = component;
      continue;
    }
    len += component->writeImage(ioWrapper, byteOrder);
  }
  if (pSubIfd) {
    len += pSubIfd->writeImage(ioWrapper, byteOrder);
  }
  if (pNext_) {
    len += pNext_->writeImage(ioWrapper, byteOrder);
  }
  return len;
}  // TiffDirectory::doWriteImage

size_t TiffEntryBase::doWriteImage(IoWrapper& /*ioWrapper*/, ByteOrder /*byteOrder*/) const {
  return 0;
}  // TiffEntryBase::doWriteImage

size_t TiffSubIfd::doWriteImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const {
  size_t len = 0;
  for (auto&& ifd : ifds_) {
    len += ifd->writeImage(ioWrapper, byteOrder);
  }
  return len;
}  // TiffSubIfd::doWriteImage

size_t TiffIfdMakernote::doWriteImage(IoWrapper& ioWrapper, ByteOrder byteOrder) const {
  if (this->byteOrder() != invalidByteOrder) {
    byteOrder = this->byteOrder();
  }
  size_t len = ifd_.writeImage(ioWrapper, byteOrder);
  return len;
}  // TiffIfdMakernote::doWriteImage

size_t TiffImageEntry::doWriteImage(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  if (!pValue())
    throw Error(ErrorCode::kerImageWriteFailed);  // #1296

  size_t len = pValue()->sizeDataArea();
  if (len > 0) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "TiffImageEntry, Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0')
              << std::hex << tag() << std::dec << ": Writing data area, size = " << len;
#endif
    DataBuf buf = pValue()->dataArea();
    ioWrapper.write(buf.c_data(), buf.size());
    size_t align = len & 1;  // Align image data to word boundary
    if (align)
      ioWrapper.putb(0x0);
    len += align;
  } else {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "TiffImageEntry, Directory " << groupName(group()) << ", entry 0x" << std::setw(4) << std::setfill('0')
              << std::hex << tag() << std::dec << ": Writing " << strips_.size() << " strips";
#endif
    len = 0;
    for (auto&& [f, s] : strips_) {
      ioWrapper.write(f, s);
      len += s;
      size_t align = s & 1;  // Align strip data to word boundary
      if (align)
        ioWrapper.putb(0x0);
      len += align;
    }
  }
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << ", len = " << len << " bytes\n";
#endif
  return len;
}  // TiffImageEntry::doWriteImage

size_t TiffComponent::size() const {
  return doSize();
}

size_t TiffDirectory::doSize() const {
  size_t compCount = count();
  // Size of the directory, without values and additional data
  size_t len = 2 + (12 * compCount) + (hasNext_ ? 4 : 0);
  // Size of IFD values and data
  for (auto&& component : components_) {
    if (size_t sv = component->size(); sv > 4) {
      sv += sv & 1;  // Align value to word boundary
      len += sv;
    }
    auto sd = component->sizeData();
    sd += sd & 1;  // Align data to word boundary
    len += sd;
  }
  // Size of next-IFD, if any
  size_t sizeNext = 0;
  if (pNext_) {
    sizeNext = pNext_->size();
    len += sizeNext;
  }
  // Reset size of IFD if it has no entries and no or empty next IFD.
  if (compCount == 0 && sizeNext == 0)
    len = 0;
  return len;
}

size_t TiffEntryBase::doSize() const {
  return size_;
}

size_t TiffImageEntry::doSize() const {
  return strips_.size() * 4;
}

size_t TiffSubIfd::doSize() const {
  return ifds_.size() * 4;
}

size_t TiffMnEntry::doSize() const {
  if (!mn_) {
    return TiffEntryBase::doSize();
  }
  return mn_->size();
}

size_t TiffIfdMakernote::doSize() const {
  return sizeHeader() + ifd_.size();
}

size_t TiffBinaryArray::doSize() const {
  if (!cfg() || !decoded())
    return TiffEntryBase::doSize();

  if (elements_.empty())
    return 0;

  // Remaining assumptions:
  // - array elements don't "overlap"
  // - no duplicate tags in the array
  size_t idx = 0;
  size_t sz = cfg()->tagStep();
  for (auto&& element : elements_) {
    if (element->tag() > idx) {
      idx = element->tag();
      sz = element->size();
    }
  }
  idx = idx * cfg()->tagStep() + sz;

  if (cfg()->hasFillers_ && def()) {
    const ArrayDef* lastDef = def() + defSize() - 1;
    auto lastTag = static_cast<uint16_t>(lastDef->idx_ / cfg()->tagStep());
    idx = std::max<size_t>(idx, lastDef->idx_ + lastDef->size(lastTag, cfg()->group_));
  }
  return idx;

}  // TiffBinaryArray::doSize

size_t TiffBinaryElement::doSize() const {
  if (!pValue())
    return 0;
  return pValue()->size();
}

size_t TiffComponent::sizeData() const {
  return doSizeData();
}

size_t TiffDirectory::doSizeData() const {
  return 0;
}

size_t TiffEntryBase::doSizeData() const {
  return 0;
}

size_t TiffImageEntry::doSizeData() const {
  size_t len = 0;
  // For makernotes, TIFF image data is written to the data area
  if (group() > IfdId::mnId) {  // Todo: Fix this hack!!
    len = sizeImage();
  }
  return len;
}

size_t TiffDataEntry::doSizeData() const {
  if (!pValue())
    return 0;
  return pValue()->sizeDataArea();
}

size_t TiffSubIfd::doSizeData() const {
  size_t len = 0;
  for (auto&& ifd : ifds_) {
    len += ifd->size();
  }
  return len;
}

size_t TiffIfdMakernote::doSizeData() const {
  return 0;
}

size_t TiffComponent::sizeImage() const {
  return doSizeImage();
}

size_t TiffDirectory::doSizeImage() const {
  size_t len = 0;
  for (auto&& component : components_) {
    len += component->sizeImage();
  }
  if (pNext_) {
    len += pNext_->sizeImage();
  }
  return len;
}

size_t TiffSubIfd::doSizeImage() const {
  size_t len = 0;
  for (auto&& ifd : ifds_) {
    len += ifd->sizeImage();
  }
  return len;
}  // TiffSubIfd::doSizeImage

size_t TiffIfdMakernote::doSizeImage() const {
  return ifd_.sizeImage();
}  // TiffIfdMakernote::doSizeImage

size_t TiffEntryBase::doSizeImage() const {
  return 0;
}  // TiffEntryBase::doSizeImage

size_t TiffImageEntry::doSizeImage() const {
  if (!pValue())
    return 0;
  auto len = pValue()->sizeDataArea();
  if (len == 0) {
    for (const auto& [_, off] : strips_) {
      len += off;
    }
  }
  return len;
}  // TiffImageEntry::doSizeImage

static const TagInfo* findTagInfo(uint16_t tag, IfdId group) {
  const TagInfo* result = nullptr;
  const TagInfo* tags = [=] {
    if (group == IfdId::gpsId)
      return Internal::gpsTagList();
    return group == IfdId::exifId ? Internal::exifTagList() : nullptr;
  }();
  if (!tags)
    return result;

  for (size_t idx = 0; !result && tags[idx].tag_ != 0xffff; ++idx) {
    if (tags[idx].tag_ == tag) {
      result = tags + idx;
    }
  }
  return result;
}

// *************************************************************************
// free functions
TypeId toTypeId(TiffType tiffType, uint16_t tag, IfdId group) {
  auto ti = static_cast<TypeId>(tiffType);
  // On the fly type conversion for Exif.Photo.UserComment, Exif.GPSProcessingMethod, GPSAreaInformation
  if (const TagInfo* pTag = ti == undefined ? findTagInfo(tag, group) : nullptr) {
    if (pTag->typeId_ == comment) {
      ti = comment;
    }
  }
  // http://dev.exiv2.org/boards/3/topics/1337 change unsignedByte to signedByte
  // Exif.NikonAFT.AFFineTuneAdj || Exif.Pentax.Temperature
  if (ti == Exiv2::unsignedByte &&
      ((tag == 0x0002 && group == IfdId::nikonAFTId) || (tag == 0x0047 && group == IfdId::pentaxId))) {
    ti = Exiv2::signedByte;
  }
  return ti;
}

TiffType toTiffType(TypeId typeId) {
  if (static_cast<uint32_t>(typeId) > 0xffff) {
#ifndef SUPPRESS_WARNINGS
    EXV_ERROR << "'" << TypeInfo::typeName(typeId) << "' is not a valid Exif (TIFF) type; using type '"
              << TypeInfo::typeName(undefined) << "'.\n";
#endif
    return undefined;
  }
  return static_cast<uint16_t>(typeId);
}

bool cmpTagLt(const TiffComponent* lhs, const TiffComponent* rhs) {
  if (lhs->tag() != rhs->tag())
    return lhs->tag() < rhs->tag();
  return lhs->idx() < rhs->idx();
}

bool cmpGroupLt(const TiffComponent* lhs, const TiffComponent* rhs) {
  return lhs->group() < rhs->group();
}

TiffComponent::UniquePtr newTiffEntry(uint16_t tag, IfdId group) {
  return std::make_unique<TiffEntry>(tag, group);
}

TiffComponent::UniquePtr newTiffMnEntry(uint16_t tag, IfdId group) {
  return std::make_unique<TiffMnEntry>(tag, group, IfdId::mnId);
}

TiffComponent::UniquePtr newTiffBinaryElement(uint16_t tag, IfdId group) {
  return std::make_unique<TiffBinaryElement>(tag, group);
}

}  // namespace Exiv2::Internal

// *****************************************************************************
// local definitions
namespace {
size_t fillGap(Exiv2::Internal::IoWrapper& ioWrapper, size_t curr, size_t tobe) {
  if (curr < tobe) {
    Exiv2::DataBuf buf(tobe - curr);
    ioWrapper.write(buf.c_data(), buf.size());
    return tobe - curr;
  }
  return 0;
}
}  // namespace
