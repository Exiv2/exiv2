// SPDX-License-Identifier: GPL-2.0-or-later

#include "helper_functions.hpp"

#include <cmath>
#include <cstring>
#include <numeric>
#include "enforce.hpp"

std::string string_from_unterminated(const char* data, size_t data_length) {
  if (data_length == 0) {
    return {};
  }
  const size_t StringLength = strnlen(data, data_length);
  return {data, StringLength};
}

namespace {
std::string utf16ToUtf8(const std::u16string& wstr) {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
  auto str = std::string(wstr.begin(), wstr.end());
#pragma warning(pop)
#else
  auto str = std::string(wstr.begin(), wstr.end());
#endif
  str.erase(std::remove(str.begin(), str.end(), '\0'), str.end());
  return str;
}
}  // namespace

namespace Exiv2 {
uint64_t readQWORDTag(const BasicIo::UniquePtr& io) {
  Internal::enforce(QWORD <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf = io->read(QWORD);
  return FieldBuf.read_uint64(0, littleEndian);
}

uint32_t readDWORDTag(const BasicIo::UniquePtr& io) {
  Internal::enforce(DWORD <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf = io->read(DWORD);
  return FieldBuf.read_uint32(0, littleEndian);
}

uint16_t readWORDTag(const BasicIo::UniquePtr& io) {
  Internal::enforce(WORD <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf = io->read(WORD);
  return FieldBuf.read_uint16(0, littleEndian);
}

std::string readStringWcharTag(const BasicIo::UniquePtr& io, size_t length) {
  Internal::enforce(length <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf(length + 1);
  io->readOrThrow(FieldBuf.data(), length, ErrorCode::kerFailedToReadImageData);
  std::u16string wst(FieldBuf.begin(), FieldBuf.end());
  return utf16ToUtf8(wst);
}

std::string readStringTag(const BasicIo::UniquePtr& io, size_t length) {
  Internal::enforce(length <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf(length + 1);
  io->readOrThrow(FieldBuf.data(), length, ErrorCode::kerFailedToReadImageData);
  return Exiv2::toString(FieldBuf.data()).substr(0, length);
}

std::string getAspectRatio(uint64_t width, uint64_t height) {
  if (height == 0 || width == 0)
    return std::to_string(width) + ":" + std::to_string(height);

  auto ratioWidth = width / std::gcd(width, height);
  auto ratioHeight = height / std::gcd(width, height);
  return std::to_string(ratioWidth) + ":" + std::to_string(ratioHeight);
}

}  // namespace Exiv2
