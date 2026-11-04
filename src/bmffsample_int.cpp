// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffsample_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include "enforce.hpp"
#include "error.hpp"

namespace Exiv2::Internal {
namespace {

// Require complete version-zero tables before interpreting their fixed-width entries.
BmffCursor fields(BmffReader& reader, const BmffBox& box) {
  auto input = reader.cursor(box.payload());
  enforce(input.number(4) == 0, ErrorCode::kerCorruptedMetadata);
  return input;
}

// Reject truncated or trailing table entries without multiplying an untrusted count.
void entries(const BmffCursor& input, uint64_t count, unsigned width) {
  enforce(input.remaining() % width == 0 && input.remaining() / width == count, ErrorCode::kerCorruptedMetadata);
}

}  // namespace

std::optional<BmffSingleSample> readBmffSingleSample(BmffReader& reader, const BmffBox& table, unsigned depth) {
  std::optional<BmffBox> mapping;
  std::optional<BmffBox> sizes;
  std::optional<BmffBox> offsets;
  bool compactSizes = false;
  auto input = reader.cursor(table.payload());

  // Only standard singleton tables are owned here; other child payloads remain opaque.
  reader.visit(input, depth, [&](BmffBox& box, BmffCursor&) {
    std::optional<BmffBox>* target = nullptr;
    if (box.type == bmffType("stsc"))
      target = &mapping;
    else if (box.type == bmffType("stsz"))
      target = &sizes;
    else if (box.type == bmffType("stco") || box.type == bmffType("co64"))
      target = &offsets;
    else if (box.type == bmffType("stz2"))
      compactSizes = true;

    if (target) {
      enforce(!target->has_value(), ErrorCode::kerCorruptedMetadata);
      *target = box;
    }
  });
  if (!mapping || !sizes || !offsets || compactSizes)
    return std::nullopt;

  // A still sample occupies exactly one chunk and uses its explicit description index.
  auto map = fields(reader, *mapping);
  const auto mapCount = map.number(4);
  entries(map, mapCount, 12);
  if (mapCount != 1)
    return std::nullopt;
  const auto firstChunk = map.number(4);
  const auto samplesPerChunk = map.number(4);
  const auto description = static_cast<uint32_t>(map.number(4));
  if (firstChunk != 1 || samplesPerChunk != 1 || description == 0)
    return std::nullopt;

  // stsz either repeats one fixed size or stores the singleton size out of line.
  auto sizeFields = fields(reader, *sizes);
  const auto fixedSize = sizeFields.number(4);
  const auto sampleCount = sizeFields.number(4);
  entries(sizeFields, fixedSize == 0 ? sampleCount : 0, 4);
  if (sampleCount != 1)
    return std::nullopt;
  const auto size = fixedSize == 0 ? sizeFields.number(4) : fixedSize;
  if (size == 0)
    return std::nullopt;

  // Chunk offsets are file-relative, independently of the size of the box headers.
  auto chunkFields = fields(reader, *offsets);
  const auto chunkCount = chunkFields.number(4);
  const unsigned width = offsets->type == bmffType("co64") ? 8 : 4;
  entries(chunkFields, chunkCount, width);
  if (chunkCount != 1)
    return std::nullopt;
  const BmffSpan range{chunkFields.number(width), size};
  (void)reader.cursor(range);
  return BmffSingleSample{range, description};
}

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
