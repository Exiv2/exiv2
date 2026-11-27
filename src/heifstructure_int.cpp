// SPDX-License-Identifier: GPL-2.0-or-later

#include "heifstructure_int.hpp"

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

// Test for a required child without imposing its meaning on generic traversal.
bool has(const std::vector<BmffBox>& boxes, uint32_t type) {
  return std::any_of(boxes.begin(), boxes.end(), [type](const auto& box) { return box.type == type; });
}

// Keep native HEIF admission and Canon extensions separate from standard item mechanics.
class HeifItemPolicy final : public BmffItemPolicy {
 public:
  //! @brief Borrow the document while framing fills its media ownership ranges.
  explicit HeifItemPolicy(const HeifDocument& document) : document_(document) {
  }

  //! @brief Preserve the HEIF diagnostic for unsupported standard item syntax.
  [[noreturn]] void unsupported(std::string_view feature) const override {
    throw Error(ErrorCode::kerErrorMessage, std::string("Unsupported HEIF layout: ") + std::string(feature));
  }

  //! @brief Admit only picture handlers to the native HEIF metadata path.
  void checkHandler(uint32_t handler) const override {
    supported(handler == bmffType("pict"), "non-picture handler");
  }

  //! @brief Keep protected items on the unsupported-layout path.
  void checkProtection(uint16_t index) const override {
    supported(index == 0, "protected item");
  }

  //! @brief Require the tables needed to resolve the primary image and its properties.
  void checkMeta(const BmffBox& box) const override {
    for (const auto required :
         {bmffType("hdlr"), bmffType("pitm"), bmffType("iinf"), bmffType("iloc"), bmffType("iprp")})
      require(has(box.children, required));
  }

  //! @brief Preserve the native reader's nonempty primary-image requirement.
  void checkItems(const BmffItemModel& model) const override {
    require(!model.items.empty());
    require(model.items.contains(model.primaryItem));
  }

  //! @brief Require each file-relative extent to lie wholly in one mdat payload.
  void checkExtent(const BmffItemLocation& location, const BmffExtent& extent) const override {
    if (location.constructionMethod != 0)
      return;

    const auto& ranges = document_.mediaData;
    auto media = std::upper_bound(ranges.begin(), ranges.end(), extent.source.offset,
                                  [](uint64_t offset, const BmffSpan& range) { return offset < range.offset; });
    require(media != ranges.begin());
    --media;
    const auto offset = extent.source.offset - media->offset;
    require(offset <= media->size && extent.length <= media->size - offset);
  }

  //! @brief Expose Canon UUID child headers with the enclosing reader's limits and duplicate checks.
  void parseExtension(BmffReader& reader, BmffBox& box, BmffCursor& fields, unsigned childDepth) const override {
    if (box.type != bmffType("uuid") || box.userType != canonUuid)
      return;

    std::set<uint32_t> singletons;
    reader.visit(fields, childDepth, [&](BmffBox& child, BmffCursor&) {
      require(singletons.insert(child.type).second);
      box.children.push_back(std::move(child));
    });
  }

 private:
  const HeifDocument& document_;
};

// Relocation decisions depend on the HEIF container owning each box.
enum class Context { file, meta, properties, propertyList, dataInfo, dataRefs };

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

std::vector<uint32_t> HeifDocument::metadataItems(uint32_t type) const {
  // cdsc points from the metadata item to the image it describes.
  std::set<uint32_t> describing;
  for (const auto& reference : meta.references) {
    if (reference.type == bmffType("cdsc") &&
        std::find(reference.to.begin(), reference.to.end(), meta.primaryItem) != reference.to.end())
      describing.insert(reference.from);
  }

  // Keep iinf order so later metadata items have deterministic merge precedence.
  std::vector<uint32_t> result;
  for (const auto id : meta.infoOrder) {
    const auto& info = meta.items.at(id).info;
    if (describing.contains(id) && info.type == type &&
        (type != bmffType("mime") || info.contentType == "application/rdf+xml"))
      result.push_back(id);
  }

  return result;
}

HeifDocument parseHeif(BasicIo& io, const HeifLimits& limits) {
  BmffReader reader(io, limits.boxes);
  HeifDocument document;
  document.file.size = reader.fileSize();
  const HeifItemPolicy policy(document);
  auto input = reader.cursor({0, document.file.size});
  std::set<uint32_t> singletons;

  // Frame the file while the item parser handles only the standard meta tables.
  reader.visit(input, 0, [&](BmffBox& box, BmffCursor& fields) {
    if (box.type == bmffType("ftyp")) {
      require(singletons.insert(box.type).second);
      require(fields.remaining() >= 8 && fields.remaining() % 4 == 0);
      document.majorBrand = static_cast<uint32_t>(fields.number(4));
      document.minorVersion = static_cast<uint32_t>(fields.number(4));
      require(fields.remaining() / 4 <= limits.boxes.maxBoxes);
      while (fields.remaining() != 0)
        document.compatibleBrands.push_back(static_cast<uint32_t>(fields.number(4)));
    } else if (box.type == bmffType("meta")) {
      require(singletons.insert(box.type).second);
      auto itemLimits = limits.items;
      itemLimits.maxEntries = std::min(itemLimits.maxEntries, limits.boxes.maxBoxes);
      document.meta = parseBmffItems(reader, box, fields, 1, itemLimits, policy);
    } else if (box.type == bmffType("mdat")) {
      document.mediaData.push_back(box.payload());
    }
    document.file.boxes.push_back(std::move(box));
  });

  // Resolve ranges after framing discovers mdat payloads both before and after meta.
  require(!document.file.boxes.empty() && document.file.boxes.front().type == bmffType("ftyp"));
  require(has(document.file.boxes, bmffType("ftyp")) && has(document.file.boxes, bmffType("meta")));
  resolveBmffItems(document.meta, document.file.size, policy);
  return document;
}

void enforceHeifWriteSupport(const HeifDocument& document) {
  supported(document.majorBrand == bmffType("heic") || document.majorBrand == bmffType("heix") ||
                document.majorBrand == bmffType("mif1"),
            "file brand");

  // A generic mif1 brand can accompany an AVIF or another deferred codec.
  supported(std::find(document.compatibleBrands.begin(), document.compatibleBrands.end(), bmffType("avif")) ==
                    document.compatibleBrands.end() &&
                std::find(document.compatibleBrands.begin(), document.compatibleBrands.end(), bmffType("avis")) ==
                    document.compatibleBrands.end(),
            "AVIF writing is deferred");

  const auto primaryType = document.meta.items.at(document.meta.primaryItem).info.type;
  supported(primaryType == bmffType("hvc1") || primaryType == bmffType("grid") || primaryType == bmffType("iden") ||
                primaryType == bmffType("iovl"),
            "primary image type");

  // Recognize near-matching XMP MIME labels so they cannot evade removal checks.
  for (const auto& reference : document.meta.references) {
    if (reference.type != bmffType("cdsc") ||
        std::find(reference.to.begin(), reference.to.end(), document.meta.primaryItem) == reference.to.end())
      continue;
    const auto& info = document.meta.items.at(reference.from).info;
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
  checkRelocation(document.file.boxes, Context::file);
}

std::vector<uint8_t> readHeifItem(BasicIo& input, const BmffItem& item, uint64_t limit) {
  if (item.location.dataSize > limit || item.location.dataSize > std::numeric_limits<size_t>::max())
    throw Error(ErrorCode::kerErrorMessage, "Unsupported HEIF edit: metadata exceeds the allocation limit");
  return readBmffItem(input, item, limit);
}

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
