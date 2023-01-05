// SPDX-License-Identifier: GPL-2.0-or-later

#include "metadatum.hpp"

namespace Exiv2 {
Key::UniquePtr Key::clone() const {
  return UniquePtr(clone_());
}

std::string Metadatum::print(const ExifData* pMetadata) const {
  std::ostringstream os;
  write(os, pMetadata);
  return os.str();
}

uint32_t Metadatum::toUint32(size_t n) const {
  return static_cast<uint32_t>(toInt64(n));
}

bool cmpMetadataByTag(const Metadatum& lhs, const Metadatum& rhs) {
  return lhs.tag() < rhs.tag();
}

bool cmpMetadataByKey(const Metadatum& lhs, const Metadatum& rhs) {
  return lhs.key() < rhs.key();
}

}  // namespace Exiv2
