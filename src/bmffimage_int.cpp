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

void require(bool condition) {
  enforce(condition, ErrorCode::kerCorruptedMetadata);
}

void supported(bool condition, std::string_view feature) {
  if (!condition)
    throw Error(ErrorCode::kerErrorMessage, std::string("Unsupported HEIF layout: ") + std::string(feature));
}

void consume(uint64_t& used, uint64_t amount, uint64_t limit) {
  require(used <= limit && amount <= limit - used);
  used += amount;
}

struct Budget {
  const BmffLimits& limits;
  uint64_t boxes{};
  uint64_t extents{};
  uint64_t references{};
  uint64_t associations{};
  uint64_t strings{};
  uint64_t bytes{};
};

//! Every field read is checked against its immediate box, not merely the file.
class Cursor {
 public:
  Cursor(BasicIo& io, BmffSpan range, Budget& budget) :
      io_(io), position_(range.offset), end_(range.offset + range.size), budget_(budget) {
  }

  uint64_t position() const {
    return position_;
  }
  uint64_t remaining() const {
    return end_ - position_;
  }
  uint64_t end() const {
    return end_;
  }
  void advance(uint64_t count) {
    require(count <= remaining());
    position_ += count;
  }
  void read(byte* bytes, size_t size) {
    require(size <= remaining());
    consume(budget_.bytes, size, budget_.limits.maxBytesRead);
    io_.seekOrThrow(static_cast<int64_t>(position_), BasicIo::beg, ErrorCode::kerInputDataReadFailed);
    io_.readOrThrow(bytes, size, ErrorCode::kerInputDataReadFailed);
    if (io_.error())
      throw Error(ErrorCode::kerInputDataReadFailed);
    position_ += size;
  }
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
  void finish() const {
    require(remaining() == 0);
  }

 private:
  BasicIo& io_;
  uint64_t position_;
  uint64_t end_;
  Budget& budget_;
};

enum class Context { file, meta, info, properties, propertyList, references, dataInfo, dataRefs, canon };

class Parser {
 public:
  Parser(BasicIo& io, const BmffLimits& limits) : io_(io), budget_{limits} {
    require(io.isopen());
    document_.fileSize = io.size();
    require(document_.fileSize <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()));
  }

  BmffDocument parse() {
    Cursor input(io_, {0, document_.fileSize}, budget_);
    document_.boxes = children(input, Context::file, 0);
    require(!document_.boxes.empty() && document_.boxes.front().type == bmffType("ftyp"));
    require(has(document_.boxes, bmffType("ftyp")) && has(document_.boxes, bmffType("meta")));
    validateItems();
    return std::move(document_);
  }

 private:
  static bool has(const std::vector<BmffBox>& boxes, uint32_t type) {
    return std::any_of(boxes.begin(), boxes.end(), [type](const auto& box) { return box.type == type; });
  }

  BmffBox header(Cursor& input) {
    consume(budget_.boxes, 1, budget_.limits.maxBoxes);
    require(input.remaining() >= 8);
    BmffBox box;
    box.span.offset = input.position();
    const auto available = input.remaining();
    box.span.size = input.number(4);
    box.type = static_cast<uint32_t>(input.number(4));
    box.headerSize = 8;
    if (box.span.size == 1) {
      box.span.size = input.number(8);
      box.headerSize = 16;
    } else if (box.span.size == 0) {
      // A size-zero box extends to EOF, even when it is nested.
      require(input.end() == document_.fileSize);
      box.span.size = available;
      box.extendsToEnd = true;
    }
    if (box.type == bmffType("uuid")) {
      box.headerSize += 16;
      require(box.span.size >= box.headerSize && box.span.size <= available);
      input.read(box.userType.data(), box.userType.size());
    }
    require(box.span.size >= box.headerSize && box.span.size <= available);
    input.advance(box.span.size - box.headerSize);
    return box;
  }

  BmffFullBox fullBox(BmffBox& box, Cursor& input, uint8_t maxVersion, uint32_t allowedFlags = 0) {
    const auto value = static_cast<uint32_t>(input.number(4));
    BmffFullBox full{static_cast<uint8_t>(value >> 24), value & 0xffffff};
    supported(full.version <= maxVersion, "box version");
    require((full.flags & ~allowedFlags) == 0);
    box.fullBox = full;
    return full;
  }

  BmffItem& item(uint32_t id) {
    require(id != 0);
    const auto found = document_.items.find(id);
    if (found != document_.items.end())
      return found->second;
    require(document_.items.size() < budget_.limits.maxItems);
    return document_.items.try_emplace(id).first->second;
  }

  std::vector<BmffBox> children(Cursor& input, Context context, unsigned depth, uint8_t version = 0) {
    require(depth <= budget_.limits.maxDepth);
    std::vector<BmffBox> boxes;
    std::set<uint32_t> singletons;
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

  void info(BmffBox& box, Cursor& input) {
    const auto full = fullBox(box, input, 3, 1);
    supported(full.version >= 2, "legacy item info entry");
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

  void locations(BmffBox& box, Cursor& input) {
    const auto full = fullBox(box, input, 2);
    auto& format = document_.locationFormat;
    format.box = box.span;
    format.version = full.version;
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

  void reference(const BmffBox& box, Cursor& input, uint8_t version) {
    BmffReference ref;
    ref.box = box.span;
    ref.type = box.type;
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

  void associations(BmffBox& box, Cursor& input) {
    const auto full = fullBox(box, input, 1, 1);
    const unsigned idWidth = full.version == 0 ? 2 : 4;
    const unsigned propertyWidth = (full.flags & 1) ? 2 : 1;
    const auto count = input.number(4);
    consume(budget_.associations, count, budget_.limits.maxAssociations);
    require(count <= input.remaining() / (idWidth + 1));
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

  void validateItems() {
    require(infoIds_ == locationIds_ && !infoIds_.empty());
    require(document_.items.contains(document_.primaryItem));
    for (auto& [id, item] : document_.items) {
      auto& location = item.location;
      for (auto& extent : location.extents) {
        BmffSpan enclosing{0, document_.fileSize};
        if (location.constructionMethod == 1) {
          require(document_.itemData.has_value());
          enclosing = *document_.itemData;
        }
        require(location.baseOffset <= enclosing.size && extent.offset <= enclosing.size - location.baseOffset);
        const auto relative = location.baseOffset + extent.offset;
        require(extent.length <= enclosing.size - relative);
        extent.source = {enclosing.offset + relative, extent.length};
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
    for (const auto& ref : document_.references) {
      require(document_.items.contains(ref.from));
      for (const auto id : ref.to)
        require(document_.items.contains(id));
    }
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

bool padding(uint32_t type) {
  return type == bmffType("free") || type == bmffType("skip");
}

void checkRelocation(const std::vector<BmffBox>& boxes, Context context) {
  static constexpr auto propertyTypes =
      std::array{bmffType("hvcC"), bmffType("av1C"), bmffType("ispe"), bmffType("pixi"), bmffType("colr"),
                 bmffType("irot"), bmffType("imir"), bmffType("clap"), bmffType("pasp"), bmffType("auxC"),
                 bmffType("rloc"), bmffType("clli"), bmffType("mdcv"), bmffType("cclv")};
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
  std::set<uint32_t> describing;
  for (const auto& reference : references) {
    if (reference.type == bmffType("cdsc") &&
        std::find(reference.to.begin(), reference.to.end(), primaryItem) != reference.to.end())
      describing.insert(reference.from);
  }
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
  for (const auto id : document.metadataItems(bmffType("mime")))
    supported(document.items.at(id).info.contentEncoding.empty(), "compressed primary XMP");
  checkRelocation(document.boxes, Context::file);
}

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
