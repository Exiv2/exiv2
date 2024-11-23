// SPDX-License-Identifier: GPL-2.0-or-later

#include "helper_functions.hpp"

#include "convert.hpp"
#include "enforce.hpp"

#include <cmath>
#include <cstring>
#include <numeric>

std::string string_from_unterminated(const char* data, size_t data_length) {
  if (data_length == 0) {
    return {};
  }
  const size_t StringLength = strnlen(data, data_length);
  return {data, StringLength};
}

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
  std::string wst(FieldBuf.begin(), FieldBuf.end() - 3);
  if (wst.size() % 2 != 0)
    Exiv2::convertStringCharset(wst, "UCS-2LE", "UTF-8");
  Exiv2::convertStringCharset(wst, "UCS-2LE", "UTF-8");
  return wst;
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
