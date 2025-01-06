// SPDX-License-Identifier: GPL-2.0-or-later

#include "image_int.hpp"

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <vector>

namespace Exiv2::Internal {
[[nodiscard]] std::string indent(size_t i) {
  return std::string(2 * i, ' ');
}

}  // namespace Exiv2::Internal
