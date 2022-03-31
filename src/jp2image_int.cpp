// SPDX-License-Identifier: GPL-2.0-or-later

#include "jp2image_int.hpp"

#include "error.hpp"
#include "types.hpp"

#include <cassert>

namespace Exiv2::Internal {

bool isValidBoxFileType(const std::vector<uint8_t>& boxData) {
  // BR & MinV are obligatory (4 + 4 bytes). Afterwards we have N compatibility lists (of size 4)
  if (boxData.size() < 8 || ((boxData.size() - 8u) % 4u) != 0) {
    return false;
  }

  const size_t N = (boxData.size() - 8u) / 4u;
  const uint32_t brand = getULong(boxData.data(), bigEndian);
  const uint32_t minorVersion = getULong(boxData.data() + 4, bigEndian);

  bool clWithRightBrand = false;
  for (size_t i = 0; i < N; i++) {
    uint32_t compatibilityList = getULong(boxData.data() + 8 + i * 4, bigEndian);
    if (compatibilityList == brandJp2) {
      clWithRightBrand = true;
      break;
    }
  }
  return (brand == brandJp2 && minorVersion == 0 && clWithRightBrand);
}
}  // namespace Exiv2::Internal
