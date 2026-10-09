// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffwrite_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <set>
#include <string_view>

namespace Exiv2::Internal {
namespace {
using Bytes = std::vector<uint8_t>;
constexpr uint64_t maxPosition = std::numeric_limits<int64_t>::max();
constexpr uint64_t max32 = std::numeric_limits<uint32_t>::max();
constexpr uint32_t max16 = std::numeric_limits<uint16_t>::max();
constexpr size_t copyBufferSize = 64 * 1024;

void require(bool condition) {
  enforce(condition, ErrorCode::kerCorruptedMetadata);
}

void supported(bool condition, std::string_view reason) {
  if (!condition)
    throw Error(ErrorCode::kerErrorMessage, std::string("Unsupported HEIF edit: ") + std::string(reason));
}

uint64_t add(uint64_t a, uint64_t b) {
  require(a <= maxPosition && b <= maxPosition - a);
  return a + b;
}

uint64_t end(BmffSpan span) {
  return add(span.offset, span.size);
}

void number(Bytes& bytes, uint64_t value, unsigned width) {
  require(width <= 8 && (width == 8 || value < (uint64_t{1} << (width * 8))));
  require(bytes.size() <= BmffLimits{}.maxBytesRead - width);
  for (unsigned i = width; i != 0; --i)
    bytes.push_back(static_cast<uint8_t>(value >> ((i - 1) * 8)));
}

void string(Bytes& bytes, std::string_view value) {
  require(value.size() < BmffLimits{}.maxBytesRead && bytes.size() < BmffLimits{}.maxBytesRead - value.size());
  bytes.insert(bytes.end(), value.begin(), value.end());
  bytes.push_back(0);
}

Bytes full(uint8_t version, uint32_t flags = 0) {
  Bytes bytes;
  number(bytes, (uint32_t{version} << 24) | flags, 4);
  return bytes;
}

bool padding(uint32_t type) {
  return type == bmffType("free") || type == bmffType("skip");
}

void read(BasicIo& input, uint64_t offset, byte* data, size_t size) {
  require(offset <= maxPosition && size <= maxPosition - offset);
  input.seekOrThrow(static_cast<int64_t>(offset), BasicIo::beg, ErrorCode::kerInputDataReadFailed);
  input.readOrThrow(data, size, ErrorCode::kerInputDataReadFailed);
  enforce(!input.error(), ErrorCode::kerInputDataReadFailed);
}

void write(BasicIo& output, const byte* data, size_t size) {
  if (size != 0)
    enforce(output.write(data, size) == size && !output.error(), ErrorCode::kerImageWriteFailed);
}

struct Chunk {
  BmffSpan source;
  const Bytes* bytes{};
  uint32_t item{};
  uint64_t position{};

  uint64_t size() const {
    return bytes ? bytes->size() : source.size;
  }
};

struct Box {
  uint32_t type{};
  bool extended{};
  std::array<uint8_t, 16> uuid{};
  Bytes prefix;
  std::vector<Box> children;
  std::vector<Chunk> chunks;
  uint64_t position{};
  uint64_t size{};
  uint8_t header{};
};

struct Relocation {
  BmffSpan source;
  uint64_t destination{};
};

void measure(Box& box) {
  uint64_t payload = box.prefix.size();
  for (auto& child : box.children) {
    measure(child);
    payload = add(payload, child.size);
  }
  for (const auto& chunk : box.chunks)
    payload = add(payload, chunk.size());
  const unsigned uuidSize = box.type == bmffType("uuid") ? 16 : 0;
  box.extended |= add(payload, 8 + uuidSize) > max32;
  box.header = static_cast<uint8_t>((box.extended ? 16 : 8) + uuidSize);
  box.size = add(box.header, payload);
}

//! Unite only ranges from the same source payload. Original item boundaries stay in iloc.
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

class Rewrite {
 public:
  Rewrite(BasicIo& input, BasicIo& output, const BmffDocument& original) :
      input_(input), output_(output), original_(original), document_(original) {
  }

  void run(const BmffMetadataUpdate& update) {
    require(&input_ != &output_ && input_.isopen() && output_.isopen() && output_.size() == 0);
    require(input_.size() == original_.fileSize);
    enforceHeifWriteSupport(original_);
    select(bmffType("Exif"), update.exif);
    select(bmffType("mime"), update.xmp);
    validateReferences();
    collectRanges();
    applyUpdates();
    for (const auto& entry : document_.associations)
      associations_[entry.box.offset].push_back(&entry);
    indexBoxes(original_.boxes);
    normalizeLocations();

    // Only offset width can change after locating the output; it promotes 4 -> 8 once.
    for (unsigned pass = 0; pass < 2; ++pass) {
      auto boxes = build();
      const auto size = locate(boxes);
      if (relocate())
        continue;
      boxes = build();
      require(locate(boxes) == size);
      output_.seekOrThrow(0, BasicIo::beg, ErrorCode::kerImageWriteFailed);
      for (const auto& box : boxes)
        emit(box);
      require(output_.size() == size);
      verify(boxes);
      return;
    }
    throw Error(ErrorCode::kerCorruptedMetadata);
  }

 private:
  void select(uint32_t type, const std::optional<Bytes>& value) {
    if (!value)
      return;
    supported(value->size() <= bmffMetadataLimit, "metadata exceeds the allocation limit");
    const auto ids = original_.metadataItems(type);
    removed_.insert(ids.begin(), ids.end());
    if (!value->empty())
      additions_.emplace(type, &*value);
  }

  void validateReferences() const {
    for (const auto& ref : original_.references) {
      if (removed_.contains(ref.from)) {
        supported(ref.type == bmffType("cdsc") &&
                      std::all_of(ref.to.begin(), ref.to.end(), [&](auto id) { return id == original_.primaryItem; }),
                  "selected metadata is shared or has other relationships");
      } else {
        for (const auto id : ref.to)
          supported(!removed_.contains(id), "selected metadata is referenced by another item");
      }
    }
  }

  uint64_t container(const BmffItem& item, BmffSpan span) const {
    if (item.location.constructionMethod == 1) {
      require(original_.itemData.has_value());
      const auto data = *original_.itemData;
      require(span.offset >= data.offset && end(span) <= end(data));
      return data.offset;
    }
    auto found = std::upper_bound(original_.mediaData.begin(), original_.mediaData.end(), span.offset,
                                  [](uint64_t offset, auto range) { return offset < range.offset; });
    require(found != original_.mediaData.begin());
    --found;
    require(span.offset >= found->offset && end(span) <= end(*found));
    return found->offset;
  }

  void collectRanges() {
    std::vector<BmffSpan> discarded;
    for (const auto& [id, item] : original_.items) {
      for (const auto& extent : item.location.extents) {
        if (removed_.contains(id))
          discarded.push_back(extent.source);
        else
          ranges_[container(item, extent.source)].push_back(extent.source);
      }
    }
    std::vector<BmffSpan> retained;
    for (auto& [offset, spans] : ranges_) {
      mergeRanges(spans);
      retained.insert(retained.end(), spans.begin(), spans.end());
    }
    mergeRanges(discarded);
    size_t next = 0;
    for (const auto span : discarded) {
      while (next < retained.size() && end(retained[next]) <= span.offset)
        ++next;
      supported(next == retained.size() || retained[next].offset >= end(span),
                "removed metadata overlaps a retained item");
    }
  }

  uint32_t allocateId() const {
    require(!document_.items.empty());
    const auto greatest = document_.items.rbegin()->first;
    if (greatest != max32)
      return greatest + 1;
    uint32_t id = 1;
    for (const auto& [used, item] : document_.items) {
      if (used != id)
        return id;
      supported(id != max32, "no unused item ID");
      ++id;
    }
    return id;
  }

  void applyUpdates() {
    for (const auto id : removed_)
      document_.items.erase(id);
    std::erase_if(document_.infoOrder, [&](auto id) { return removed_.contains(id); });
    std::erase_if(document_.locationOrder, [&](auto id) { return removed_.contains(id); });
    std::erase_if(document_.references, [&](const auto& ref) { return removed_.contains(ref.from); });
    std::erase_if(document_.associations, [&](const auto& entry) { return removed_.contains(entry.itemId); });
    for (const auto& [type, bytes] : additions_) {
      require(document_.items.size() < BmffLimits{}.maxItems);
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
      document_.items.emplace(id, std::move(item));
      document_.infoOrder.push_back(id);
      document_.locationOrder.push_back(id);
      document_.references.push_back({{}, bmffType("cdsc"), id, {document_.primaryItem}});
      newItems_.emplace(id, bytes);
    }
  }

  void indexBoxes(const std::vector<BmffBox>& boxes) {
    for (const auto& box : boxes) {
      originalBoxes_.emplace(box.span.offset, &box);
      indexBoxes(box.children);
    }
  }

  void normalizeLocations() {
    auto& format = document_.locationFormat;
    format.offsetSize = 4;
    format.lengthSize = 4;
    format.baseOffsetSize = 0;
    if (document_.items.rbegin()->first > max16 || document_.items.size() > max16)
      format.version = 2;
    for (auto& [id, item] : document_.items) {
      item.location.baseOffset = 0;
      if (item.location.constructionMethod == 1)
        format.version = std::max<uint8_t>(format.version, 1);
      for (const auto& extent : item.location.extents)
        if (extent.length > max32)
          format.lengthSize = 8;
    }
  }

  Box itemInfo(uint32_t id) const {
    const auto& info = document_.items.at(id).info;
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

  Bytes locations() const {
    const auto& format = document_.locationFormat;
    auto bytes = full(format.version);
    number(bytes, (format.offsetSize << 4) | format.lengthSize, 1);
    number(bytes, format.indexSize, 1);
    const unsigned width = format.version == 2 ? 4 : 2;
    number(bytes, document_.items.size(), width);
    for (const auto id : document_.locationOrder) {
      const auto& loc = document_.items.at(id).location;
      number(bytes, id, width);
      if (format.version)
        number(bytes, loc.constructionMethod, 2);
      number(bytes, 0, 2);
      number(bytes, loc.extents.size(), 2);
      for (const auto& extent : loc.extents) {
        if (format.version && format.indexSize)
          number(bytes, extent.index, format.indexSize);
        // The first layout pass measures fields whose relocated offsets are not known yet.
        number(bytes, format.offsetSize == 4 && extent.offset > max32 ? 0 : extent.offset, format.offsetSize);
        number(bytes, extent.length, format.lengthSize);
      }
    }
    return bytes;
  }

  Box references(const BmffBox* original = nullptr) const {
    Box box;
    box.type = bmffType("iref");
    box.extended = original && original->headerSize == 16;
    uint8_t version = original ? original->fullBox->version : 0;
    if (document_.items.rbegin()->first > max16)
      version = 1;
    box.prefix = full(version);
    for (const auto& ref : document_.references) {
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

  static Box opaque(const BmffBox& original) {
    Box box;
    box.type = original.type;
    box.extended = original.headerSize == (original.type == bmffType("uuid") ? 32 : 16);
    box.uuid = original.userType;
    box.chunks.push_back({original.payload()});
    return box;
  }

  std::optional<Box> rebuild(const BmffBox& original, bool property = false) const {
    auto box = opaque(original);
    if (padding(box.type)) {
      if (!property)
        return {};
      box.chunks.clear();  // Keep indexed property slots; discard their padding bytes.
    } else if (box.type == bmffType("mdat") || box.type == bmffType("idat")) {
      box.chunks.clear();
      const auto found = ranges_.find(original.payload().offset);
      if (found == ranges_.end())
        return {};
      for (const auto range : found->second)
        box.chunks.push_back({range});
    } else if (box.type == bmffType("iloc")) {
      box.chunks.clear();
      box.prefix = locations();
    } else if (box.type == bmffType("iinf")) {
      box.chunks.clear();
      auto version = original.fullBox->version;
      if (document_.items.size() > max16)
        version = std::max<uint8_t>(version, 1);
      box.prefix = full(version);
      number(box.prefix, document_.items.size(), version ? 4 : 2);
      for (const auto id : document_.infoOrder)
        box.children.push_back(itemInfo(id));
    } else if (box.type == bmffType("iref")) {
      box = references(&original);
    } else if (box.type == bmffType("ipma")) {
      box.chunks.clear();
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
      box.chunks.clear();
      if (original.fullBox)
        box.prefix = full(original.fullBox->version, original.fullBox->flags);
      bool hasReferences = false;
      for (const auto& child : original.children) {
        hasReferences |= child.type == bmffType("iref");
        if (auto rebuilt = rebuild(child, box.type == bmffType("ipco")))
          box.children.push_back(std::move(*rebuilt));
      }
      if (box.type == bmffType("meta") && !hasReferences && !document_.references.empty())
        box.children.push_back(references());
      if (box.type == bmffType("dref"))
        number(box.prefix, box.children.size(), 4);
    }
    return box;
  }

  std::vector<Box> build() const {
    std::vector<Box> boxes;
    for (const auto& box : original_.boxes)
      if (auto rebuilt = rebuild(box))
        boxes.push_back(std::move(*rebuilt));
    if (!newItems_.empty()) {
      Box box;
      box.type = bmffType("mdat");
      for (const auto& [id, bytes] : newItems_)
        box.chunks.push_back({{}, bytes, id});
      boxes.push_back(std::move(box));
    }
    return boxes;
  }

  uint64_t locate(Box& box, uint64_t offset) {
    box.position = offset;
    auto position = add(add(offset, box.header), box.prefix.size());
    if (box.type == bmffType("idat"))
      idatPosition_ = add(offset, box.header);
    for (auto& child : box.children)
      position = locate(child, position);
    for (auto& chunk : box.chunks) {
      chunk.position = position;
      if (chunk.bytes)
        newPositions_.emplace(chunk.item, position);
      else if (box.type == bmffType("mdat") || box.type == bmffType("idat"))
        relocations_.push_back({chunk.source, position});
      position = add(position, chunk.size());
    }
    require(position == add(offset, box.size));
    return position;
  }

  uint64_t locate(std::vector<Box>& boxes) {
    relocations_.clear();
    newPositions_.clear();
    idatPosition_ = 0;
    uint64_t offset = 0;
    for (auto& box : boxes) {
      measure(box);
      offset = locate(box, offset);
    }
    std::sort(relocations_.begin(), relocations_.end(),
              [](const auto& a, const auto& b) { return a.source.offset < b.source.offset; });
    return offset;
  }

  bool relocate() {
    bool promoted = false;
    for (auto& [id, item] : document_.items) {
      auto& location = item.location;
      for (auto& extent : location.extents) {
        uint64_t position = 0;
        if (newItems_.contains(id)) {
          position = newPositions_.at(id);
        } else {
          auto found = std::upper_bound(relocations_.begin(), relocations_.end(), extent.source.offset,
                                        [](auto offset, const auto& map) { return offset < map.source.offset; });
          require(found != relocations_.begin());
          --found;
          require(extent.source.offset >= found->source.offset && end(extent.source) <= end(found->source));
          position = add(found->destination, extent.source.offset - found->source.offset);
        }
        const auto base = location.constructionMethod == 1 ? idatPosition_ : 0;
        require(position >= base);
        extent.offset = position - base;
        if (extent.offset > max32 && document_.locationFormat.offsetSize != 8) {
          document_.locationFormat.offsetSize = 8;
          promoted = true;
        }
      }
    }
    return promoted;
  }

  void emit(const Box& box) {
    require(output_.tell() == box.position);
    Bytes header;
    number(header, box.extended ? 1 : box.size, 4);
    number(header, box.type, 4);
    if (box.extended)
      number(header, box.size, 8);
    if (box.type == bmffType("uuid"))
      header.insert(header.end(), box.uuid.begin(), box.uuid.end());
    write(output_, header.data(), header.size());
    write(output_, box.prefix.data(), box.prefix.size());
    for (const auto& child : box.children)
      emit(child);
    auto& buffer = copyBuffer_;
    for (const auto& chunk : box.chunks) {
      require(output_.tell() == chunk.position);
      for (uint64_t done = 0; done < chunk.size();) {
        const auto count = static_cast<size_t>(std::min<uint64_t>(buffer.size(), chunk.size() - done));
        if (chunk.bytes) {
          write(output_, chunk.bytes->data() + done, count);
        } else {
          read(input_, add(chunk.source.offset, done), buffer.data(), count);
          write(output_, buffer.data(), count);
        }
        done += count;
      }
    }
  }

  void verifyBytes(const Box& box) {
    for (const auto& child : box.children)
      verifyBytes(child);
    auto& a = copyBuffer_;
    auto& b = verifyBuffer_;
    for (const auto& chunk : box.chunks) {
      for (uint64_t done = 0; done < chunk.size();) {
        const auto count = static_cast<size_t>(std::min<uint64_t>(a.size(), chunk.size() - done));
        read(output_, add(chunk.position, done), b.data(), count);
        if (chunk.bytes) {
          require(std::equal(b.begin(), b.begin() + count, chunk.bytes->begin() + done));
        } else {
          read(input_, add(chunk.source.offset, done), a.data(), count);
          require(std::equal(a.begin(), a.begin() + count, b.begin()));
        }
        done += count;
      }
    }
  }

  void verify(const std::vector<Box>& boxes) {
    const auto parsed = parseBmff(output_);
    enforceHeifWriteSupport(parsed);
    require(parsed.majorBrand == document_.majorBrand && parsed.minorVersion == document_.minorVersion &&
            parsed.compatibleBrands == document_.compatibleBrands && parsed.primaryItem == document_.primaryItem &&
            parsed.infoOrder == document_.infoOrder && parsed.locationOrder == document_.locationOrder &&
            parsed.items.size() == document_.items.size());
    for (const auto& [id, item] : document_.items) {
      const auto& actual = parsed.items.at(id);
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
    require(parsed.references.size() == document_.references.size() &&
            parsed.associations.size() == document_.associations.size() &&
            parsed.properties.size() == document_.properties.size());
    for (size_t i = 0; i < document_.references.size(); ++i) {
      const auto& a = parsed.references[i];
      const auto& e = document_.references[i];
      require(a.type == e.type && a.from == e.from && a.to == e.to);
    }
    for (size_t i = 0; i < document_.associations.size(); ++i) {
      const auto& a = parsed.associations[i];
      const auto& e = document_.associations[i];
      require(a.itemId == e.itemId && a.properties.size() == e.properties.size());
      for (size_t j = 0; j < a.properties.size(); ++j)
        require(a.properties[j].index == e.properties[j].index &&
                a.properties[j].essential == e.properties[j].essential);
    }
    for (size_t i = 0; i < document_.properties.size(); ++i)
      require(parsed.properties[i].type == document_.properties[i].type);
    for (const auto& box : boxes)
      verifyBytes(box);
  }

  BasicIo& input_;
  BasicIo& output_;
  const BmffDocument& original_;
  BmffDocument document_;
  std::set<uint32_t> removed_;
  std::map<uint32_t, const Bytes*> additions_;
  std::map<uint32_t, const Bytes*> newItems_;
  std::map<uint64_t, std::vector<BmffSpan>> ranges_;
  std::map<uint64_t, const BmffBox*> originalBoxes_;
  std::vector<Relocation> relocations_;
  std::map<uint64_t, std::vector<const BmffAssociationEntry*>> associations_;
  std::array<byte, copyBufferSize> copyBuffer_{};
  std::array<byte, copyBufferSize> verifyBuffer_{};
  std::map<uint32_t, uint64_t> newPositions_;
  uint64_t idatPosition_{};
};

}  // namespace

std::vector<uint8_t> readBmffItem(BasicIo& input, const BmffItem& item, uint64_t limit) {
  supported(item.location.dataSize <= limit && item.location.dataSize <= std::numeric_limits<size_t>::max(),
            "metadata exceeds the allocation limit");
  Bytes bytes(static_cast<size_t>(item.location.dataSize));
  size_t position = 0;
  for (const auto& extent : item.location.extents) {
    require(extent.source.size <= bytes.size() - position);
    read(input, extent.source.offset, bytes.data() + position, static_cast<size_t>(extent.source.size));
    position += static_cast<size_t>(extent.source.size);
  }
  require(position == bytes.size());
  return bytes;
}

void rewriteBmff(BasicIo& input, BasicIo& output, const BmffDocument& original, const BmffMetadataUpdate& update) {
  Rewrite(input, output, original).run(update);
}

}  // namespace Exiv2::Internal
#endif
