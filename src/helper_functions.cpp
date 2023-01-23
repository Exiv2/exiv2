// SPDX-License-Identifier: GPL-2.0-or-later

#include "helper_functions.hpp"

#include <cmath>
#include <cstring>
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

std::string toString16(Exiv2::DataBuf& buf) {
  std::ostringstream os;
  char t;

  for (size_t i = 0; i < buf.size(); i += 2) {
    t = buf.data()[i] + 16 * buf.data()[i + 1];
    if (t == 0) {
      if (i)
        os << '\0';
      break;
    }
    os << t;
  }
  return os.str();
}

uint64_t readQWORDTag(BasicIo::UniquePtr& io) {
  enforce(QWORD <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
    DataBuf FieldBuf = io->read(QWORD);
  return FieldBuf.read_uint64(0, littleEndian);
}

uint32_t readDWORDTag(BasicIo::UniquePtr& io) {
  enforce(DWORD <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
   DataBuf FieldBuf = io->read(DWORD);
  return FieldBuf.read_uint32(0, littleEndian);
}

uint16_t readWORDTag(BasicIo::UniquePtr& io) {
  enforce(WORD <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf = io->read(WORD);
  return FieldBuf.read_uint16(0, littleEndian);
}

std::string readStringWcharTag(BasicIo::UniquePtr& io, uint16_t length) {
  enforce(length <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf = io->read(length);
  return toString16(FieldBuf);
}

std::string readStringTag(BasicIo::UniquePtr& io, uint16_t length) {
  enforce(length <= io->size() - io->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  DataBuf FieldBuf = io->read(length);
  return Exiv2::toString(FieldBuf.data());
}

}  // namespace Exiv2
