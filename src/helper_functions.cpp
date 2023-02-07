// SPDX-License-Identifier: GPL-2.0-or-later

#include "helper_functions.hpp"

#include <cmath>
#include <codecvt>
#include <cstring>
#include <locale>
#include "enforce.hpp"

std::string string_from_unterminated(const char* data, size_t data_length) {
  if (data_length == 0) {
    return {};
  }
  const size_t StringLength = strnlen(data, data_length);
  return {data, StringLength};
}

namespace Exiv2 {
char returnHex(int n) {
  if (n >= 0 && n <= 9)
    return static_cast<char>(n + 48);
  return static_cast<char>(n + 55);
}

std::string utf16ToUtf8(const std::wstring& wstr) {
  using convert_typeX = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_typeX, wchar_t> converterX;

  std::string str = converterX.to_bytes(wstr);
  str.erase(std::remove(str.begin(), str.end(), '\0'), str.end());
  return str;
}

uint64_t readQWORDTag(BasicIo::UniquePtr& io) {
  Internal::enforce(QWORD <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf = io->read(QWORD);
  return FieldBuf.read_uint64(0, littleEndian);
}

uint32_t readDWORDTag(BasicIo::UniquePtr& io) {
  Internal::enforce(DWORD <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf = io->read(DWORD);
  return FieldBuf.read_uint32(0, littleEndian);
}

uint16_t readWORDTag(BasicIo::UniquePtr& io) {
  Internal::enforce(WORD <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf = io->read(WORD);
  return FieldBuf.read_uint16(0, littleEndian);
}

std::string readStringWcharTag(BasicIo::UniquePtr& io, size_t length) {
  Internal::enforce(length <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf(length + 1);
  io->readOrThrow(FieldBuf.data(), length, ErrorCode::kerFailedToReadImageData);
  std::wstring wst(FieldBuf.begin(), FieldBuf.end());
  return utf16ToUtf8(wst);
}

std::string readStringTag(BasicIo::UniquePtr& io, size_t length) {
  Internal::enforce(length <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf(length + 1);
  io->readOrThrow(FieldBuf.data(), length, ErrorCode::kerFailedToReadImageData);
  return Exiv2::toString(FieldBuf.data()).substr(0, length);
}

}  // namespace Exiv2
