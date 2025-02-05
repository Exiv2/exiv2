// SPDX-License-Identifier: GPL-2.0-or-later
/*
  File:      makernote.cpp
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
 */
// included header files
#include "config.h"

#include "makernote_int.hpp"
#include "safe_op.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "tiffvisitor_int.hpp"
#include "utils.hpp"

// + standard includes
#include <array>
#include <iostream>

#ifdef EXV_ENABLE_FILESYSTEM
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#endif

#if !defined(_WIN32)
#include <pwd.h>
#include <unistd.h>
#else
#include <shlobj.h>
#ifndef CSIDL_PROFILE
#define CSIDL_PROFILE 40
#endif
#include <process.h>
#endif

#ifdef EXV_ENABLE_INIH
#include <INIReader.h>
#endif

// *****************************************************************************
namespace {
// Todo: Can be generalized further - get any tag as a string/long/...
//! Get the Value for a tag within a particular group
const Exiv2::Value* getExifValue(Exiv2::Internal::TiffComponent* pRoot, const uint16_t& tag, const Exiv2::IfdId& group);
//! Get the model name from tag Exif.Image.Model
std::string getExifModel(Exiv2::Internal::TiffComponent* pRoot);

//! Nikon en/decryption function
void ncrypt(Exiv2::byte* pData, uint32_t size, uint32_t count, uint32_t serial);
}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
// Function first looks for a config file in current working directory
// on Win the file should be named "exiv2.ini"
// on Lin the file should be named ".exiv2"
// If not found in cwd, we return the default path
// which is the user profile path on win and the home dir on linux
std::string getExiv2ConfigPath() {
#ifdef EXV_ENABLE_FILESYSTEM
#ifdef _WIN32
  std::string inifile("exiv2.ini");
#else
  std::string inifile(".exiv2");
#endif
  auto currentPath = fs::current_path();
  auto iniPath = currentPath / inifile;
  if (fs::exists(iniPath)) {
    return iniPath.string();
  }

#ifdef _WIN32
  char buffer[1024];
  if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, buffer))) {
    currentPath = buffer;
  }
#else
  auto pw = getpwuid(getuid());
  currentPath = std::string(pw ? pw->pw_dir : "");
#endif
  return (currentPath / inifile).string();
#else
  return "";
#endif
}

std::string readExiv2Config([[maybe_unused]] const std::string& section, [[maybe_unused]] const std::string& value,
                            const std::string& def) {
  std::string result = def;

#if defined(EXV_ENABLE_INIH) && defined(EXV_ENABLE_FILESYSTEM)
  INIReader reader(Exiv2::Internal::getExiv2ConfigPath());
  if (reader.ParseError() == 0) {
    result = reader.Get(section, value, def);
  }
#endif

  return result;
}

const TiffMnRegistry TiffMnCreator::registry_[] = {
    {"Canon", IfdId::canonId, newIfdMn, newIfdMn2},
    {"FOVEON", IfdId::sigmaId, newSigmaMn, newSigmaMn2},
    {"FUJI", IfdId::fujiId, newFujiMn, newFujiMn2},
    {"KONICA MINOLTA", IfdId::minoltaId, newIfdMn, newIfdMn2},
    {"Minolta", IfdId::minoltaId, newIfdMn, newIfdMn2},
    {"NIKON", IfdId::ifdIdNotSet, newNikonMn, nullptr},      // mnGroup_ is not used
    {"OLYMPUS", IfdId::ifdIdNotSet, newOlympusMn, nullptr},  // mnGroup_ is not used
    {"OM Digital", IfdId::olympus2Id, newOMSystemMn, newOMSystemMn2},
    {"Panasonic", IfdId::panasonicId, newPanasonicMn, newPanasonicMn2},
    {"PENTAX", IfdId::ifdIdNotSet, newPentaxMn, nullptr},  // mnGroup_ is not used
    {"RICOH", IfdId::ifdIdNotSet, newPentaxMn, nullptr},   // mnGroup_ is not used
    {"SAMSUNG", IfdId::samsung2Id, newSamsungMn, newSamsungMn2},
    {"SIGMA", IfdId::sigmaId, newSigmaMn, newSigmaMn2},
    {"SONY", IfdId::ifdIdNotSet, newSonyMn, nullptr},    // mnGroup_ is not used
    {"CASIO", IfdId::ifdIdNotSet, newCasioMn, nullptr},  // mnGroup_ is not used
    // Entries below are only used for lookup by group
    {"-", IfdId::nikon1Id, nullptr, newIfdMn2},
    {"-", IfdId::nikon2Id, nullptr, newNikon2Mn2},
    {"-", IfdId::nikon3Id, nullptr, newNikon3Mn2},
    {"-", IfdId::sony1Id, nullptr, newSony1Mn2},
    {"-", IfdId::sony2Id, nullptr, newSony2Mn2},
    {"-", IfdId::olympusId, nullptr, newOlympusMn2},
    {"-", IfdId::olympus2Id, nullptr, newOlympus2Mn2},
    {"-", IfdId::pentaxId, nullptr, newPentaxMn2},
    {"-", IfdId::pentaxDngId, nullptr, newPentaxDngMn2},
    {"-", IfdId::casioId, nullptr, newIfdMn2},
    {"-", IfdId::casio2Id, nullptr, newCasio2Mn2},
};

bool TiffMnRegistry::operator==(const std::string& key) const {
  std::string make(make_);
  if (!key.empty() && key.front() == '-')
    return false;
  return make == key.substr(0, make.length());
}

bool TiffMnRegistry::operator==(IfdId key) const {
  return mnGroup_ == key;
}

std::unique_ptr<TiffComponent> TiffMnCreator::create(uint16_t tag, IfdId group, const std::string& make,
                                                     const byte* pData, size_t size, ByteOrder byteOrder) {
  if (auto tmr = Exiv2::find(registry_, make))
    return tmr->newMnFct_(tag, group, tmr->mnGroup_, pData, size, byteOrder);
  return nullptr;
}  // TiffMnCreator::create

std::unique_ptr<TiffComponent> TiffMnCreator::create(uint16_t tag, IfdId group, IfdId mnGroup) {
  if (auto tmr = Exiv2::find(registry_, mnGroup)) {
    if (tmr->newMnFct2_) {
      return tmr->newMnFct2_(tag, group, mnGroup);
    }
    std::cout << "mnGroup = " << mnGroup << "\n";
  }
  return nullptr;
}  // TiffMnCreator::create

void MnHeader::setByteOrder(ByteOrder /*byteOrder*/) {
}

size_t MnHeader::ifdOffset() const {
  return 0;
}

ByteOrder MnHeader::byteOrder() const {
  return invalidByteOrder;
}

size_t MnHeader::baseOffset(size_t /*mnOffset*/) const {
  return 0;
}

const byte OlympusMnHeader::signature_[] = {'O', 'L', 'Y', 'M', 'P', 0x00, 0x01, 0x00};

size_t OlympusMnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

OlympusMnHeader::OlympusMnHeader() {
  read(signature_, sizeOfSignature(), invalidByteOrder);
}

size_t OlympusMnHeader::size() const {
  return header_.size();
}

size_t OlympusMnHeader::ifdOffset() const {
  return sizeOfSignature();
}

bool OlympusMnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  header_.alloc(sizeOfSignature());
  std::copy_n(pData, header_.size(), header_.data());
  return header_.size() >= sizeOfSignature() && 0 == header_.cmpBytes(0, signature_, 6);
}

size_t OlympusMnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}  // OlympusMnHeader::write

const byte Olympus2MnHeader::signature_[] = {'O', 'L', 'Y', 'M', 'P', 'U', 'S', 0x00, 'I', 'I', 0x03, 0x00};

size_t Olympus2MnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

Olympus2MnHeader::Olympus2MnHeader() {
  read(signature_, sizeOfSignature(), invalidByteOrder);
}

size_t Olympus2MnHeader::size() const {
  return header_.size();
}

size_t Olympus2MnHeader::ifdOffset() const {
  return sizeOfSignature();
}

size_t Olympus2MnHeader::baseOffset(size_t mnOffset) const {
  return mnOffset;
}

bool Olympus2MnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  header_.alloc(sizeOfSignature());
  std::copy_n(pData, header_.size(), header_.data());
  return header_.size() >= sizeOfSignature() && 0 == header_.cmpBytes(0, signature_, 10);
}

size_t Olympus2MnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}  // Olympus2MnHeader::write

const byte OMSystemMnHeader::signature_[] = {'O', 'M',  ' ',  'S',  'Y', 'S', 'T',  'E',
                                             'M', 0x00, 0x00, 0x00, 'I', 'I', 0x04, 0x00};

size_t OMSystemMnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

OMSystemMnHeader::OMSystemMnHeader() {
  read(signature_, sizeOfSignature(), invalidByteOrder);
}

size_t OMSystemMnHeader::size() const {
  return header_.size();
}

size_t OMSystemMnHeader::ifdOffset() const {
  return sizeOfSignature();
}

size_t OMSystemMnHeader::baseOffset(size_t mnOffset) const {
  return mnOffset;
}

bool OMSystemMnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  header_.alloc(sizeOfSignature());
  std::copy_n(pData, header_.size(), header_.data());
  return header_.size() >= sizeOfSignature() && 0 == header_.cmpBytes(0, signature_, sizeOfSignature() - 2);
}

size_t OMSystemMnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}  // OMSystemMnHeader::write

const byte FujiMnHeader::signature_[] = {'F', 'U', 'J', 'I', 'F', 'I', 'L', 'M', 0x0c, 0x00, 0x00, 0x00};
const ByteOrder FujiMnHeader::byteOrder_ = littleEndian;

size_t FujiMnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

FujiMnHeader::FujiMnHeader() {
  read(signature_, sizeOfSignature(), byteOrder_);
}

size_t FujiMnHeader::size() const {
  return header_.size();
}

size_t FujiMnHeader::ifdOffset() const {
  return start_;
}

ByteOrder FujiMnHeader::byteOrder() const {
  return byteOrder_;
}

size_t FujiMnHeader::baseOffset(size_t mnOffset) const {
  return mnOffset;
}

bool FujiMnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  header_.alloc(sizeOfSignature());
  std::copy_n(pData, header_.size(), header_.data());
  // Read offset to the IFD relative to the start of the makernote
  // from the header. Note that we ignore the byteOrder argument
  start_ = header_.read_uint32(8, byteOrder_);
  return header_.size() >= sizeOfSignature() && 0 == header_.cmpBytes(0, signature_, 8);
}

size_t FujiMnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}  // FujiMnHeader::write

const byte Nikon2MnHeader::signature_[] = {'N', 'i', 'k', 'o', 'n', '\0', 0x01, 0x00};

size_t Nikon2MnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

Nikon2MnHeader::Nikon2MnHeader() {
  read(signature_, sizeOfSignature(), invalidByteOrder);
}

size_t Nikon2MnHeader::size() const {
  return sizeOfSignature();
}

size_t Nikon2MnHeader::ifdOffset() const {
  return start_;
}

bool Nikon2MnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  if (0 != memcmp(pData, signature_, 6))
    return false;
  buf_.alloc(sizeOfSignature());
  std::copy_n(pData, buf_.size(), buf_.data());
  start_ = sizeOfSignature();
  return true;
}  // Nikon2MnHeader::read

size_t Nikon2MnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}  // Nikon2MnHeader::write

const byte Nikon3MnHeader::signature_[] = {'N',  'i',  'k',  'o',  'n',  '\0', 0x02, 0x10, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

size_t Nikon3MnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

Nikon3MnHeader::Nikon3MnHeader() : start_(sizeOfSignature()) {
  buf_.alloc(sizeOfSignature());
  std::copy_n(signature_, buf_.size(), buf_.data());
}

size_t Nikon3MnHeader::size() const {
  return sizeOfSignature();
}

size_t Nikon3MnHeader::ifdOffset() const {
  return start_;
}

ByteOrder Nikon3MnHeader::byteOrder() const {
  return byteOrder_;
}

size_t Nikon3MnHeader::baseOffset(size_t mnOffset) const {
  return Safe::add<size_t>(mnOffset, 10);
}

bool Nikon3MnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  if (0 != memcmp(pData, signature_, 6))
    return false;
  buf_.alloc(sizeOfSignature());
  std::copy_n(pData, buf_.size(), buf_.data());
  TiffHeader th;
  if (!th.read(buf_.data(10), 8))
    return false;
  byteOrder_ = th.byteOrder();
  start_ = 10 + th.offset();
  return true;
}  // Nikon3MnHeader::read

size_t Nikon3MnHeader::write(IoWrapper& ioWrapper, ByteOrder byteOrder) const {
  ioWrapper.write(buf_.c_data(), 10);
  /// \todo: This removes any gap between the header and makernote IFD. The gap should be copied too.
  TiffHeader th(byteOrder);
  DataBuf buf = th.write();
  ioWrapper.write(buf.c_data(), buf.size());
  return 10 + buf.size();
}

void Nikon3MnHeader::setByteOrder(ByteOrder byteOrder) {
  byteOrder_ = byteOrder;
}

const byte PanasonicMnHeader::signature_[] = {'P', 'a', 'n', 'a', 's', 'o', 'n', 'i', 'c', 0x00, 0x00, 0x00};

size_t PanasonicMnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

PanasonicMnHeader::PanasonicMnHeader() {
  read(signature_, sizeOfSignature(), invalidByteOrder);
}

size_t PanasonicMnHeader::size() const {
  return sizeOfSignature();
}

size_t PanasonicMnHeader::ifdOffset() const {
  return start_;
}

bool PanasonicMnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  if (0 != memcmp(pData, signature_, 9))
    return false;
  buf_.alloc(sizeOfSignature());
  std::copy_n(pData, buf_.size(), buf_.data());
  start_ = sizeOfSignature();
  return true;
}  // PanasonicMnHeader::read

size_t PanasonicMnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}  // PanasonicMnHeader::write

const byte PentaxDngMnHeader::signature_[] = {'P', 'E', 'N', 'T', 'A', 'X', ' ', 0x00, 'M', 'M'};

size_t PentaxDngMnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

PentaxDngMnHeader::PentaxDngMnHeader() {
  read(signature_, sizeOfSignature(), invalidByteOrder);
}

size_t PentaxDngMnHeader::size() const {
  return header_.size();
}

size_t PentaxDngMnHeader::baseOffset(size_t mnOffset) const {
  return mnOffset;
}

size_t PentaxDngMnHeader::ifdOffset() const {
  return sizeOfSignature();
}

bool PentaxDngMnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  header_.alloc(sizeOfSignature());
  std::copy_n(pData, header_.size(), header_.data());
  return header_.size() >= sizeOfSignature() && 0 == header_.cmpBytes(0, signature_, 7);
}

size_t PentaxDngMnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}  // PentaxDngMnHeader::write

const byte PentaxMnHeader::signature_[] = {'A', 'O', 'C', 0x00, 'M', 'M'};

size_t PentaxMnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

PentaxMnHeader::PentaxMnHeader() {
  read(signature_, sizeOfSignature(), invalidByteOrder);
}

size_t PentaxMnHeader::size() const {
  return header_.size();
}

size_t PentaxMnHeader::ifdOffset() const {
  return sizeOfSignature();
}

bool PentaxMnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  header_.alloc(sizeOfSignature());
  std::copy_n(pData, header_.size(), header_.data());
  return header_.size() >= sizeOfSignature() && 0 == header_.cmpBytes(0, signature_, 3);
}

size_t PentaxMnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}

SamsungMnHeader::SamsungMnHeader() {
  read(nullptr, 0, invalidByteOrder);
}

size_t SamsungMnHeader::size() const {
  return 0;
}

size_t SamsungMnHeader::baseOffset(size_t mnOffset) const {
  return mnOffset;
}

bool SamsungMnHeader::read(const byte* /*pData*/, size_t /*size*/, ByteOrder /*byteOrder*/) {
  return true;
}  // SamsungMnHeader::read

size_t SamsungMnHeader::write(IoWrapper& /*ioWrapper*/, ByteOrder /*byteOrder*/) const {
  return 0;
}  // SamsungMnHeader::write

const byte SigmaMnHeader::signature1_[] = {'S', 'I', 'G', 'M', 'A', '\0', '\0', '\0', 0x01, 0x00};
const byte SigmaMnHeader::signature2_[] = {'F', 'O', 'V', 'E', 'O', 'N', '\0', '\0', 0x01, 0x00};

size_t SigmaMnHeader::sizeOfSignature() {
  return sizeof(signature1_);
}

SigmaMnHeader::SigmaMnHeader() {
  read(signature1_, sizeOfSignature(), invalidByteOrder);
}

size_t SigmaMnHeader::size() const {
  return sizeOfSignature();
}

size_t SigmaMnHeader::ifdOffset() const {
  return start_;
}

bool SigmaMnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  if (0 != memcmp(pData, signature1_, 8) && 0 != memcmp(pData, signature2_, 8))
    return false;
  buf_.alloc(sizeOfSignature());
  std::copy_n(pData, buf_.size(), buf_.data());
  start_ = sizeOfSignature();
  return true;
}  // SigmaMnHeader::read

size_t SigmaMnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature1_, sizeOfSignature());
  return sizeOfSignature();
}  // SigmaMnHeader::write

const byte SonyMnHeader::signature_[] = {'S', 'O', 'N', 'Y', ' ', 'D', 'S', 'C', ' ', '\0', '\0', '\0'};

size_t SonyMnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

SonyMnHeader::SonyMnHeader() {
  read(signature_, sizeOfSignature(), invalidByteOrder);
}

size_t SonyMnHeader::size() const {
  return sizeOfSignature();
}

size_t SonyMnHeader::ifdOffset() const {
  return start_;
}

bool SonyMnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  if (0 != memcmp(pData, signature_, sizeOfSignature()))
    return false;
  buf_.alloc(sizeOfSignature());
  std::copy_n(pData, buf_.size(), buf_.data());
  start_ = sizeOfSignature();
  return true;
}  // SonyMnHeader::read

size_t SonyMnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}  // SonyMnHeader::write

const byte Casio2MnHeader::signature_[] = {'Q', 'V', 'C', '\0', '\0', '\0'};
const ByteOrder Casio2MnHeader::byteOrder_ = bigEndian;

size_t Casio2MnHeader::sizeOfSignature() {
  return sizeof(signature_);
}

Casio2MnHeader::Casio2MnHeader() {
  read(signature_, sizeOfSignature(), invalidByteOrder);
}

size_t Casio2MnHeader::size() const {
  return sizeOfSignature();
}

size_t Casio2MnHeader::ifdOffset() const {
  return start_;
}

ByteOrder Casio2MnHeader::byteOrder() const {
  return byteOrder_;
}

bool Casio2MnHeader::read(const byte* pData, size_t size, ByteOrder /*byteOrder*/) {
  if (!pData || size < sizeOfSignature())
    return false;
  if (0 != memcmp(pData, signature_, sizeOfSignature()))
    return false;
  buf_.alloc(sizeOfSignature());
  std::copy_n(pData, buf_.size(), buf_.data());
  start_ = sizeOfSignature();
  return true;
}  // Casio2MnHeader::read

size_t Casio2MnHeader::write(IoWrapper& ioWrapper, ByteOrder /*byteOrder*/) const {
  ioWrapper.write(signature_, sizeOfSignature());
  return sizeOfSignature();
}  // Casio2MnHeader::write

// *************************************************************************
// free functions

std::unique_ptr<TiffComponent> newIfdMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* /*pData*/, size_t size,
                                        ByteOrder /*byteOrder*/) {
  // Require at least an IFD with 1 entry, but not necessarily a next pointer
  if (size < 14)
    return nullptr;
  return newIfdMn2(tag, group, mnGroup);
}

std::unique_ptr<TiffComponent> newIfdMn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, nullptr);
}

std::unique_ptr<TiffComponent> newOlympusMn(uint16_t tag, IfdId group, IfdId /*mnGroup*/, const byte* pData,
                                            size_t size, ByteOrder /*byteOrder*/) {
  // FIXME: workaround for overwritten OM System header in Olympus files (https://github.com/Exiv2/exiv2/issues/2542)
  if (size >= 14 && std::string(reinterpret_cast<const char*>(pData), 14) == std::string("OM SYSTEM\0\0\0II", 14)) {
    // Require at least the header and an IFD with 1 entry
    if (size < OMSystemMnHeader::sizeOfSignature() + 18)
      return nullptr;
    return newOMSystemMn2(tag, group, IfdId::olympus2Id);
  }
  if (size < 10 || std::string(reinterpret_cast<const char*>(pData), 10) != std::string("OLYMPUS\0II", 10)) {
    // Require at least the header and an IFD with 1 entry
    if (size < OlympusMnHeader::sizeOfSignature() + 18)
      return nullptr;
    return newOlympusMn2(tag, group, IfdId::olympusId);
  }
  // Require at least the header and an IFD with 1 entry
  if (size < Olympus2MnHeader::sizeOfSignature() + 18)
    return nullptr;
  return newOlympus2Mn2(tag, group, IfdId::olympus2Id);
}

std::unique_ptr<TiffComponent> newOlympusMn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<OlympusMnHeader>());
}

std::unique_ptr<TiffComponent> newOlympus2Mn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<Olympus2MnHeader>());
}

std::unique_ptr<TiffComponent> newOMSystemMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* /*pData*/,
                                             size_t size, ByteOrder /*byteOrder*/) {
  // Require at least the header and an IFD with 1 entry
  if (size < OMSystemMnHeader::sizeOfSignature() + 18)
    return nullptr;
  return newOMSystemMn2(tag, group, mnGroup);
}

std::unique_ptr<TiffComponent> newOMSystemMn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<OMSystemMnHeader>());
}

std::unique_ptr<TiffComponent> newFujiMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* /*pData*/, size_t size,
                                         ByteOrder /*byteOrder*/) {
  // Require at least the header and an IFD with 1 entry
  if (size < FujiMnHeader::sizeOfSignature() + 18)
    return nullptr;
  return newFujiMn2(tag, group, mnGroup);
}

std::unique_ptr<TiffComponent> newFujiMn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<FujiMnHeader>());
}

std::unique_ptr<TiffComponent> newNikonMn(uint16_t tag, IfdId group, IfdId /*mnGroup*/, const byte* pData, size_t size,
                                          ByteOrder /*byteOrder*/) {
  // If there is no "Nikon" string it must be Nikon1 format
  if (size < 6 || std::string(reinterpret_cast<const char*>(pData), 6) != std::string("Nikon\0", 6)) {
    // Require at least an IFD with 1 entry
    if (size < 18)
      return nullptr;
    return newIfdMn2(tag, group, IfdId::nikon1Id);
  }
  // If the "Nikon" string is not followed by a TIFF header, we assume
  // Nikon2 format
  TiffHeader tiffHeader;
  if (size < 18 || !tiffHeader.read(pData + 10, size - 10) || tiffHeader.tag() != 0x002a) {
    // Require at least the header and an IFD with 1 entry
    if (size < Nikon2MnHeader::sizeOfSignature() + 18)
      return nullptr;
    return newNikon2Mn2(tag, group, IfdId::nikon2Id);
  }
  // Else we have a Nikon3 makernote
  // Require at least the header and an IFD with 1 entry
  if (size < Nikon3MnHeader::sizeOfSignature() + 18)
    return nullptr;
  return newNikon3Mn2(tag, group, IfdId::nikon3Id);
}

std::unique_ptr<TiffComponent> newNikon2Mn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<Nikon2MnHeader>());
}

std::unique_ptr<TiffComponent> newNikon3Mn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<Nikon3MnHeader>());
}

std::unique_ptr<TiffComponent> newPanasonicMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* /*pData*/,
                                              size_t size, ByteOrder /*byteOrder*/) {
  // Require at least the header and an IFD with 1 entry, but without a next pointer
  if (size < PanasonicMnHeader::sizeOfSignature() + 14)
    return nullptr;
  return newPanasonicMn2(tag, group, mnGroup);
}

std::unique_ptr<TiffComponent> newPanasonicMn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<PanasonicMnHeader>(), false);
}

std::unique_ptr<TiffComponent> newPentaxMn(uint16_t tag, IfdId group, IfdId /*mnGroup*/, const byte* pData, size_t size,
                                           ByteOrder /*byteOrder*/) {
  if (size > 8 && std::string(reinterpret_cast<const char*>(pData), 8) == std::string("PENTAX \0", 8)) {
    // Require at least the header and an IFD with 1 entry
    if (size < PentaxDngMnHeader::sizeOfSignature() + 18)
      return nullptr;
    return newPentaxDngMn2(tag, group, (tag == 0xc634 ? IfdId::pentaxDngId : IfdId::pentaxId));
  }
  if (size > 4 && std::string(reinterpret_cast<const char*>(pData), 4) == std::string("AOC\0", 4)) {
    // Require at least the header and an IFD with 1 entry
    if (size < PentaxMnHeader::sizeOfSignature() + 18)
      return nullptr;
    return newPentaxMn2(tag, group, IfdId::pentaxId);
  }
  return nullptr;
}

std::unique_ptr<TiffComponent> newPentaxMn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<PentaxMnHeader>());
}

std::unique_ptr<TiffComponent> newPentaxDngMn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<PentaxDngMnHeader>());
}

std::unique_ptr<TiffComponent> newSamsungMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* pData, size_t size,
                                            ByteOrder /*byteOrder*/) {
  if (size > 4 && std::string(reinterpret_cast<const char*>(pData), 4) == std::string("AOC\0", 4)) {
    // Samsung branded Pentax camera:
    // Require at least the header and an IFD with 1 entry
    if (size < PentaxMnHeader::sizeOfSignature() + 18)
      return nullptr;
    return newPentaxMn2(tag, group, IfdId::pentaxId);
  }
  // Genuine Samsung camera:
  // Require at least an IFD with 1 entry
  if (size < 18)
    return nullptr;
  return newSamsungMn2(tag, group, mnGroup);
}

std::unique_ptr<TiffComponent> newSamsungMn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<SamsungMnHeader>());
}

std::unique_ptr<TiffComponent> newSigmaMn(uint16_t tag, IfdId group, IfdId mnGroup, const byte* /*pData*/, size_t size,
                                          ByteOrder /*byteOrder*/) {
  // Require at least the header and an IFD with 1 entry
  if (size < SigmaMnHeader::sizeOfSignature() + 18)
    return nullptr;
  return newSigmaMn2(tag, group, mnGroup);
}

std::unique_ptr<TiffComponent> newSigmaMn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<SigmaMnHeader>());
}

std::unique_ptr<TiffComponent> newSonyMn(uint16_t tag, IfdId group, IfdId /*mnGroup*/, const byte* pData, size_t size,
                                         ByteOrder /*byteOrder*/) {
  // If there is no "SONY DSC " string we assume it's a simple IFD Makernote
  if (size < 12 || std::string(reinterpret_cast<const char*>(pData), 12) != std::string("SONY DSC \0\0\0", 12)) {
    // Require at least an IFD with 1 entry
    if (size < 18)
      return nullptr;
    return newSony2Mn2(tag, group, IfdId::sony2Id);
  }
  // Require at least the header and an IFD with 1 entry, but without a next pointer
  if (size < SonyMnHeader::sizeOfSignature() + 14)
    return nullptr;
  return newSony1Mn2(tag, group, IfdId::sony1Id);
}

std::unique_ptr<TiffComponent> newSony1Mn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<SonyMnHeader>(), false);
}

std::unique_ptr<TiffComponent> newSony2Mn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, nullptr, true);
}

std::unique_ptr<TiffComponent> newCasioMn(uint16_t tag, IfdId group, IfdId /* mnGroup*/, const byte* pData, size_t size,
                                          ByteOrder /* byteOrder */) {
  if (size > 6 && std::string(reinterpret_cast<const char*>(pData), 6) == std::string("QVC\0\0\0", 6))
    return newCasio2Mn2(tag, group, IfdId::casio2Id);
  // Require at least an IFD with 1 entry, but not necessarily a next pointer
  if (size < 14)
    return nullptr;
  return newIfdMn2(tag, group, IfdId::casioId);
}

std::unique_ptr<TiffComponent> newCasio2Mn2(uint16_t tag, IfdId group, IfdId mnGroup) {
  return std::make_unique<TiffIfdMakernote>(tag, group, mnGroup, std::make_unique<Casio2MnHeader>());
}

//! Structure for an index into the array set of complex binary arrays.
struct NikonArrayIdx {
  //! Key for comparisons
  struct Key {
    uint16_t tag_;     //!< Tag number
    const char* ver_;  //!< Version string
    size_t size_;      //!< Size of the data (not the version string)
  };
  //! Comparison operator for a key
  bool operator==(const Key& key) const {
    return key.tag_ == tag_ && 0 == strncmp(key.ver_, ver_, strlen(ver_)) && (size_ == 0 || key.size_ == size_);
  }

  uint16_t tag_;     //!< Tag number of the binary array
  const char* ver_;  //!< Version string
  uint32_t size_;    //!< Size of the data
  int idx_;          //!< Index into the array set
  uint32_t start_;   //!< Start of the encrypted data
};

#define NA std::numeric_limits<uint32_t>::max()

//! Nikon binary array version lookup table
constexpr NikonArrayIdx nikonArrayIdx[] = {
    // NikonSi
    {0x0091, "0208", 0, 0, 4},     // D80
    {0x0091, "0209", 0, 1, 4},     // D40
    {0x0091, "0210", 5291, 2, 4},  // D300
    {0x0091, "0210", 5303, 3, 4},  // D300, firmware version 1.10
    {0x0091, "02", 0, 4, 4},       // Other v2.* (encrypted)
    {0x0091, "01", 0, 5, NA},      // Other v1.* (not encrypted)
    // NikonCb
    {0x0097, "0100", 0, 0, NA},
    {0x0097, "0102", 0, 1, NA},
    {0x0097, "0103", 0, 4, NA},
    {0x0097, "0205", 0, 2, 4},
    {0x0097, "0209", 0, 5, 284},
    {0x0097, "0212", 0, 5, 284},
    {0x0097, "0214", 0, 5, 284},
    {0x0097, "02", 0, 3, 284},
    // NikonLd
    {0x0098, "0100", 0, 0, NA},
    {0x0098, "0101", 0, 1, NA},
    {0x0098, "0201", 0, 1, 4},
    {0x0098, "0202", 0, 1, 4},
    {0x0098, "0203", 0, 1, 4},
    {0x0098, "0204", 0, 2, 4},
    {0x0098, "0800", 0, 3, 4},  // for e.g. Z6/7
    {0x0098, "0801", 0, 3, 4},  // for e.g. Z6/7
    {0x0098, "0802", 0, 3, 4},  // for e.g. Z9
    // NikonFl
    {0x00a8, "0100", 0, 0, NA},
    {0x00a8, "0101", 0, 0, NA},
    {0x00a8, "0102", 0, 1, NA},
    {0x00a8, "0103", 0, 2, NA},
    {0x00a8, "0104", 0, 2, NA},
    {0x00a8, "0105", 0, 2, NA},
    {0x00a8, "0106", 0, 3, NA},
    {0x00a8, "0107", 0, 4, NA},
    {0x00a8, "0108", 0, 4, NA},
    // NikonAf
    {0x00b7, "0100", 30, 0, NA},  // These sizes have been found in tiff headers of MN
    {0x00b7, "0101", 84, 1, NA},  // tag 0xb7 in sample image metadata for each version
};

int nikonSelector(uint16_t tag, const byte* pData, size_t size, TiffComponent* /*pRoot*/) {
  if (size < 4)
    return -1;

  auto ix = NikonArrayIdx::Key{tag, reinterpret_cast<const char*>(pData), size};
  if (auto it = Exiv2::find(nikonArrayIdx, ix))
    return it->idx_;
  return -1;
}

DataBuf nikonCrypt(uint16_t tag, const byte* pData, size_t size, TiffComponent* pRoot) {
  DataBuf buf;

  if (size < 4)
    return buf;
  auto nci = Exiv2::find(nikonArrayIdx, NikonArrayIdx::Key{tag, reinterpret_cast<const char*>(pData), size});
  if (!nci || nci->start_ == NA || size <= nci->start_)
    return buf;

  // Find Exif.Nikon3.ShutterCount
  TiffFinder finder(0x00a7, IfdId::nikon3Id);
  pRoot->accept(finder);
  auto te = dynamic_cast<TiffEntryBase*>(finder.result());
  if (!te || !te->pValue() || te->pValue()->count() == 0)
    return buf;
  auto count = te->pValue()->toUint32();

  // Find Exif.Nikon3.SerialNumber
  finder.init(0x001d, IfdId::nikon3Id);
  pRoot->accept(finder);
  te = dynamic_cast<TiffEntryBase*>(finder.result());
  if (!te || !te->pValue() || te->pValue()->count() == 0)
    return buf;
  bool ok(false);
  auto serial = stringTo<uint32_t>(te->pValue()->toString(), ok);
  if (!ok) {
    std::string model = getExifModel(pRoot);
    if (model.empty())
      return buf;
    if (Internal::contains(model, "D50")) {
      serial = 0x22;
    } else {
      serial = 0x60;
    }
  }
  buf.alloc(size);
  std::copy_n(pData, buf.size(), buf.data());
  ncrypt(buf.data(nci->start_), static_cast<uint32_t>(buf.size()) - nci->start_, count, serial);
  return buf;
}

int sonyCsSelector(uint16_t /*tag*/, const byte* /*pData*/, size_t /*size*/, TiffComponent* pRoot) {
  std::string model = getExifModel(pRoot);
  if (model.empty())
    return -1;
  int idx = 0;
  if (Internal::contains(model, "DSLR-A330") || Internal::contains(model, "DSLR-A380")) {
    idx = 1;
  }
  return idx;
}
int sony2010eSelector(uint16_t /*tag*/, const byte* /*pData*/, size_t /*size*/, TiffComponent* pRoot) {
  static constexpr const char* models[] = {
      "SLT-A58",   "SLT-A99",  "ILCE-3000", "ILCE-3500", "NEX-3N",    "NEX-5R",   "NEX-5T",
      "NEX-6",     "VG30E",    "VG900",     "DSC-RX100", "DSC-RX1",   "DSC-RX1R", "DSC-HX300",
      "DSC-HX50V", "DSC-TX30", "DSC-WX60",  "DSC-WX200", "DSC-WX300",
  };
  return Exiv2::find(models, getExifModel(pRoot)) ? 0 : -1;
}

int sony2FpSelector(uint16_t /*tag*/, const byte* /*pData*/, size_t /*size*/, TiffComponent* pRoot) {
  // Not valid for models beginning
  std::string model = getExifModel(pRoot);
  const std::array strs{"SLT-", "HV", "ILCA-"};
  return std::any_of(strs.begin(), strs.end(), [&model](auto& m) { return startsWith(model, m); }) ? -1 : 0;
}

int sonyMisc2bSelector(uint16_t /*tag*/, const byte* /*pData*/, size_t /*size*/, TiffComponent* pRoot) {
  // From Exiftool: https://github.com/exiftool/exiftool/blob/master/lib/Image/ExifTool/Sony.pm
  // >  First byte must be 9 or 12 or 13 or 15 or 16 and 4th byte must be 2 (deciphered)

  // Get the value from the image format that is being used
  auto value = getExifValue(pRoot, 0x9404, Exiv2::IfdId::sony1Id);
  if (!value) {
    value = getExifValue(pRoot, 0x9404, Exiv2::IfdId::sony2Id);
    if (!value)
      return -1;
  }

  if (value->count() < 4)
    return -1;

  switch (value->toInt64(0)) {                 // Using encrypted values
    case 231:                                  // 231 == 9
    case 234:                                  // 234 == 12
    case 205:                                  // 205 == 13
    case 138:                                  // 138 == 15
    case 112:                                  // 112 == 16
      return value->toInt64(3) == 8 ? 0 : -1;  // 8   == 2
    default:
      break;
  }
  return -1;
}
int sonyMisc3cSelector(uint16_t /*tag*/, const byte* /*pData*/, size_t /*size*/, TiffComponent* pRoot) {
  // For condition, see Exiftool (Tag 9400c):
  // https://github.com/exiftool/exiftool/blob/7368629751669ba170511419b3d1e05bf0076d0e/lib/Image/ExifTool/Sony.pm#L1681

  // Get the value from the image format that is being used
  auto value = getExifValue(pRoot, 0x9400, Exiv2::IfdId::sony1Id);
  if (!value) {
    value = getExifValue(pRoot, 0x9400, Exiv2::IfdId::sony2Id);
    if (!value)
      return -1;
  }

  if (value->count() < 1)
    return -1;

  switch (value->toInt64()) {
    case 35:
    case 36:
    case 38:
    case 40:
    case 49:
    case 50:
      return 0;
    default:
      break;
  }
  return -1;
}
}  // namespace Exiv2::Internal

// *****************************************************************************
// local definitions
namespace {
const Exiv2::Value* getExifValue(Exiv2::Internal::TiffComponent* pRoot, const uint16_t& tag,
                                 const Exiv2::IfdId& group) {
  Exiv2::Internal::TiffFinder finder(tag, group);
  if (!pRoot)
    return nullptr;
  pRoot->accept(finder);
  auto te = dynamic_cast<Exiv2::Internal::TiffEntryBase*>(finder.result());
  return (!te || !te->pValue()) ? nullptr : te->pValue();
}

std::string getExifModel(Exiv2::Internal::TiffComponent* pRoot) {
  // Lookup the Exif.Image.Model tag
  const auto value = getExifValue(pRoot, 0x0110, Exiv2::IfdId::ifd0Id);
  return (!value || value->count() == 0) ? std::string() : value->toString();
}

void ncrypt(Exiv2::byte* pData, uint32_t size, uint32_t count, uint32_t serial) {
  static const Exiv2::byte xlat[2][256] = {
      {0xc1, 0xbf, 0x6d, 0x0d, 0x59, 0xc5, 0x13, 0x9d, 0x83, 0x61, 0x6b, 0x4f, 0xc7, 0x7f, 0x3d, 0x3d, 0x53, 0x59, 0xe3,
       0xc7, 0xe9, 0x2f, 0x95, 0xa7, 0x95, 0x1f, 0xdf, 0x7f, 0x2b, 0x29, 0xc7, 0x0d, 0xdf, 0x07, 0xef, 0x71, 0x89, 0x3d,
       0x13, 0x3d, 0x3b, 0x13, 0xfb, 0x0d, 0x89, 0xc1, 0x65, 0x1f, 0xb3, 0x0d, 0x6b, 0x29, 0xe3, 0xfb, 0xef, 0xa3, 0x6b,
       0x47, 0x7f, 0x95, 0x35, 0xa7, 0x47, 0x4f, 0xc7, 0xf1, 0x59, 0x95, 0x35, 0x11, 0x29, 0x61, 0xf1, 0x3d, 0xb3, 0x2b,
       0x0d, 0x43, 0x89, 0xc1, 0x9d, 0x9d, 0x89, 0x65, 0xf1, 0xe9, 0xdf, 0xbf, 0x3d, 0x7f, 0x53, 0x97, 0xe5, 0xe9, 0x95,
       0x17, 0x1d, 0x3d, 0x8b, 0xfb, 0xc7, 0xe3, 0x67, 0xa7, 0x07, 0xf1, 0x71, 0xa7, 0x53, 0xb5, 0x29, 0x89, 0xe5, 0x2b,
       0xa7, 0x17, 0x29, 0xe9, 0x4f, 0xc5, 0x65, 0x6d, 0x6b, 0xef, 0x0d, 0x89, 0x49, 0x2f, 0xb3, 0x43, 0x53, 0x65, 0x1d,
       0x49, 0xa3, 0x13, 0x89, 0x59, 0xef, 0x6b, 0xef, 0x65, 0x1d, 0x0b, 0x59, 0x13, 0xe3, 0x4f, 0x9d, 0xb3, 0x29, 0x43,
       0x2b, 0x07, 0x1d, 0x95, 0x59, 0x59, 0x47, 0xfb, 0xe5, 0xe9, 0x61, 0x47, 0x2f, 0x35, 0x7f, 0x17, 0x7f, 0xef, 0x7f,
       0x95, 0x95, 0x71, 0xd3, 0xa3, 0x0b, 0x71, 0xa3, 0xad, 0x0b, 0x3b, 0xb5, 0xfb, 0xa3, 0xbf, 0x4f, 0x83, 0x1d, 0xad,
       0xe9, 0x2f, 0x71, 0x65, 0xa3, 0xe5, 0x07, 0x35, 0x3d, 0x0d, 0xb5, 0xe9, 0xe5, 0x47, 0x3b, 0x9d, 0xef, 0x35, 0xa3,
       0xbf, 0xb3, 0xdf, 0x53, 0xd3, 0x97, 0x53, 0x49, 0x71, 0x07, 0x35, 0x61, 0x71, 0x2f, 0x43, 0x2f, 0x11, 0xdf, 0x17,
       0x97, 0xfb, 0x95, 0x3b, 0x7f, 0x6b, 0xd3, 0x25, 0xbf, 0xad, 0xc7, 0xc5, 0xc5, 0xb5, 0x8b, 0xef, 0x2f, 0xd3, 0x07,
       0x6b, 0x25, 0x49, 0x95, 0x25, 0x49, 0x6d, 0x71, 0xc7},
      {0xa7, 0xbc, 0xc9, 0xad, 0x91, 0xdf, 0x85, 0xe5, 0xd4, 0x78, 0xd5, 0x17, 0x46, 0x7c, 0x29, 0x4c, 0x4d, 0x03, 0xe9,
       0x25, 0x68, 0x11, 0x86, 0xb3, 0xbd, 0xf7, 0x6f, 0x61, 0x22, 0xa2, 0x26, 0x34, 0x2a, 0xbe, 0x1e, 0x46, 0x14, 0x68,
       0x9d, 0x44, 0x18, 0xc2, 0x40, 0xf4, 0x7e, 0x5f, 0x1b, 0xad, 0x0b, 0x94, 0xb6, 0x67, 0xb4, 0x0b, 0xe1, 0xea, 0x95,
       0x9c, 0x66, 0xdc, 0xe7, 0x5d, 0x6c, 0x05, 0xda, 0xd5, 0xdf, 0x7a, 0xef, 0xf6, 0xdb, 0x1f, 0x82, 0x4c, 0xc0, 0x68,
       0x47, 0xa1, 0xbd, 0xee, 0x39, 0x50, 0x56, 0x4a, 0xdd, 0xdf, 0xa5, 0xf8, 0xc6, 0xda, 0xca, 0x90, 0xca, 0x01, 0x42,
       0x9d, 0x8b, 0x0c, 0x73, 0x43, 0x75, 0x05, 0x94, 0xde, 0x24, 0xb3, 0x80, 0x34, 0xe5, 0x2c, 0xdc, 0x9b, 0x3f, 0xca,
       0x33, 0x45, 0xd0, 0xdb, 0x5f, 0xf5, 0x52, 0xc3, 0x21, 0xda, 0xe2, 0x22, 0x72, 0x6b, 0x3e, 0xd0, 0x5b, 0xa8, 0x87,
       0x8c, 0x06, 0x5d, 0x0f, 0xdd, 0x09, 0x19, 0x93, 0xd0, 0xb9, 0xfc, 0x8b, 0x0f, 0x84, 0x60, 0x33, 0x1c, 0x9b, 0x45,
       0xf1, 0xf0, 0xa3, 0x94, 0x3a, 0x12, 0x77, 0x33, 0x4d, 0x44, 0x78, 0x28, 0x3c, 0x9e, 0xfd, 0x65, 0x57, 0x16, 0x94,
       0x6b, 0xfb, 0x59, 0xd0, 0xc8, 0x22, 0x36, 0xdb, 0xd2, 0x63, 0x98, 0x43, 0xa1, 0x04, 0x87, 0x86, 0xf7, 0xa6, 0x26,
       0xbb, 0xd6, 0x59, 0x4d, 0xbf, 0x6a, 0x2e, 0xaa, 0x2b, 0xef, 0xe6, 0x78, 0xb6, 0x4e, 0xe0, 0x2f, 0xdc, 0x7c, 0xbe,
       0x57, 0x19, 0x32, 0x7e, 0x2a, 0xd0, 0xb8, 0xba, 0x29, 0x00, 0x3c, 0x52, 0x7d, 0xa8, 0x49, 0x3b, 0x2d, 0xeb, 0x25,
       0x49, 0xfa, 0xa3, 0xaa, 0x39, 0xa7, 0xc5, 0xa7, 0x50, 0x11, 0x36, 0xfb, 0xc6, 0x67, 0x4a, 0xf5, 0xa5, 0x12, 0x65,
       0x7e, 0xb0, 0xdf, 0xaf, 0x4e, 0xb3, 0x61, 0x7f, 0x2f}};
  Exiv2::byte key = 0;
  for (int i = 0; i < 4; ++i) {
    key ^= (count >> (i * 8)) & 0xff;
  }
  Exiv2::byte ci = xlat[0][serial & 0xff];
  Exiv2::byte cj = xlat[1][key];
  Exiv2::byte ck = 0x60;
  for (uint32_t i = 0; i < size; ++i) {
    cj += ci * ck++;
    pData[i] ^= cj;
  }
}
}  // namespace
