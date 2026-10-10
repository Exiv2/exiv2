// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffitem_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"

#include <algorithm>
#include <limits>
#include <set>
#include <string_view>
#include <utility>

namespace Exiv2::Internal {
namespace {

// Report malformed structure through the common metadata error code.
void require(bool condition) {
  enforce(condition, ErrorCode::kerCorruptedMetadata);
}

// Charge aggregate work without overflowing the counter or its limit.
void consume(uint64_t& used, uint64_t amount, uint64_t limit) {
  require(used <= limit && amount <= limit - used);
  used += amount;
}

// Shared counters bound work across all nested boxes in one parse.
struct Budget {
  const BmffItemLimits& limits;
  uint64_t extents{};
  uint64_t references{};
  uint64_t associations{};
  uint64_t strings{};
};

using Cursor = BmffCursor;

// Interpret child box types according to their immediate container.
enum class Context { meta, info, properties, propertyList, references, dataInfo, dataRefs };

// Build a range-backed item graph, then validate links after all boxes are known.
class Parser {
 public:
  //! @brief Borrow the enclosing reader, item budgets and format policy for one meta box.
  Parser(BmffReader& reader, const BmffItemLimits& limits, const BmffItemPolicy& policy) :
      reader_(reader), budget_{limits}, policy_(policy) {
  }

  //! @brief Decode tables while leaving range and cross-table validation to resolveBmffItems.
  BmffItemModel parse(BmffBox& box, Cursor& fields, unsigned childDepth) {
    fullBox(box, fields, 0);
    box.children = children(fields, Context::meta, childDepth);
    policy_.checkMeta(box);
    return std::move(model_);
  }

 private:
  // Delegate unsupported-syntax diagnostics without putting format names in the parser.
  void supported(bool condition, std::string_view feature) const {
    if (!condition)
      policy_.unsupported(feature);
  }

  // Check whether a container includes a required child type.
  static bool has(const std::vector<BmffBox>& boxes, uint32_t type) {
    return std::any_of(boxes.begin(), boxes.end(), [type](const auto& box) { return box.type == type; });
  }

  // Decode item strings while retaining the adapter's per-string and aggregate budgets.
  std::string string(Cursor& input) {
    std::string result;
    while (true) {
      require(input.remaining() != 0);
      const auto c = static_cast<char>(input.number(1));
      consume(budget_.strings, 1, budget_.limits.maxStringBytes);
      if (c == '\0')
        return result;

      require(result.size() < budget_.limits.maxStringLength);
      result.push_back(c);
    }
  }

  // Decode version/flags while rejecting unsupported versions and reserved bits.
  BmffFullBox fullBox(BmffBox& box, Cursor& input, uint8_t maxVersion, uint32_t allowedFlags = 0) {
    const auto full = decodeBmffFullBox(static_cast<uint32_t>(input.number(4)));
    supported(full.version <= maxVersion, "box version");
    require((full.flags & ~allowedFlags) == 0);
    box.fullBox = full;
    return full;
  }

  // Find or create an item shared by its independently ordered info/location records.
  BmffItem& item(uint32_t id) {
    require(id != 0);
    const auto found = model_.items.find(id);
    if (found != model_.items.end())
      return found->second;

    require(model_.items.size() < budget_.limits.maxItems);
    return model_.items.try_emplace(id).first->second;
  }

  // Traverse children with context-specific decoding and a shared nesting budget.
  std::vector<BmffBox> children(Cursor& input, Context context, unsigned depth, uint8_t version = 0) {
    std::vector<BmffBox> boxes;
    std::set<uint32_t> singletons;

    // Singleton checks apply within each immediate container, not globally.
    reader_.visit(input, depth, [&](BmffBox& box, Cursor& fields) {
      auto unique = [&] { require(singletons.insert(box.type).second); };

      switch (context) {
        case Context::meta:
          if (box.type == bmffType("hdlr")) {
            unique();
            fullBox(box, fields, 0);
            require(fields.number(4) == 0);
            model_.handler = static_cast<uint32_t>(fields.number(4));
            policy_.checkHandler(model_.handler);
            for (unsigned i = 0; i < 3; ++i)
              require(fields.number(4) == 0);
            // The optional handler name is opaque and stays in its original box.
          } else if (box.type == bmffType("pitm")) {
            unique();
            const auto full = fullBox(box, fields, 1);
            model_.primaryItem = static_cast<uint32_t>(fields.number(full.version == 0 ? 2 : 4));
            model_.hasPrimaryItem = true;
            fields.finish();
          } else if (box.type == bmffType("iinf")) {
            unique();
            const auto full = fullBox(box, fields, 2);
            const auto count = fields.number(full.version == 0 ? 2 : 4);
            require(count <= budget_.limits.maxItems && count <= fields.remaining() / 8);
            box.children = children(fields, Context::info, depth + 1);
            require(box.children.size() == count);
          } else if (box.type == bmffType("iloc")) {
            unique();
            locations(box, fields);
          } else if (box.type == bmffType("iref")) {
            unique();
            const auto full = fullBox(box, fields, 1);
            box.children = children(fields, Context::references, depth + 1, full.version);
          } else if (box.type == bmffType("iprp")) {
            unique();
            box.children = children(fields, Context::properties, depth + 1);
            require(has(box.children, bmffType("ipco")));
          } else if (box.type == bmffType("idat")) {
            unique();
            model_.itemData = box.payload();
          } else if (box.type == bmffType("dinf")) {
            unique();
            box.children = children(fields, Context::dataInfo, depth + 1);
            require(has(box.children, bmffType("dref")));
          } else {
            policy_.parseExtension(reader_, box, fields, depth + 1);
          }
          break;
        case Context::info:
          supported(box.type == bmffType("infe"), "non-infe item info entry");
          info(box, fields);
          break;
        case Context::properties:
          if (box.type == bmffType("ipco")) {
            unique();
            box.children = children(fields, Context::propertyList, depth + 1);
            model_.properties = box.children;
          } else if (box.type == bmffType("ipma")) {
            associations(box, fields);
          }
          break;
        case Context::propertyList:
          // Property bodies stay opaque. The write gate checks their relocation support.
          break;
        case Context::references:
          reference(box, fields, version);
          break;
        case Context::dataInfo:
          if (box.type == bmffType("dref")) {
            unique();
            fullBox(box, fields, 0);
            const auto count = fields.number(4);
            require(count <= budget_.limits.maxEntries && count <= fields.remaining() / 8);
            box.children = children(fields, Context::dataRefs, depth + 1);
            require(box.children.size() == count);
          }
          break;
        case Context::dataRefs:
          if (box.type == bmffType("url ")) {
            const auto full = fullBox(box, fields, 0, 1);
            supported(full.flags == 1, "external data reference");
            if (fields.remaining() != 0)
              require(string(fields).empty());
            fields.finish();
          }
          break;
      }

      boxes.push_back(std::move(box));
    });
    return boxes;
  }

  // Decode one item description, retaining its original span and MIME details.
  void info(BmffBox& box, Cursor& input) {
    const auto full = fullBox(box, input, 3, 1);
    supported(full.version >= 2, "legacy item info entry");

    // Item info and location records may arrive in either order; IDs join them later.
    const auto id = static_cast<uint32_t>(input.number(full.version == 2 ? 2 : 4));
    require(infoIds_.insert(id).second);
    auto& info = item(id).info;
    info.box = box.span;
    info.version = full.version;
    info.flags = full.flags;
    info.protectionIndex = static_cast<uint16_t>(input.number(2));
    policy_.checkProtection(info.protectionIndex);
    info.type = static_cast<uint32_t>(input.number(4));
    info.name = string(input);

    // Only MIME and URI items append type-specific strings to the common entry.
    if (info.type == bmffType("mime")) {
      info.contentType = string(input);
      if (input.remaining() != 0)
        info.contentEncoding = string(input);
    } else if (info.type == bmffType("uri ")) {
      info.uriType = string(input);
    }

    input.finish();
    model_.infoOrder.push_back(id);
  }

  // Read iloc field widths and extents; absolute ranges are resolved after parsing.
  void locations(BmffBox& box, Cursor& input) {
    const auto full = fullBox(box, input, 2);
    auto& format = model_.locationFormat;
    format.box = box.span;
    format.version = full.version;

    // The four width nibbles control every subsequent location field.
    const auto widths = input.number(2);
    format.offsetSize = static_cast<uint8_t>(widths >> 12);
    format.lengthSize = static_cast<uint8_t>((widths >> 8) & 15);
    format.baseOffsetSize = static_cast<uint8_t>((widths >> 4) & 15);
    format.indexSize = static_cast<uint8_t>(widths & 15);
    if (full.version == 0)
      require(format.indexSize == 0);
    for (const auto width : {format.offsetSize, format.lengthSize, format.baseOffsetSize, format.indexSize})
      supported(width == 0 || width == 4 || width == 8, "item location field width");
    supported(format.lengthSize != 0, "implicit extent length");

    // Bound the item count using the smallest possible record before allocating.
    const unsigned idWidth = full.version < 2 ? 2 : 4;
    const auto count = input.number(idWidth);
    const auto minimum = idWidth + (full.version == 0 ? 0 : 2) + 2 + format.baseOffsetSize + 2;
    require(count <= budget_.limits.maxItems && count <= input.remaining() / minimum);

    for (uint64_t i = 0; i < count; ++i) {
      const auto id = static_cast<uint32_t>(input.number(idWidth));
      require(locationIds_.insert(id).second);
      auto& location = item(id).location;
      if (full.version != 0) {
        const auto method = input.number(2);
        require((method & 0xfff0) == 0);
        location.constructionMethod = static_cast<uint8_t>(method);
      }
      supported(location.constructionMethod <= 1, "item construction method");
      location.dataReferenceIndex = static_cast<uint16_t>(input.number(2));
      supported(location.dataReferenceIndex == 0, "external item location");
      location.baseOffset = input.number(format.baseOffsetSize);

      // Explicit nonempty extents are required; defer their coordinate conversion.
      const auto extents = input.number(2);
      consume(budget_.extents, extents, budget_.limits.maxExtents);
      require(extents != 0 &&
              extents <= input.remaining() / (format.indexSize + format.offsetSize + format.lengthSize));
      location.extents.reserve(static_cast<size_t>(extents));
      for (uint64_t j = 0; j < extents; ++j) {
        BmffExtent extent;
        extent.index = input.number(format.indexSize);
        extent.offset = input.number(format.offsetSize);
        extent.length = input.number(format.lengthSize);
        supported(extent.length != 0, "implicit extent length");
        location.extents.push_back(extent);
      }
      model_.locationOrder.push_back(id);
    }

    input.finish();
  }

  // Retain a directed item relationship using the enclosing iref ID width.
  void reference(const BmffBox& box, Cursor& input, uint8_t version) {
    BmffReference ref;
    ref.box = box.span;
    ref.type = box.type;

    // The parent iref version controls both source and destination ID widths.
    const unsigned width = version == 0 ? 2 : 4;
    ref.from = static_cast<uint32_t>(input.number(width));
    const auto count = input.number(2);
    consume(budget_.references, count + 1, budget_.limits.maxReferences);
    require(count <= input.remaining() / width);
    ref.to.reserve(static_cast<size_t>(count));
    for (uint64_t i = 0; i < count; ++i)
      ref.to.push_back(static_cast<uint32_t>(input.number(width)));

    input.finish();
    model_.references.push_back(std::move(ref));
  }

  // Read property indices and their essential flags without interpreting properties.
  void associations(BmffBox& box, Cursor& input) {
    const auto full = fullBox(box, input, 1, 1);
    const unsigned idWidth = full.version == 0 ? 2 : 4;
    const unsigned propertyWidth = (full.flags & 1) ? 2 : 1;
    const auto count = input.number(4);
    consume(budget_.associations, count, budget_.limits.maxAssociations);
    require(count <= input.remaining() / (idWidth + 1));

    // An item may occur once per ipma box; property zero is checked after parsing.
    std::set<uint32_t> ids;
    for (uint64_t i = 0; i < count; ++i) {
      BmffAssociationEntry entry;
      entry.box = box.span;
      entry.itemId = static_cast<uint32_t>(input.number(idWidth));
      require(ids.insert(entry.itemId).second);
      const auto n = input.number(1);
      consume(budget_.associations, n, budget_.limits.maxAssociations);
      require(n <= input.remaining() / propertyWidth);
      entry.properties.reserve(static_cast<size_t>(n));
      for (uint64_t j = 0; j < n; ++j) {
        const auto value = input.number(propertyWidth);
        const unsigned essential = propertyWidth == 1 ? 0x80 : 0x8000;
        entry.properties.push_back({static_cast<uint16_t>(value & (essential - 1)), (value & essential) != 0});
      }
      model_.associations.push_back(std::move(entry));
    }

    input.finish();
  }

  BmffReader& reader_;
  Budget budget_;
  const BmffItemPolicy& policy_;
  BmffItemModel model_;
  std::set<uint32_t> infoIds_;
  std::set<uint32_t> locationIds_;
};

}  // namespace

void BmffItemPolicy::unsupported(std::string_view feature) const {
  throw Error(ErrorCode::kerErrorMessage, std::string("Unsupported BMFF item layout: ") + std::string(feature));
}

void BmffItemPolicy::checkHandler(uint32_t) const {
}

void BmffItemPolicy::checkProtection(uint16_t) const {
}

void BmffItemPolicy::checkMeta(const BmffBox&) const {
}

void BmffItemPolicy::checkItems(const BmffItemModel&) const {
}

void BmffItemPolicy::checkExtent(const BmffItemLocation&, const BmffExtent&) const {
}

void BmffItemPolicy::parseExtension(BmffReader&, BmffBox&, BmffCursor&, unsigned) const {
}

BmffItemModel parseBmffItems(BmffReader& reader, BmffBox& box, BmffCursor& fields, unsigned childDepth,
                             const BmffItemLimits& limits, const BmffItemPolicy& policy) {
  return Parser(reader, limits, policy).parse(box, fields, childDepth);
}

void resolveBmffItems(BmffItemModel& model, uint64_t fileSize, const BmffItemPolicy& policy) {
  require(fileSize <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
  require(model.infoOrder.size() == model.items.size() && model.locationOrder.size() == model.items.size());
  policy.checkItems(model);
  if (model.hasPrimaryItem)
    require(model.items.contains(model.primaryItem));

  // Method 0 uses file offsets; method 1 uses offsets from the idat payload.
  for (auto& [id, item] : model.items) {
    auto& location = item.location;
    location.dataSize = 0;
    for (auto& extent : location.extents) {
      BmffSpan enclosing{0, fileSize};
      if (location.constructionMethod == 1) {
        require(model.itemData.has_value());
        enclosing = *model.itemData;
      }

      // Check each relative addition against the containing interval before resolving it.
      require(location.baseOffset <= enclosing.size && extent.offset <= enclosing.size - location.baseOffset);
      const auto relative = location.baseOffset + extent.offset;
      require(extent.length <= enclosing.size - relative);
      extent.source = {enclosing.offset + relative, extent.length};

      policy.checkExtent(location, extent);
      consume(location.dataSize, extent.length, std::numeric_limits<int64_t>::max());
    }
  }

  // Validate graph endpoints after every item has a complete description and location.
  for (const auto& ref : model.references) {
    require(model.items.contains(ref.from));
    for (const auto id : ref.to)
      require(model.items.contains(id));
  }

  // Property indices are one-based; an essential property cannot use the zero sentinel.
  for (const auto& entry : model.associations) {
    require(model.items.contains(entry.itemId));
    for (const auto& property : entry.properties) {
      require(property.index <= model.properties.size());
      require(property.index != 0 || !property.essential);
    }
  }
}

std::vector<uint8_t> readBmffItem(BasicIo& input, const BmffItem& item, uint64_t limit) {
  enforce(item.location.dataSize <= limit && item.location.dataSize <= std::numeric_limits<size_t>::max(),
          ErrorCode::kerErrorMessage, "BMFF item exceeds the allocation limit");

  // Gather the requested item only after bounding its aggregate allocation.
  std::vector<uint8_t> bytes(static_cast<size_t>(item.location.dataSize));
  size_t position = 0;
  for (const auto& extent : item.location.extents) {
    require(extent.source.size <= bytes.size() - position);
    require(extent.source.offset <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) &&
            extent.source.size <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) - extent.source.offset);
    input.seekOrThrow(static_cast<int64_t>(extent.source.offset), BasicIo::beg, ErrorCode::kerInputDataReadFailed);
    input.readOrThrow(bytes.data() + position, static_cast<size_t>(extent.source.size),
                      ErrorCode::kerInputDataReadFailed);
    enforce(!input.error(), ErrorCode::kerInputDataReadFailed);
    position += static_cast<size_t>(extent.source.size);
  }
  require(position == bytes.size());
  return bytes;
}

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
