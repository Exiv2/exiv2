// SPDX-License-Identifier: GPL-2.0-or-later

#include "helper_functions.hpp"

#include <cstring>

std::string string_from_unterminated(const char* data, size_t data_length) {
  if (data_length == 0) {
    return {};
  }
  const size_t StringLength = strnlen(data, data_length);
  return {data, StringLength};
}
