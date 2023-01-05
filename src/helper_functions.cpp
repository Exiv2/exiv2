// SPDX-License-Identifier: GPL-2.0-or-later

#include "helper_functions.hpp"

#include <cmath>
#include <cstring>

std::string string_from_unterminated(const char* data, size_t data_length) {
  if (data_length == 0) {
    return {};
  }
  const size_t StringLength = strnlen(data, data_length);
  return {data, StringLength};
}

namespace Util {
char returnHEX(int n) {
  if (n >= 0 && n <= 9)
    return static_cast<char>(n + 48);
  return static_cast<char>(n + 55);
}

std::string toString16(Exiv2::DataBuf& buf) {
  std::ostringstream os;
  char t;

  for (size_t i = 0; i <= buf.size(); i += 2) {
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

uint64_t getUint64_t(Exiv2::DataBuf& buf) {
  uint64_t temp = 0;

  for (int i = 0; i < 8; ++i) {
    temp = temp + static_cast<uint64_t>(buf.data()[i] * (pow(static_cast<float>(256), i)));
  }
  return temp;
}
}  // namespace Util
