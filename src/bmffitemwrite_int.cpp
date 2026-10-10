// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffitemwrite_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include "enforce.hpp"
#include "error.hpp"

#include <algorithm>
#include <limits>
#include <utility>

namespace Exiv2::Internal {
namespace {
using Bytes = std::vector<uint8_t>;
constexpr uint64_t max32 = std::numeric_limits<uint32_t>::max();
constexpr uint32_t max16 = std::numeric_limits<uint16_t>::max();

// Reject inconsistent item tables before encoding them.
void require(bool condition) {
  enforce(condition, ErrorCode::kerCorruptedMetadata);
}

// Encode a field within the adapter's explicit structural allocation budget.
void number(Bytes& bytes, uint64_t value, unsigned width, uint64_t maxBytes) {
  require(width <= 8 && (width == 8 || value < (uint64_t{1} << (width * 8))));
  require(width <= maxBytes && bytes.size() <= maxBytes - width);
  for (unsigned i = width; i != 0; --i)
    bytes.push_back(static_cast<uint8_t>(value >> ((i - 1) * 8)));
}
}  // namespace

std::vector<BmffOutputBox> layoutBmffItems(BmffItemModel& model, const BmffItemLayoutBuilder& builder) {
  normalizeBmffLocations(model);

  // Only offset width can change after locating the output; it promotes 4 -> 8 once.
  for (unsigned pass = 0; pass < 2; ++pass) {
    auto boxes = builder.build();
    const auto size = layoutBmff(boxes);
    auto positions = builder.positions(boxes);
    if (relocateBmffItems(model, BmffRelocations(std::move(positions.retained)), positions.replacements,
                          positions.itemDataPosition))
      continue;

    // Rebuild offset-bearing boxes with the now-resolved positions.
    boxes = builder.build();
    require(layoutBmff(boxes) == size);
    return boxes;
  }

  throw Error(ErrorCode::kerCorruptedMetadata);
}

void normalizeBmffLocations(BmffItemModel& model) {
  require(!model.items.empty());
  auto& format = model.locationFormat;
  format.offsetSize = 4;
  format.lengthSize = 4;
  format.baseOffsetSize = 0;
  if (model.items.rbegin()->first > max16 || model.items.size() > max16)
    format.version = 2;

  // Preserve idat addressing, but eliminate per-item base offsets in the new layout.
  for (auto& [id, item] : model.items) {
    item.location.baseOffset = 0;
    if (item.location.constructionMethod == 1)
      format.version = std::max<uint8_t>(format.version, 1);
    for (const auto& extent : item.location.extents)
      if (extent.length > max32)
        format.lengthSize = 8;
  }
}

std::vector<uint8_t> encodeBmffLocations(const BmffItemModel& model, uint64_t maxBytes) {
  const auto& format = model.locationFormat;
  Bytes bytes;
  number(bytes, uint32_t{format.version} << 24, 4, maxBytes);
  number(bytes, (format.offsetSize << 4) | format.lengthSize, 1, maxBytes);
  number(bytes, format.indexSize, 1, maxBytes);
  const unsigned width = format.version == 2 ? 4 : 2;
  number(bytes, model.items.size(), width, maxBytes);

  // Retain location-table order while writing normalized extent coordinates.
  for (const auto id : model.locationOrder) {
    const auto& loc = model.items.at(id).location;
    number(bytes, id, width, maxBytes);
    if (format.version)
      number(bytes, loc.constructionMethod, 2, maxBytes);
    number(bytes, 0, 2, maxBytes);
    number(bytes, loc.extents.size(), 2, maxBytes);
    for (const auto& extent : loc.extents) {
      if (format.version && format.indexSize)
        number(bytes, extent.index, format.indexSize, maxBytes);
      // The first layout pass measures fields whose relocated offsets are not known yet.
      number(bytes, format.offsetSize == 4 && extent.offset > max32 ? 0 : extent.offset, format.offsetSize, maxBytes);
      number(bytes, extent.length, format.lengthSize, maxBytes);
    }
  }

  return bytes;
}

bool relocateBmffItems(BmffItemModel& model, const BmffRelocations& retained,
                       const std::map<uint32_t, uint64_t>& replacements, uint64_t itemDataPosition) {
  bool promoted = false;
  for (auto& [id, item] : model.items) {
    auto& location = item.location;
    for (auto& extent : location.extents) {
      uint64_t position = 0;
      if (const auto found = replacements.find(id); found != replacements.end()) {
        position = found->second;
      } else {
        position = retained.position(extent.source);
      }

      // Convert absolute output positions back to the coordinate system required by iloc.
      const auto base = location.constructionMethod == 1 ? itemDataPosition : 0;
      require(position >= base);
      extent.offset = position - base;
      if (extent.offset > max32 && model.locationFormat.offsetSize != 8) {
        model.locationFormat.offsetSize = 8;
        promoted = true;
      }
    }
  }

  return promoted;
}

}  // namespace Exiv2::Internal
#endif
