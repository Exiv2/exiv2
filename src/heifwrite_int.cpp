// SPDX-License-Identifier: GPL-2.0-or-later

#include "heifwrite_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include "basicio.hpp"
#include "bmffitemwrite_int.hpp"
#include "bmfflayout_int.hpp"
#include "enforce.hpp"
#include "error.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <string_view>

namespace Exiv2::Internal {
namespace {
using Bytes = std::vector<uint8_t>;
using Box = BmffOutputBox;
constexpr uint64_t maxPosition = std::numeric_limits<int64_t>::max();
constexpr uint64_t max32 = std::numeric_limits<uint32_t>::max();
constexpr uint32_t max16 = std::numeric_limits<uint16_t>::max();

// Reject inconsistent layout state through the common metadata error code.
void require(bool condition) {
  enforce(condition, ErrorCode::kerCorruptedMetadata);
}

// Explain why an otherwise parsed input cannot support the requested edit.
void supported(bool condition, std::string_view reason) {
  if (!condition)
    throw Error(ErrorCode::kerErrorMessage, std::string("Unsupported HEIF edit: ") + std::string(reason));
}

// Add positions within the signed seek range supported by BasicIo.
uint64_t add(uint64_t a, uint64_t b) {
  require(a <= maxPosition && b <= maxPosition - a);
  return a + b;
}

// Return the checked exclusive end of an input or output span.
uint64_t end(BmffSpan span) {
  return add(span.offset, span.size);
}

// Append a bounded big-endian field, rejecting values that do not fit its width.
void number(Bytes& bytes, uint64_t value, unsigned width) {
  require(width <= 8 && (width == 8 || value < (uint64_t{1} << (width * 8))));
  require(bytes.size() <= HeifLimits{}.boxes.maxBytesRead - width);

  for (unsigned i = width; i != 0; --i)
    bytes.push_back(static_cast<uint8_t>(value >> ((i - 1) * 8)));
}

// Append a terminated string within the structural serialization budget.
void string(Bytes& bytes, std::string_view value) {
  require(value.size() < HeifLimits{}.boxes.maxBytesRead &&
          bytes.size() < HeifLimits{}.boxes.maxBytesRead - value.size());
  bytes.insert(bytes.end(), value.begin(), value.end());
  bytes.push_back(0);
}

// Construct the version/flags prefix shared by FullBox records.
Bytes full(uint8_t version, uint32_t flags = 0) {
  Bytes bytes;
  number(bytes, (uint32_t{version} << 24) | flags, 4);
  return bytes;
}

// Identify padding whose old bytes must not survive compaction.
bool padding(uint32_t type) {
  return type == bmffType("free") || type == bmffType("skip");
}

//! @brief Unite only ranges from the same source payload. Original item boundaries stay in iloc.
void mergeRanges(std::vector<BmffSpan>& spans) {
  std::sort(spans.begin(), spans.end(), [](auto a, auto b) { return a.offset < b.offset; });

  size_t count = 0;
  for (const auto span : spans) {
    if (count && span.offset <= end(spans[count - 1])) {
      auto& previous = spans[count - 1];
      previous.size = std::max(end(previous), end(span)) - previous.offset;
    } else {
      spans[count++] = span;
    }
  }

  spans.resize(count);
}

// Prepare, emit, and verify a compact container without transferring it to the source.
class Rewrite : public BmffItemLayoutBuilder {
 public:
  //! @brief Borrow both streams and the original model; keep a separate model for the edit.
  Rewrite(BasicIo& input, BasicIo& output, const HeifDocument& original) :
      input_(input), output_(output), original_(original), document_(original) {
  }

  //! @brief Validate the edit, converge its layout, and verify the prepared output.
  void run(const HeifMetadataUpdate& update) {
    require(&input_ != &output_ && input_.isopen() && output_.isopen() && output_.size() == 0);
    require(input_.size() == original_.file.size);
    enforceHeifWriteSupport(original_);

    // Check graph and byte ownership before preparing any output.
    select(bmffType("Exif"), update.exif);
    select(bmffType("mime"), update.xmp);
    validateReferences();
    collectRanges();
    applyUpdates();

    // Index retained structures and normalize field widths before layout iteration.
    for (const auto& entry : document_.meta.associations)
      associations_[entry.box.offset].push_back(&entry);
    indexBoxes(original_.file.boxes);
    const auto boxes = layoutBmffItems(document_.meta, *this);

    // Only a stable layout may be emitted and compared with the requested result.
    writeBmffLayout(input_, output_, boxes);
    verify(boxes);
  }

 private:
  // Preserve absent updates; otherwise remove old primary items and retain any replacement.
  void select(uint32_t type, const std::optional<Bytes>& value) {
    if (!value)
      return;

    supported(value->size() <= heifMetadataLimit, "metadata exceeds the allocation limit");
    const auto ids = original_.metadataItems(type);
    removed_.insert(ids.begin(), ids.end());
    if (!value->empty())
      additions_.emplace(type, &*value);
  }

  // Reject removal of metadata that also participates in retained relationships.
  void validateReferences() const {
    for (const auto& ref : original_.meta.references) {
      if (removed_.contains(ref.from)) {
        supported(
            ref.type == bmffType("cdsc") &&
                std::all_of(ref.to.begin(), ref.to.end(), [&](auto id) { return id == original_.meta.primaryItem; }),
            "selected metadata is shared or has other relationships");
      } else {
        for (const auto id : ref.to)
          supported(!removed_.contains(id), "selected metadata is referenced by another item");
      }
    }
  }

  // Find the owning media payload, keyed by its absolute start in the input.
  uint64_t container(const BmffItem& item, BmffSpan span) const {
    if (item.location.constructionMethod == 1) {
      require(original_.meta.itemData.has_value());
      const auto data = *original_.meta.itemData;
      require(span.offset >= data.offset && end(span) <= end(data));
      return data.offset;
    }

    // Method 0 extents belong to the last mdat payload starting at or before them.
    auto found = std::upper_bound(original_.mediaData.begin(), original_.mediaData.end(), span.offset,
                                  [](uint64_t offset, auto range) { return offset < range.offset; });
    require(found != original_.mediaData.begin());
    --found;
    require(span.offset >= found->offset && end(span) <= end(*found));
    return found->offset;
  }

  // Collect retained byte unions and prove they do not overlap discarded metadata.
  void collectRanges() {
    std::vector<BmffSpan> discarded;
    for (const auto& [id, item] : original_.meta.items) {
      for (const auto& extent : item.location.extents) {
        if (removed_.contains(id))
          discarded.push_back(extent.source);
        else
          ranges_[container(item, extent.source)].push_back(extent.source);
      }
    }

    // Copy each retained byte once, even when several items share that byte.
    std::vector<BmffSpan> retained;
    for (auto& [offset, spans] : ranges_) {
      mergeRanges(spans);
      retained.insert(retained.end(), spans.begin(), spans.end());
    }

    // Removal is impossible when another item still needs any discarded byte.
    mergeRanges(discarded);
    size_t next = 0;
    for (const auto span : discarded) {
      while (next < retained.size() && end(retained[next]) <= span.offset)
        ++next;
      supported(next == retained.size() || retained[next].offset >= end(span),
                "removed metadata overlaps a retained item");
    }
  }

  // Prefer an ID above the existing set, falling back to its first gap at the limit.
  uint32_t allocateId() const {
    require(!document_.meta.items.empty());
    const auto greatest = document_.meta.items.rbegin()->first;
    if (greatest != max32)
      return greatest + 1;

    // At the maximum ID, search existing gaps rather than wrapping the counter.
    uint32_t id = 1;
    for (const auto& [used, item] : document_.meta.items) {
      if (used != id)
        return id;
      supported(id != max32, "no unused item ID");
      ++id;
    }
    return id;
  }

  // Remove old metadata records and create primary-image descriptions for replacements.
  void applyUpdates() {
    // Drop the removed items and all structures owned by those metadata items.
    for (const auto id : removed_)
      document_.meta.items.erase(id);
    std::erase_if(document_.meta.infoOrder, [&](auto id) { return removed_.contains(id); });
    std::erase_if(document_.meta.locationOrder, [&](auto id) { return removed_.contains(id); });
    std::erase_if(document_.meta.references, [&](const auto& ref) { return removed_.contains(ref.from); });
    std::erase_if(document_.meta.associations, [&](const auto& entry) { return removed_.contains(entry.itemId); });

    // Every replacement gets a new description and a cdsc link to the primary image.
    for (const auto& [type, bytes] : additions_) {
      require(document_.meta.items.size() < HeifLimits{}.items.maxItems);
      const auto id = allocateId();
      BmffItem item;
      item.info.version = id > max16 ? 3 : 2;
      item.info.flags = 1;
      item.info.type = type;
      item.info.name = type == bmffType("Exif") ? "Exif" : "XMP";
      if (type == bmffType("mime"))
        item.info.contentType = "application/rdf+xml";

      item.location.dataSize = bytes->size();
      item.location.extents.push_back({0, 0, bytes->size(), {}});
      document_.meta.items.emplace(id, std::move(item));
      document_.meta.infoOrder.push_back(id);
      document_.meta.locationOrder.push_back(id);
      document_.meta.references.push_back({{}, bmffType("cdsc"), id, {document_.meta.primaryItem}});
      newItems_.emplace(id, bytes);
    }
  }

  // Index original spans for preserving unchanged box headers and payloads.
  void indexBoxes(const std::vector<BmffBox>& boxes) {
    for (const auto& box : boxes) {
      originalBoxes_.emplace(box.span.offset, &box);
      indexBoxes(box.children);
    }
  }

  // Preserve existing infe entries or serialize a description for a new metadata item.
  Box itemInfo(uint32_t id) const {
    const auto& info = document_.meta.items.at(id).info;
    if (info.box.size)
      return opaque(*originalBoxes_.at(info.box.offset));

    Box box;
    box.type = bmffType("infe");
    box.prefix = full(info.version, info.flags);
    number(box.prefix, id, info.version == 2 ? 2 : 4);
    number(box.prefix, 0, 2);
    number(box.prefix, info.type, 4);
    string(box.prefix, info.name);
    if (info.type == bmffType("mime")) {
      string(box.prefix, info.contentType);
      string(box.prefix, info.contentEncoding);
    }

    return box;
  }

  // Rebuild directed references, widening IDs while retaining original child ordering.
  Box references(const BmffBox* original = nullptr) const {
    Box box;
    box.type = bmffType("iref");
    box.extended = original && original->headerSize == 16;
    uint8_t version = original ? original->fullBox->version : 0;
    if (document_.meta.items.rbegin()->first > max16)
      version = 1;
    box.prefix = full(version);

    // Relationships retain their direction and destination order.
    for (const auto& ref : document_.meta.references) {
      Box child;
      child.type = ref.type;
      if (ref.box.size)
        child.extended = originalBoxes_.at(ref.box.offset)->headerSize == 16;
      number(child.prefix, ref.from, version ? 4 : 2);
      number(child.prefix, ref.to.size(), 2);
      for (const auto id : ref.to)
        number(child.prefix, id, version ? 4 : 2);
      box.children.push_back(std::move(child));
    }

    return box;
  }

  // Represent an unchanged payload as a borrowed range rather than an allocated copy.
  static Box opaque(const BmffBox& original) {
    Box box;
    box.type = original.type;
    box.extended = original.headerSize == (original.type == bmffType("uuid") ? 32 : 16);
    box.uuid = original.userType;
    box.segments.push_back({original.payload()});
    return box;
  }

  // Rebuild modeled structures; preserve opaque bytes only after the write gate accepts them.
  std::optional<Box> rebuild(const BmffBox& original, bool property = false) const {
    auto box = opaque(original);
    if (padding(box.type)) {
      if (!property)
        return {};
      box.segments.clear();  // Keep indexed property slots; discard their padding bytes.
    } else if (box.type == bmffType("mdat") || box.type == bmffType("idat")) {
      // Build media solely from retained ranges, excluding old metadata and gaps.
      box.segments.clear();
      const auto found = ranges_.find(original.payload().offset);
      if (found == ranges_.end())
        return {};
      for (const auto range : found->second)
        box.segments.push_back({range});
    } else if (box.type == bmffType("iloc")) {
      box.segments.clear();
      box.prefix = encodeBmffLocations(document_.meta, HeifLimits{}.boxes.maxBytesRead);
    } else if (box.type == bmffType("iinf")) {
      box.segments.clear();
      auto version = original.fullBox->version;
      if (document_.meta.items.size() > max16)
        version = std::max<uint8_t>(version, 1);
      box.prefix = full(version);
      number(box.prefix, document_.meta.items.size(), version ? 4 : 2);
      for (const auto id : document_.meta.infoOrder)
        box.children.push_back(itemInfo(id));
    } else if (box.type == bmffType("iref")) {
      box = references(&original);
    } else if (box.type == bmffType("ipma")) {
      // Keep property slots stable while removing associations owned by deleted items.
      box.segments.clear();
      const auto format = *original.fullBox;
      box.prefix = full(format.version, format.flags);
      const auto found = associations_.find(original.span.offset);
      number(box.prefix, found == associations_.end() ? 0 : found->second.size(), 4);
      if (found == associations_.end())
        return box;
      for (const auto* pointer : found->second) {
        const auto& entry = *pointer;

        number(box.prefix, entry.itemId, format.version ? 4 : 2);
        number(box.prefix, entry.properties.size(), 1);
        for (const auto& prop : entry.properties)
          number(box.prefix, prop.index | (prop.essential ? (format.flags & 1 ? 0x8000 : 0x80) : 0),
                 format.flags & 1 ? 2 : 1);
      }
    } else if (!original.children.empty() || box.type == bmffType("ipco")) {
      box.segments.clear();
      if (original.fullBox)
        box.prefix = full(original.fullBox->version, original.fullBox->flags);

      // Recurse in original order and add an iref box only if the edit first needs one.
      bool hasReferences = false;
      for (const auto& child : original.children) {
        hasReferences |= child.type == bmffType("iref");
        if (auto rebuilt = rebuild(child, box.type == bmffType("ipco")))
          box.children.push_back(std::move(*rebuilt));
      }
      if (box.type == bmffType("meta") && !hasReferences && !document_.meta.references.empty())
        box.children.push_back(references());
      if (box.type == bmffType("dref"))
        number(box.prefix, box.children.size(), 4);
    }

    return box;
  }

  //! @brief Preserve top-level order and append one media box for replacement metadata.
  std::vector<Box> build() const override {
    std::vector<Box> boxes;
    for (const auto& box : original_.file.boxes)
      if (auto rebuilt = rebuild(box))
        boxes.push_back(std::move(*rebuilt));

    // New metadata lives in one appended mdat; existing image bytes are not duplicated.
    if (!newItems_.empty()) {
      Box box;
      box.type = bmffType("mdat");
      for (const auto& [id, bytes] : newItems_)
        box.segments.push_back({{}, bytes});
      boxes.push_back(std::move(box));
    }

    return boxes;
  }

  // Select only adapter-approved media mappings and the current idat origin.
  static void collectPositions(const Box& box, BmffItemPositions& positions,
                               std::map<const Bytes*, uint64_t>& replacements) {
    if (box.type == bmffType("idat"))
      positions.itemDataPosition = add(box.position, box.header);
    for (const auto& child : box.children)
      collectPositions(child, positions, replacements);
    for (const auto& segment : box.segments) {
      if (segment.bytes)
        replacements.emplace(segment.bytes, segment.position);
      else if (box.type == bmffType("mdat") || box.type == bmffType("idat"))
        positions.retained.push_back({segment.source, segment.position});
    }
  }

  //! @brief Select retained media and replacement positions using HEIF's ownership policy.
  BmffItemPositions positions(const std::vector<Box>& boxes) const override {
    BmffItemPositions result;
    std::map<const Bytes*, uint64_t> replacements;
    for (const auto& box : boxes)
      collectPositions(box, result, replacements);

    for (const auto& [id, bytes] : newItems_)
      result.replacements.emplace(id, replacements.at(bytes));
    return result;
  }

  // Reparse the output and check item identity, relocated addressing, relationships, and bytes.
  void verify(const std::vector<Box>& boxes) {
    const auto parsed = parseHeif(output_);

    // The prepared structure must describe the same retained images and item ordering.
    enforceHeifWriteSupport(parsed);
    require(parsed.majorBrand == document_.majorBrand && parsed.minorVersion == document_.minorVersion &&
            parsed.compatibleBrands == document_.compatibleBrands &&
            parsed.meta.primaryItem == document_.meta.primaryItem &&
            parsed.meta.infoOrder == document_.meta.infoOrder &&
            parsed.meta.locationOrder == document_.meta.locationOrder &&
            parsed.meta.items.size() == document_.meta.items.size());

    // Check every item description and each relocated extent against the planned model.
    for (const auto& [id, item] : document_.meta.items) {
      const auto& actual = parsed.meta.items.at(id);
      const auto& info = item.info;
      require(actual.info.type == info.type && actual.info.name == info.name && actual.info.flags == info.flags &&
              actual.info.contentType == info.contentType && actual.info.contentEncoding == info.contentEncoding &&
              actual.info.uriType == info.uriType);
      const auto& expected = item.location;
      require(actual.location.constructionMethod == expected.constructionMethod && actual.location.baseOffset == 0 &&
              actual.location.dataSize == expected.dataSize &&
              actual.location.extents.size() == expected.extents.size());
      for (size_t i = 0; i < expected.extents.size(); ++i) {
        const auto& a = actual.location.extents[i];
        const auto& e = expected.extents[i];
        require(a.index == e.index && a.offset == e.offset && a.length == e.length);
      }
    }

    // Relationships and property associations must survive with identical meanings.
    require(parsed.meta.references.size() == document_.meta.references.size() &&
            parsed.meta.associations.size() == document_.meta.associations.size() &&
            parsed.meta.properties.size() == document_.meta.properties.size());
    for (size_t i = 0; i < document_.meta.references.size(); ++i) {
      const auto& a = parsed.meta.references[i];
      const auto& e = document_.meta.references[i];
      require(a.type == e.type && a.from == e.from && a.to == e.to);
    }
    for (size_t i = 0; i < document_.meta.associations.size(); ++i) {
      const auto& a = parsed.meta.associations[i];
      const auto& e = document_.meta.associations[i];
      require(a.itemId == e.itemId && a.properties.size() == e.properties.size());
      for (size_t j = 0; j < a.properties.size(); ++j)
        require(a.properties[j].index == e.properties[j].index &&
                a.properties[j].essential == e.properties[j].essential);
    }
    for (size_t i = 0; i < document_.meta.properties.size(); ++i)
      require(parsed.meta.properties[i].type == document_.meta.properties[i].type);

    // Structural agreement alone is insufficient; verify every copied or generated byte.
    verifyBmffLayout(input_, output_, boxes);
  }

  BasicIo& input_;
  BasicIo& output_;
  const HeifDocument& original_;
  HeifDocument document_;
  std::set<uint32_t> removed_;
  std::map<uint32_t, const Bytes*> additions_;
  std::map<uint32_t, const Bytes*> newItems_;
  std::map<uint64_t, std::vector<BmffSpan>> ranges_;
  std::map<uint64_t, const BmffBox*> originalBoxes_;
  std::map<uint64_t, std::vector<const BmffAssociationEntry*>> associations_;
};

}  // namespace

void rewriteHeif(BasicIo& input, BasicIo& output, const HeifDocument& original, const HeifMetadataUpdate& update) {
  Rewrite(input, output, original).run(update);
}

}  // namespace Exiv2::Internal
#endif
