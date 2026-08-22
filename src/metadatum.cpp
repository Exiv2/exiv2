// SPDX-License-Identifier: GPL-2.0-or-later

#include "metadatum.hpp"
#include <limits>
#include "enforce.hpp"

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
  auto i = toInt64(n);
  Internal::enforce(std::numeric_limits<uint32_t>::min() <= i, ErrorCode::kerCorruptedMetadata);
  Internal::enforce(i <= std::numeric_limits<uint32_t>::max(), ErrorCode::kerCorruptedMetadata);
  return static_cast<uint32_t>(i);
}

bool cmpMetadataByTag(const Metadatum& lhs, const Metadatum& rhs) {
  return lhs.tag() < rhs.tag();
}

bool cmpMetadataByKey(const Metadatum& lhs, const Metadatum& rhs) {
  return lhs.key() < rhs.key();
}

}  // namespace Exiv2
