// SPDX-License-Identifier: GPL-2.0-or-later

#include "cr3preview_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include "bmffsample_int.hpp"
#include "enforce.hpp"
#include "error.hpp"

#include <algorithm>
#include <limits>

namespace Exiv2::Internal {
namespace {

// Collect only immediate headers; unknown payloads and compressed media stay opaque.
std::vector<BmffBox> children(BmffReader& reader, BmffSpan range, unsigned depth) {
  std::vector<BmffBox> boxes;
  auto input = reader.cursor(range);
  reader.visit(input, depth, [&](BmffBox& box, BmffCursor&) { boxes.push_back(box); });
  return boxes;
}

// Borrow an unambiguous child from a collection that must outlive the pointer.
const BmffBox* unique(const std::vector<BmffBox>& boxes, uint32_t type) {
  const BmffBox* result = nullptr;
  for (const auto& box : boxes) {
    if (box.type == type) {
      enforce(result == nullptr, ErrorCode::kerCorruptedMetadata);
      result = &box;
    }
  }
  return result;
}

// Select a one-based entry from a version-zero, counted FullBox without buffering entries.
std::optional<BmffBox> indexed(BmffReader& reader, const BmffBox& table, uint32_t index, unsigned depth) {
  auto input = reader.cursor(table.payload());
  enforce(input.number(4) == 0, ErrorCode::kerCorruptedMetadata);
  const auto count = input.number(4);
  if (index == 0 || index > count)
    return std::nullopt;

  std::optional<BmffBox> selected;
  uint64_t visited = 0;
  reader.visit(
      input, depth,
      [&](BmffBox& entry, BmffCursor&) {
        if (++visited == index)
          selected = entry;
      },
      count);
  enforce(visited == count, ErrorCode::kerCorruptedMetadata);
  input.finish();
  return selected;
}

// Follow the description's data reference only when it explicitly names this file.
bool localReference(BmffReader& reader, const BmffBox& dinf, uint32_t index) {
  const auto boxes = children(reader, dinf.payload(), 5);
  const auto dref = unique(boxes, bmffType("dref"));
  if (!dref)
    return false;
  const auto entry = indexed(reader, *dref, index, 6);
  if (!entry || entry->type != bmffType("url "))
    return false;

  auto input = reader.cursor(entry->payload());
  return input.number(4) == 1 && input.remaining() == 0;
}

// Interpret Canon's 82-byte visual-entry prefix and require its explicit JPEG child.
std::optional<NativePreview> description(BmffReader& reader, const BmffBox& entry, const BmffBox& dinf) {
  if (entry.type != bmffType("CRAW"))
    return std::nullopt;

  // Canon offsets are relative to the payload, so extended headers need no adjustment.
  // Reference: lclevy/canon_cr3 revision 151c1a965775ef409ace17714c24ed491cfcf9c4.
  auto input = reader.cursor(entry.payload());
  input.advance(6);
  const auto reference = static_cast<uint32_t>(input.number(2));
  input.advance(16);
  NativePreview preview;
  preview.width_ = static_cast<size_t>(input.number(2));
  preview.height_ = static_cast<size_t>(input.number(2));
  input.advance(54);
  if (preview.width_ == 0 || preview.height_ == 0)
    return std::nullopt;

  const auto boxes = children(reader, {input.position(), input.remaining()}, 7);
  const auto jpeg = unique(boxes, bmffType("JPEG"));
  if (!jpeg || unique(boxes, bmffType("CMP1")) || unique(boxes, bmffType("CDI1")) || unique(boxes, bmffType("hvcC")) ||
      unique(boxes, bmffType("sinf")))
    return std::nullopt;
  auto marker = reader.cursor(jpeg->payload());
  if (marker.number(4) != 0 || marker.remaining() != 0 || !localReference(reader, dinf, reference))
    return std::nullopt;

  preview.mimeType_ = "image/jpeg";
  return preview;
}

// Require media ownership, rather than accepting offsets into headers or unrelated boxes.
bool inMedia(const std::vector<BmffBox>& roots, BmffSpan sample) {
  return std::any_of(roots.begin(), roots.end(), [&](const BmffBox& box) {
    if (box.type != bmffType("mdat"))
      return false;
    const auto media = box.payload();
    return sample.offset >= media.offset && sample.offset - media.offset <= media.size &&
           sample.size <= media.size - (sample.offset - media.offset);
  });
}

// Resolve one ordinary still-image track; timed and unsupported codecs yield no candidate.
std::optional<NativePreview> trackPreview(BmffReader& reader, const BmffBox& track, const std::vector<BmffBox>& roots) {
  const auto trackBoxes = children(reader, track.payload(), 2);
  const auto mdia = unique(trackBoxes, bmffType("mdia"));
  if (!mdia)
    return std::nullopt;
  const auto mediaBoxes = children(reader, mdia->payload(), 3);
  const auto hdlr = unique(mediaBoxes, bmffType("hdlr"));
  const auto minf = unique(mediaBoxes, bmffType("minf"));
  if (!hdlr || !minf)
    return std::nullopt;
  auto handler = reader.cursor(hdlr->payload());
  if (handler.number(4) != 0)
    return std::nullopt;
  handler.advance(4);
  if (handler.number(4) != bmffType("vide"))
    return std::nullopt;

  // Sample mechanics are standard; description interpretation stays in this adapter.
  const auto infoBoxes = children(reader, minf->payload(), 4);
  const auto table = unique(infoBoxes, bmffType("stbl"));
  const auto dinf = unique(infoBoxes, bmffType("dinf"));
  if (!table || !dinf)
    return std::nullopt;
  const auto sample = readBmffSingleSample(reader, *table, 5);
  if (!sample || sample->data.size < 2 || !inMedia(roots, sample->data) ||
      sample->data.offset > std::numeric_limits<size_t>::max() ||
      sample->data.size > std::numeric_limits<size_t>::max() - sample->data.offset)
    return std::nullopt;
  const auto tables = children(reader, table->payload(), 5);
  const auto stsd = unique(tables, bmffType("stsd"));
  if (!stsd)
    return std::nullopt;
  const auto entry = indexed(reader, *stsd, sample->descriptionIndex, 6);
  if (!entry)
    return std::nullopt;
  auto preview = description(reader, *entry, *dinf);
  if (!preview)
    return std::nullopt;

  // Discovery reads the signature only; PreviewManager performs normal extraction later.
  auto signature = reader.cursor({sample->data.offset, 2});
  if (signature.number(2) != 0xffd8)
    return std::nullopt;
  preview->position_ = static_cast<size_t>(sample->data.offset);
  preview->size_ = static_cast<size_t>(sample->data.size);
  return preview;
}

}  // namespace

std::optional<NativePreview> findCr3JpegPreview(BasicIo& io, size_t maxDepth) {
  try {
    // These limits apply only to optional CR3 track discovery, never legacy metadata reads.
    const auto depth = static_cast<unsigned>(std::min<size_t>(maxDepth, std::numeric_limits<unsigned>::max()));
    BmffReader reader(io, {4096, 1024 * 1024, depth});
    const auto roots = children(reader, {0, reader.fileSize()}, 0);
    const auto ftyp = unique(roots, bmffType("ftyp"));
    const auto moov = unique(roots, bmffType("moov"));
    if (!ftyp || !moov)
      return std::nullopt;
    auto brand = reader.cursor(ftyp->payload());
    if (brand.number(4) != bmffType("crx "))
      return std::nullopt;

    // At most one new candidate is needed; equal-area tracks retain original file order.
    std::optional<NativePreview> best;
    for (const auto& track : children(reader, moov->payload(), 1)) {
      if (track.type != bmffType("trak"))
        continue;
      const auto candidate = trackPreview(reader, track, roots);
      if (candidate &&
          (!best || uint64_t{candidate->width_} * candidate->height_ > uint64_t{best->width_} * best->height_))
        best = candidate;
    }
    return best;
  } catch (const Error& error) {
    // Corrupt optional tracks and exhausted discovery budgets must not hide readable metadata.
    if (error.code() != ErrorCode::kerCorruptedMetadata)
      throw;
    return std::nullopt;
  }
}

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
