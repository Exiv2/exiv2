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

//! @brief Pack a four-character box/item code into its big-endian integer representation.
constexpr uint32_t bmffType(const char (&name)[5]) {
  return (uint32_t{static_cast<unsigned char>(name[0])} << 24) | (uint32_t{static_cast<unsigned char>(name[1])} << 16) |
         (uint32_t{static_cast<unsigned char>(name[2])} << 8) | uint32_t{static_cast<unsigned char>(name[3])};
}

//! @brief A validated interval in the original input. Payloads are never owned by the model.
struct BmffSpan {
  uint64_t offset{};  //!< Absolute byte offset from the start of the input.
  uint64_t size{};    //!< Length of the interval in bytes.

  //! @brief Compare the absolute offset and length of two input intervals.
  bool operator==(const BmffSpan&) const = default;
};

//! @brief Version and flags decoded from the common FullBox prefix.
struct BmffFullBox {
  uint8_t version{};  //!< FullBox version selecting its field layout.
  uint32_t flags{};   //!< Low 24 flag bits of the FullBox prefix.
};

//! @brief Input box boundaries and decoded children, with opaque payloads left in the input.
struct BmffBox {
  uint32_t type{};                     //!< Packed four-character box code.
  BmffSpan span;                       //!< Complete original box interval, including its header.
  uint8_t headerSize{};                //!< Includes a UUID user type, but excludes FullBox version/flags.
  bool extendsToEnd{};                 //!< True when the original size field extends to end of file.
  std::array<uint8_t, 16> userType{};  //!< UUID user type, populated only for uuid boxes.
  std::optional<BmffFullBox> fullBox;  //!< Decoded version and flags for recognized FullBox layouts.
  std::vector<BmffBox> children;       //!< Decoded immediate children in original file order.

  //! @brief Return the absolute payload interval, excluding the box header and UUID user type.
  [[nodiscard]] BmffSpan payload() const;
};

//! @brief Item description and its original infe span, retained for lossless rewriting.
struct BmffItemInfo {
  BmffSpan box;                 //!< Original infe interval, used to preserve unchanged descriptions.
  uint8_t version{};            //!< Item info version selecting the ID width.
  uint32_t flags{};             //!< Item info flags retained when rewriting.
  uint32_t type{};              //!< Packed four-character item type.
  uint16_t protectionIndex{};   //!< Zero for supported unprotected items.
  std::string name;             //!< Decoded null-terminated item name.
  std::string contentType;      //!< MIME content type, present only for mime items.
  std::string contentEncoding;  //!< Optional MIME content encoding.
  std::string uriType;          //!< URI type string, present only for uri items.
};

//! @brief An iloc extent with its original fields and resolved absolute source interval.
struct BmffExtent {
  uint64_t index{};   //!< Original extent index, retained without interpreting it.
  uint64_t offset{};  //!< Offset relative to the item base in its construction method.
  uint64_t length{};  //!< Explicit nonzero extent length.
  BmffSpan source;    //!< Validated absolute input interval for these bytes.
};

//! @brief Item addressing and aggregate size; method 0 is file-relative, method 1 idat-relative.
struct BmffItemLocation {
  uint8_t constructionMethod{};     //!< Zero selects mdat/file addressing; one selects idat-relative addressing.
  uint16_t dataReferenceIndex{};    //!< Zero selects local storage; external references are unsupported.
  uint64_t baseOffset{};            //!< Base offset added to each extent in the selected coordinate system.
  uint64_t dataSize{};              //!< Sum of declared extent lengths, including any repeated bytes.
  std::vector<BmffExtent> extents;  //!< Ordered extents comprising the item payload.
};

//! @brief Join an item's independently ordered info and location records by ID.
struct BmffItem {
  BmffItemInfo info;          //!< Description from the item information table.
  BmffItemLocation location;  //!< Addressing from the item location table.
};

//! @brief Version and field widths shared by all records in an iloc box.
struct BmffLocationFormat {
  BmffSpan box;              //!< Original iloc box interval.
  uint8_t version{};         //!< Location version selecting ID and construction fields.
  uint8_t offsetSize{};      //!< Extent offset field width in bytes.
  uint8_t lengthSize{};      //!< Extent length field width in bytes.
  uint8_t baseOffsetSize{};  //!< Item base-offset field width in bytes.
  uint8_t indexSize{};       //!< Extent index field width in bytes.
};

//! @brief A directed item relationship, retaining destination order and its original box span.
struct BmffReference {
  BmffSpan box;              //!< Original reference child box interval.
  uint32_t type{};           //!< Packed reference type, such as cdsc or auxl.
  uint32_t from{};           //!< Source item ID.
  std::vector<uint32_t> to;  //!< Destination item IDs in their declared order.
};

//! @brief One property index and whether it is essential to interpreting the associated item.
struct BmffPropertyAssociation {
  uint16_t index{};  //!< One-based property index; zero denotes no association.
  bool essential{};  //!< True when understanding this property is required for the item.
};

//! @brief Ordered properties associated with one item in a particular ipma box.
struct BmffAssociationEntry {
  BmffSpan box;                                     //!< Containing ipma box interval.
  uint32_t itemId{};                                //!< Item to which these properties apply.
  std::vector<BmffPropertyAssociation> properties;  //!< Associations in their declared order.
};

//! @brief Limits apply to aggregate structural work, independently of encoded image size.
struct BmffLimits {
  uint64_t maxBoxes{100000};                //!< Maximum number of boxes across all containers.
  uint64_t maxItems{65536};                 //!< Maximum number of distinct item IDs.
  uint64_t maxExtents{1000000};             //!< Maximum aggregate number of item extents.
  uint64_t maxReferences{1000000};          //!< Maximum aggregate number of reference destinations.
  uint64_t maxAssociations{1000000};        //!< Budget for association records and their property entries.
  uint64_t maxStringBytes{1024 * 1024};     //!< Maximum aggregate bytes consumed by strings, including terminators.
  uint64_t maxStringLength{65536};          //!< Maximum bytes in one string, excluding its terminator.
  uint64_t maxBytesRead{16 * 1024 * 1024};  //!< Maximum structural bytes read, independent of media size.
  unsigned maxDepth{16};                    //!< Maximum child-box nesting depth below the file context.
};

//! @brief Checked item-based BMFF structure, independent of BmffImage's metadata state.
struct BmffDocument {
  uint64_t fileSize{};                     //!< Original input length in bytes.
  uint32_t majorBrand{};                   //!< Major brand from ftyp.
  uint32_t minorVersion{};                 //!< Minor version from ftyp.
  std::vector<uint32_t> compatibleBrands;  //!< Compatible brands in their original order.
  std::vector<BmffBox> boxes;              //!< Top-level boxes in original file order.
  uint32_t primaryItem{};                  //!< Primary image item ID from pitm.
  BmffLocationFormat locationFormat;       //!< Shared iloc version and field widths.
  std::map<uint32_t, BmffItem> items;      //!< Checked item descriptions and locations keyed by ID.
  std::vector<uint32_t> infoOrder;         //!< Item IDs in original iinf order, defining metadata merge precedence.
  std::vector<uint32_t> locationOrder;     //!< Item IDs in original iloc order.
  std::vector<BmffReference> references;   //!< Directed relationships in original traversal order.
  std::vector<BmffBox> properties;         //!< Property boxes in one-based index order.
  std::vector<BmffAssociationEntry> associations;  //!< Item-to-property associations in original traversal order.
  std::vector<BmffSpan> mediaData;                 //!< Absolute mdat payload intervals in file order.
  std::optional<BmffSpan> itemData;                //!< Absolute idat payload interval when present.

  /*!
    @brief Select Exif or XMP items describing the primary image, in iinf order.
    @param type Exif item type or MIME item type; MIME selection requires application/rdf+xml.
    @return IDs whose cdsc references include the primary item.
   */
  [[nodiscard]] std::vector<uint32_t> metadataItems(uint32_t type) const;
};

/*!
  @brief Parse an open, seekable BasicIo without reading image payloads or writing.
  @param io Borrowed input, already open and seekable.
  @param limits Bounds on aggregate structural work and nesting.
  @return An owning structural model whose payload ranges refer to the original input.
  @throws Error For malformed or unsupported structure, exceeded limits, or failed I/O.

  The input must stay unchanged while its returned ranges are used. The stream
  position may change. Malformed input throws kerCorruptedMetadata; unsupported
  syntax throws an explicit unsupported-layout error. Parsing alone does not
  authorize rewriting: call enforceHeifWriteSupport before using a rewrite path.
 */
BmffDocument parseBmff(BasicIo& io, const BmffLimits& limits = {});

/*!
  @brief Reject deferred brands and opaque structures whose relocation is not supported.
  @param document A validated model returned by parseBmff().
  @throws Error If the modeled layout is outside the HEIF write envelope.

  This checks layout eligibility only. The rewriter must still validate the
  requested changes against shared metadata references and overlapping extents.
 */
void enforceHeifWriteSupport(const BmffDocument& document);

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
#endif  // BMFFIMAGE_INT_HPP
