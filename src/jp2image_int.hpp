// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef JP2IMAGE_INT_HPP
#define JP2IMAGE_INT_HPP

#include <cstdint>
#include <vector>

namespace Exiv2::Internal {

struct Jp2BoxHeader {
  uint32_t length;
  uint32_t type;
};

struct Jp2ImageHeaderBox {
  uint32_t imageHeight;
  uint32_t imageWidth;
  uint16_t componentCount;
  uint8_t bpc;   //<! Bits per component
  uint8_t c;     //<! Compression type
  uint8_t unkC;  //<! Colourspace unknown
  uint8_t ipr;   //<! Intellectual property
};

struct Jp2UuidBox {
  uint8_t uuid[16];
};

constexpr uint32_t brandJp2{0x6a703220};

/// @brief Determines if the File Type box is valid
bool isValidBoxFileType(const std::vector<std::uint8_t>& boxData);
}  // namespace Exiv2::Internal

#endif  // JP2IMAGE_INT_HPP
