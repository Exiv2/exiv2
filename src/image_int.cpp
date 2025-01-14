// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.h"

#include "image_int.hpp"

namespace Exiv2::Internal {
[[nodiscard]] std::string indent(size_t i) {
  return std::string(2 * i, ' ');
}

}  // namespace Exiv2::Internal
