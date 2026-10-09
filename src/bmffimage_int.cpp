// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffimage_int.hpp"

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

constexpr std::array<uint8_t, 16> canonUuid{0x85, 0xc0, 0xb6, 0x87, 0x82, 0x0f, 0x11, 0xe0,
                                            0x81, 0x11, 0xf4, 0xce, 0x46, 0x2b, 0x6a, 0x48};

// Report malformed structure through the common metadata error code.
void require(bool condition) {
  enforce(condition, ErrorCode::kerCorruptedMetadata);
}

// Distinguish unsupported layouts from corrupt container data.
void supported(bool condition, std::string_view feature) {
  if (!condition)
    throw Error(ErrorCode::kerErrorMessage, std::string("Unsupported HEIF layout: ") + std::string(feature));
}

// Charge aggregate work without overflowing the counter or its limit.
void consume(uint64_t& used, uint64_t amount, uint64_t limit) {
  require(used <= limit && amount <= limit - used);
  used += amount;
}

// Shared counters bound work across all nested boxes in one parse.
struct Budget {
  const BmffLimits& limits;
  uint64_t boxes{};
  uint64_t extents{};
  uint64_t references{};
  uint64_t associations{};
  uint64_t strings{};
  uint64_t bytes{};
};

//! @brief Every field read is checked against its immediate box, not merely the file.
class Cursor {
 public:
  //! @brief Borrow a previously bounded input interval and the enclosing parser budget.
  Cursor(BasicIo& io, BmffSpan range, Budget& budget) :
      io_(io), position_(range.offset), end_(range.offset + range.size), budget_(budget) {
  }

  //! @brief Return the absolute input offset of the next field.
  uint64_t position() const {
    return position_;
  }

  //! @brief Return the bytes still available within this interval.
  uint64_t remaining() const {
    return end_ - position_;
  }

  //! @brief Return the exclusive absolute end of the containing interval.
  uint64_t end() const {
    return end_;
  }

  //! @brief Skip bytes within the interval without reading their contents.
  void advance(uint64_t count) {
    require(count <= remaining());
    position_ += count;
  }

  //! @brief Read an exact field, charging its bytes to the shared structural budget.
  void read(byte* bytes, size_t size) {
    require(size <= remaining());
    consume(budget_.bytes, size, budget_.limits.maxBytesRead);

    io_.seekOrThrow(static_cast<int64_t>(position_), BasicIo::beg, ErrorCode::kerInputDataReadFailed);
    io_.readOrThrow(bytes, size, ErrorCode::kerInputDataReadFailed);
    if (io_.error())
      throw Error(ErrorCode::kerInputDataReadFailed);

    position_ += size;
  }

  //! @brief Decode an unsigned big-endian field; a zero-width field has value zero.
  uint64_t number(unsigned width) {
    require(width <= 8);
    std::array<byte, 8> bytes{};
    if (width != 0)
      read(bytes.data(), width);

    uint64_t result = 0;
    for (unsigned i = 0; i < width; ++i)
      result = (result << 8) | bytes[i];
    return result;
  }

  //! @brief Read a terminated string subject to per-string and aggregate limits.
  std::string string() {
    std::string result;
    while (true) {
      require(remaining() != 0);
      const auto c = static_cast<char>(number(1));
      consume(budget_.strings, 1, budget_.limits.maxStringBytes);
      if (c == '\0')
        return result;

      require(result.size() < budget_.limits.maxStringLength);
      result.push_back(c);
    }
  }

  //! @brief Reject trailing fields where the supported box layout must be exhausted.
  void finish() const {
    require(remaining() == 0);
  }

 private:
  BasicIo& io_;
  uint64_t position_;
  uint64_t end_;
  Budget& budget_;
};

// Interpret child box types according to their immediate container.
enum class Context { file, meta, info, properties, propertyList, references, dataInfo, dataRefs, canon };

// Build a range-backed item graph, then validate links after all boxes are known.
class Parser {
 public:
  //! @brief Borrow an open input and limits for the duration of parsing.
  Parser(BasicIo& io, const BmffLimits& limits) : io_(io), budget_{limits} {
    require(io.isopen());
    document_.fileSize = io.size();
    require(document_.fileSize <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
  }

  //! @brief Parse structure and resolve item ranges without copying encoded image data.
  BmffDocument parse() {
    // Collect structure before checking relationships that may point to later boxes.
    Cursor input(io_, {0, document_.fileSize}, budget_);
    document_.boxes = children(input, Context::file, 0);
    require(!document_.boxes.empty() && document_.boxes.front().type == bmffType("ftyp"));
    require(has(document_.boxes, bmffType("ftyp")) && has(document_.boxes, bmffType("meta")));

    // Cross-check item records only after both iinf and iloc have been read.
    validateItems();
    return std::move(document_);
  }

 private:
  // Check whether a container includes a required child type.
  static bool has(const std::vector<BmffBox>& boxes, uint32_t type) {
    return std::any_of(boxes.begin(), boxes.end(), [type](const auto& box) { return box.type == type; });
  }

  // Validate a box header and advance the parent cursor past the entire box.
  BmffBox header(Cursor& input) {
    consume(budget_.boxes, 1, budget_.limits.maxBoxes);
    require(input.remaining() >= 8);

    BmffBox box;
    box.span.offset = input.position();
    const auto available = input.remaining();
    box.span.size = input.number(4);
    box.type = static_cast<uint32_t>(input.number(4));
    box.headerSize = 8;

    // Resolve extended sizes and EOF-sized boxes before bounding the payload.
    if (box.span.size == 1) {
      box.span.size = input.number(8);
      box.headerSize = 16;
    } else if (box.span.size == 0) {
      // A size-zero box extends to EOF, even when it is nested.
      require(input.end() == document_.fileSize);
      box.span.size = available;
      box.extendsToEnd = true;
    }

    // UUID user types belong to the header, so exclude them from the payload span.
    if (box.type == bmffType("uuid")) {
      box.headerSize += 16;
      require(box.span.size >= box.headerSize && box.span.size <= available);
      input.read(box.userType.data(), box.userType.size());
    }

    // The child cursor will read fields independently of this parent position.
    require(box.span.size >= box.headerSize && box.span.size <= available);
    input.advance(box.span.size - box.headerSize);
    return box;
  }

  // Decode version/flags while rejecting unsupported versions and reserved bits.
  BmffFullBox fullBox(BmffBox& box, Cursor& input, uint8_t maxVersion, uint32_t allowedFlags = 0) {
    const auto value = static_cast<uint32_t>(input.number(4));
    BmffFullBox full{static_cast<uint8_t>(value >> 24), value & 0xffffff};
    supported(full.version <= maxVersion, "box version");
    require((full.flags & ~allowedFlags) == 0);
    box.fullBox = full;
    return full;
  }

  // Find or create an item shared by its independently ordered info/location records.
  BmffItem& item(uint32_t id) {
    require(id != 0);
    const auto found = document_.items.find(id);
    if (found != document_.items.end())
      return found->second;

    require(document_.items.size() < budget_.limits.maxItems);
    return document_.items.try_emplace(id).first->second;
  }

  // Traverse children with context-specific decoding and a shared nesting budget.
  std::vector<BmffBox> children(Cursor& input, Context context, unsigned depth, uint8_t version = 0) {
    require(depth <= budget_.limits.maxDepth);
    std::vector<BmffBox> boxes;
    std::set<uint32_t> singletons;

    // Singleton checks apply within each immediate container, not globally.
    while (input.remaining() != 0) {
      auto box = header(input);
      Cursor fields(io_, box.payload(), budget_);
      auto unique = [&] { require(singletons.insert(box.type).second); };

      switch (context) {
        case Context::file:
          if (box.type == bmffType("ftyp")) {
            unique();
            require(fields.remaining() >= 8 && fields.remaining() % 4 == 0);
            document_.majorBrand = static_cast<uint32_t>(fields.number(4));
            document_.minorVersion = static_cast<uint32_t>(fields.number(4));
            require(fields.remaining() / 4 <= budget_.limits.maxBoxes);
            while (fields.remaining() != 0)
              document_.compatibleBrands.push_back(static_cast<uint32_t>(fields.number(4)));
          } else if (box.type == bmffType("meta")) {
            unique();
            fullBox(box, fields, 0);
            box.children = children(fields, Context::meta, depth + 1);

            // A picture meta box must provide the tables needed to resolve its primary item.
            for (const auto required :
                 {bmffType("hdlr"), bmffType("pitm"), bmffType("iinf"), bmffType("iloc"), bmffType("iprp")})
              require(has(box.children, required));
          } else if (box.type == bmffType("mdat")) {
            document_.mediaData.push_back(box.payload());
          }
          break;
        case Context::meta:
          if (box.type == bmffType("hdlr")) {
            unique();
            fullBox(box, fields, 0);
            require(fields.number(4) == 0);
            supported(fields.number(4) == bmffType("pict"), "non-picture handler");
            for (unsigned i = 0; i < 3; ++i)
              require(fields.number(4) == 0);
            // The optional handler name is opaque and stays in its original box.
          } else if (box.type == bmffType("pitm")) {
            unique();
            const auto full = fullBox(box, fields, 1);
            document_.primaryItem = static_cast<uint32_t>(fields.number(full.version == 0 ? 2 : 4));
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
            document_.itemData = box.payload();
          } else if (box.type == bmffType("dinf")) {
            unique();
            box.children = children(fields, Context::dataInfo, depth + 1);
            require(has(box.children, bmffType("dref")));
          } else if (box.type == bmffType("uuid") && box.userType == canonUuid) {
            box.children = children(fields, Context::canon, depth + 1);
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
            document_.properties = box.children;
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
            require(count <= budget_.limits.maxBoxes && count <= fields.remaining() / 8);
            box.children = children(fields, Context::dataRefs, depth + 1);
            require(box.children.size() == count);
          }
          break;
        case Context::dataRefs:
          if (box.type == bmffType("url ")) {
            const auto full = fullBox(box, fields, 0, 1);
            supported(full.flags == 1, "external data reference");
            if (fields.remaining() != 0)
              require(fields.string().empty());
            fields.finish();
          }
          break;
        case Context::canon:
          unique();
          break;
      }

      boxes.push_back(std::move(box));
    }
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
    supported(info.protectionIndex == 0, "protected item");
    info.type = static_cast<uint32_t>(input.number(4));
    info.name = input.string();

    // Only MIME and URI items append type-specific strings to the common entry.
    if (info.type == bmffType("mime")) {
      info.contentType = input.string();
      if (input.remaining() != 0)
        info.contentEncoding = input.string();
    } else if (info.type == bmffType("uri ")) {
      info.uriType = input.string();
    }

    input.finish();
    document_.infoOrder.push_back(id);
  }

  // Read iloc field widths and extents; absolute ranges are resolved after parsing.
  void locations(BmffBox& box, Cursor& input) {
    const auto full = fullBox(box, input, 2);
    auto& format = document_.locationFormat;
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
      document_.locationOrder.push_back(id);
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
    document_.references.push_back(std::move(ref));
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
      document_.associations.push_back(std::move(entry));
    }

    input.finish();
  }

  // Resolve addressing and reject incomplete items, invalid ranges, and dangling links.
  void validateItems() {
    require(infoIds_ == locationIds_ && !infoIds_.empty());
    require(document_.items.contains(document_.primaryItem));

    // Method 0 uses file offsets; method 1 uses offsets from the idat payload.
    for (auto& [id, item] : document_.items) {
      auto& location = item.location;
      for (auto& extent : location.extents) {
        BmffSpan enclosing{0, document_.fileSize};
        if (location.constructionMethod == 1) {
          require(document_.itemData.has_value());
          enclosing = *document_.itemData;
        }

        // Check each relative addition against the containing interval before resolving it.
        require(location.baseOffset <= enclosing.size && extent.offset <= enclosing.size - location.baseOffset);
        const auto relative = location.baseOffset + extent.offset;
        require(extent.length <= enclosing.size - relative);
        extent.source = {enclosing.offset + relative, extent.length};

        // File-relative extents must lie wholly within a single mdat payload.
        if (location.constructionMethod == 0) {
          auto media = std::upper_bound(document_.mediaData.begin(), document_.mediaData.end(), extent.source.offset,
                                        [](uint64_t offset, const BmffSpan& range) { return offset < range.offset; });
          require(media != document_.mediaData.begin());
          --media;
          const auto offset = extent.source.offset - media->offset;
          require(offset <= media->size && extent.length <= media->size - offset);
        }
        consume(location.dataSize, extent.length, std::numeric_limits<int64_t>::max());
      }
    }

    // Validate graph endpoints after every item has a complete description and location.
    for (const auto& ref : document_.references) {
      require(document_.items.contains(ref.from));
      for (const auto id : ref.to)
        require(document_.items.contains(id));
    }

    // Property indices are one-based; an essential property cannot use the zero sentinel.
    for (const auto& entry : document_.associations) {
      require(document_.items.contains(entry.itemId));
      for (const auto& property : entry.properties) {
        require(property.index <= document_.properties.size());
        require(property.index != 0 || !property.essential);
      }
    }
  }

  BasicIo& io_;
  Budget budget_;
  BmffDocument document_;
  std::set<uint32_t> infoIds_;
  std::set<uint32_t> locationIds_;
};

// Recognize padding that can be discarded or regenerated during rewriting.
bool padding(uint32_t type) {
  return type == bmffType("free") || type == bmffType("skip");
}

// Reject opaque structures unless their payloads are known to be safe to relocate.
void checkRelocation(const std::vector<BmffBox>& boxes, Context context) {
  static constexpr auto propertyTypes =
      std::array{bmffType("hvcC"), bmffType("av1C"), bmffType("ispe"), bmffType("pixi"), bmffType("colr"),
                 bmffType("irot"), bmffType("imir"), bmffType("clap"), bmffType("pasp"), bmffType("auxC"),
                 bmffType("rloc"), bmffType("clli"), bmffType("mdcv"), bmffType("cclv")};

  // Reading preserves unknown boxes, but moving them requires an explicit safe case.
  for (const auto& box : boxes) {
    if (padding(box.type))
      continue;
    switch (context) {
      case Context::file:
        if (box.type == bmffType("meta"))
          checkRelocation(box.children, Context::meta);
        else
          supported(box.type == bmffType("ftyp") || box.type == bmffType("mdat"), "unmodeled top-level box");
        break;
      case Context::meta:
        switch (box.type) {
          case bmffType("hdlr"):
          case bmffType("pitm"):
          case bmffType("iloc"):
          case bmffType("idat"):
          case bmffType("iinf"):
          case bmffType("iref"):
            break;
          case bmffType("iprp"):
            checkRelocation(box.children, Context::properties);
            break;
          case bmffType("dinf"):
            checkRelocation(box.children, Context::dataInfo);
            break;
          case bmffType("uuid"):
            supported(
                box.userType == canonUuid && box.children.size() == 1 && box.children.front().type == bmffType("CNCV"),
                "unmodeled UUID");
            break;
          default:
            supported(false, "unmodeled metadata box");
        }
        break;
      case Context::properties:
        if (box.type == bmffType("ipco"))
          checkRelocation(box.children, Context::propertyList);
        else
          supported(box.type == bmffType("ipma"), "unmodeled property container");
        break;
      case Context::propertyList:
        supported(std::find(propertyTypes.begin(), propertyTypes.end(), box.type) != propertyTypes.end(),
                  "unmodeled item property");
        break;
      case Context::dataInfo:
        supported(box.type == bmffType("dref"), "unmodeled data information");
        checkRelocation(box.children, Context::dataRefs);
        break;
      case Context::dataRefs:
        supported(box.type == bmffType("url "), "unmodeled data reference");
        break;
      default:
        supported(false, "unmodeled container");
    }
  }
}

}  // namespace

BmffSpan BmffBox::payload() const {
  require(headerSize <= span.size && span.size <= std::numeric_limits<uint64_t>::max() - span.offset);
  return {span.offset + headerSize, span.size - headerSize};
}

std::vector<uint32_t> BmffDocument::metadataItems(uint32_t type) const {
  // cdsc points from the metadata item to the image it describes.
  std::set<uint32_t> describing;
  for (const auto& reference : references) {
    if (reference.type == bmffType("cdsc") &&
        std::find(reference.to.begin(), reference.to.end(), primaryItem) != reference.to.end())
      describing.insert(reference.from);
  }

  // Keep iinf order so later metadata items have deterministic merge precedence.
  std::vector<uint32_t> result;
  for (const auto id : infoOrder) {
    const auto& info = items.at(id).info;
    if (describing.contains(id) && info.type == type &&
        (type != bmffType("mime") || info.contentType == "application/rdf+xml"))
      result.push_back(id);
  }

  return result;
}

BmffDocument parseBmff(BasicIo& io, const BmffLimits& limits) {
  return Parser(io, limits).parse();
}

void enforceHeifWriteSupport(const BmffDocument& document) {
  supported(document.majorBrand == bmffType("heic") || document.majorBrand == bmffType("heix") ||
                document.majorBrand == bmffType("mif1"),
            "file brand");

  // A generic mif1 brand can accompany an AVIF or another deferred codec.
  supported(std::find(document.compatibleBrands.begin(), document.compatibleBrands.end(), bmffType("avif")) ==
                    document.compatibleBrands.end() &&
                std::find(document.compatibleBrands.begin(), document.compatibleBrands.end(), bmffType("avis")) ==
                    document.compatibleBrands.end(),
            "AVIF writing is deferred");

  const auto primaryType = document.items.at(document.primaryItem).info.type;
  supported(primaryType == bmffType("hvc1") || primaryType == bmffType("grid") || primaryType == bmffType("iden") ||
                primaryType == bmffType("iovl"),
            "primary image type");

  // Recognize near-matching XMP MIME labels so they cannot evade removal checks.
  for (const auto& reference : document.references) {
    if (reference.type != bmffType("cdsc") ||
        std::find(reference.to.begin(), reference.to.end(), document.primaryItem) == reference.to.end())
      continue;
    const auto& info = document.items.at(reference.from).info;
    if (info.type != bmffType("mime"))
      continue;
    auto type = info.contentType.substr(0, info.contentType.find(';'));
    std::transform(type.begin(), type.end(), type.begin(),
                   [](unsigned char c) { return static_cast<char>(c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c); });
    const auto first = type.find_first_not_of(" \t");
    const auto last = type.find_last_not_of(" \t");
    if (first != std::string::npos)
      type = type.substr(first, last - first + 1);
    if (type == "application/rdf+xml") {
      supported(info.contentType == "application/rdf+xml", "ambiguous primary XMP MIME type");
      supported(info.contentEncoding.empty(), "compressed primary XMP");
    }
  }

  // Finally reject any retained opaque box whose internal offsets are unknown.
  checkRelocation(document.boxes, Context::file);
}

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
