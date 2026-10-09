// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BMFFIMAGE_INT_HPP
#define BMFFIMAGE_INT_HPP

#include "config.h"

#ifdef EXV_ENABLE_BMFF

#include <array>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace Exiv2 {
class BasicIo;
}

namespace Exiv2::Internal {

constexpr uint32_t bmffType(const char (&name)[5]) {
  return (uint32_t{static_cast<unsigned char>(name[0])} << 24) | (uint32_t{static_cast<unsigned char>(name[1])} << 16) |
         (uint32_t{static_cast<unsigned char>(name[2])} << 8) | uint32_t{static_cast<unsigned char>(name[3])};
}

//! A validated interval in the original input. Payloads are never owned by the model.
struct BmffSpan {
  uint64_t offset{};
  uint64_t size{};
  bool operator==(const BmffSpan&) const = default;
};

struct BmffFullBox {
  uint8_t version{};
  uint32_t flags{};
};

struct BmffBox {
  uint32_t type{};
  BmffSpan span;
  uint8_t headerSize{};  //!< Includes a UUID user type, but excludes FullBox version/flags.
  bool extendsToEnd{};
  std::array<uint8_t, 16> userType{};
  std::optional<BmffFullBox> fullBox;
  std::vector<BmffBox> children;

  [[nodiscard]] BmffSpan payload() const;
};

struct BmffItemInfo {
  BmffSpan box;
  uint8_t version{};
  uint32_t flags{};
  uint32_t type{};
  uint16_t protectionIndex{};
  std::string name;
  std::string contentType;
  std::string contentEncoding;
  std::string uriType;
};

struct BmffExtent {
  uint64_t index{};
  uint64_t offset{};
  uint64_t length{};
  BmffSpan source;
};

struct BmffItemLocation {
  uint8_t constructionMethod{};
  uint16_t dataReferenceIndex{};
  uint64_t baseOffset{};
  uint64_t dataSize{};
  std::vector<BmffExtent> extents;
};

struct BmffItem {
  BmffItemInfo info;
  BmffItemLocation location;
};

struct BmffLocationFormat {
  BmffSpan box;
  uint8_t version{};
  uint8_t offsetSize{};
  uint8_t lengthSize{};
  uint8_t baseOffsetSize{};
  uint8_t indexSize{};
};

struct BmffReference {
  BmffSpan box;
  uint32_t type{};
  uint32_t from{};
  std::vector<uint32_t> to;
};

struct BmffPropertyAssociation {
  uint16_t index{};  //!< One-based property index; zero denotes no association.
  bool essential{};
};

struct BmffAssociationEntry {
  BmffSpan box;
  uint32_t itemId{};
  std::vector<BmffPropertyAssociation> properties;
};

//! Limits apply to aggregate structural work, independently of encoded image size.
struct BmffLimits {
  uint64_t maxBoxes{100000};
  uint64_t maxItems{65536};
  uint64_t maxExtents{1000000};
  uint64_t maxReferences{1000000};
  uint64_t maxAssociations{1000000};
  uint64_t maxStringBytes{1024 * 1024};
  uint64_t maxStringLength{65536};
  uint64_t maxBytesRead{16 * 1024 * 1024};
  unsigned maxDepth{16};
};

//! Checked item-based BMFF structure, independent of BmffImage's metadata state.
struct BmffDocument {
  uint64_t fileSize{};
  uint32_t majorBrand{};
  uint32_t minorVersion{};
  std::vector<uint32_t> compatibleBrands;
  std::vector<BmffBox> boxes;
  uint32_t primaryItem{};
  BmffLocationFormat locationFormat;
  std::map<uint32_t, BmffItem> items;
  std::vector<uint32_t> infoOrder;
  std::vector<uint32_t> locationOrder;
  std::vector<BmffReference> references;
  std::vector<BmffBox> properties;
  std::vector<BmffAssociationEntry> associations;
  std::vector<BmffSpan> mediaData;
  std::optional<BmffSpan> itemData;

  //! Exif or XMP items describing the primary image, in iinf order.
  [[nodiscard]] std::vector<uint32_t> metadataItems(uint32_t type) const;
};

/*!
  @brief Parse an open, seekable BasicIo without reading image payloads or writing.
  The input must stay unchanged while its returned ranges are used. The stream
  position may change. Malformed input throws kerCorruptedMetadata; unsupported
  syntax throws an explicit unsupported-layout error. Parsing alone does not
  authorize rewriting: call enforceHeifWriteSupport before using a rewrite path.
 */
BmffDocument parseBmff(BasicIo& io, const BmffLimits& limits = {});

//! Reject deferred brands and opaque structures whose relocation is not supported.
void enforceHeifWriteSupport(const BmffDocument& document);

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
#endif  // BMFFIMAGE_INT_HPP
