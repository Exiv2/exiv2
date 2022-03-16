// SPDX-License-Identifier: GPL-2.0-or-later

#include "rw2image_int.hpp"

namespace Exiv2::Internal {
Rw2Header::Rw2Header() : TiffHeaderBase(0x0055, 24, littleEndian, 0x00000018) {
}

DataBuf Rw2Header::write() const {
  // Todo: Implement me!
  return {};
}

}  // namespace Exiv2::Internal
