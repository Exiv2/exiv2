// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BMFFITEM_INT_HPP
#define BMFFITEM_INT_HPP

#include "config.h"

#ifdef EXV_ENABLE_BMFF

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "bmffbox_int.hpp"

namespace Exiv2 {
class BasicIo;
}

namespace Exiv2::Internal {

//! @brief Item description and its original infe span, retained for lossless rewriting.
struct BmffItemInfo {
  BmffSpan box;                 //!< Original infe interval, used to preserve unchanged descriptions.
  uint8_t version{};            //!< Item info version selecting the ID width.
  uint32_t flags{};             //!< Item info flags retained when rewriting.
  uint32_t type{};              //!< Packed four-character item type.
  uint16_t protectionIndex{};   //!< Protection scheme index; zero denotes an unprotected item.
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
  uint8_t constructionMethod{};     //!< Zero selects file-relative addressing; one selects idat-relative addressing.
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

//! @brief Explicit bounds for one item model; box I/O limits belong to BmffReader.
struct BmffItemLimits {
  uint64_t maxItems;         //!< Maximum number of distinct item IDs.
  uint64_t maxExtents;       //!< Maximum aggregate extent count.
  uint64_t maxReferences;    //!< Maximum aggregate reference endpoints.
  uint64_t maxAssociations;  //!< Maximum association records plus property entries.
  uint64_t maxStringBytes;   //!< Maximum aggregate string bytes including terminators.
  uint64_t maxStringLength;  //!< Maximum bytes in a string excluding its terminator.
  uint64_t maxEntries;       //!< Maximum declared data-reference entries.
};

//! @brief Standard tables in one meta box, independent of file brands and metadata interpretation.
struct BmffItemModel {
  uint32_t handler{};                              //!< Handler type from hdlr, when present.
  uint32_t primaryItem{};                          //!< Item ID from pitm; meaningful only when hasPrimaryItem is true.
  bool hasPrimaryItem{};                           //!< Distinguish an absent pitm from an invalid zero item ID.
  BmffLocationFormat locationFormat;               //!< Shared iloc version and field widths.
  std::map<uint32_t, BmffItem> items;              //!< Item descriptions and locations keyed by ID.
  std::vector<uint32_t> infoOrder;                 //!< Item IDs in original iinf order.
  std::vector<uint32_t> locationOrder;             //!< Item IDs in original iloc order.
  std::vector<BmffReference> references;           //!< Directed relationships in traversal order.
  std::vector<BmffBox> properties;                 //!< Opaque property boxes in one-based index order.
  std::vector<BmffAssociationEntry> associations;  //!< Item-to-property associations in traversal order.
  std::optional<BmffSpan> itemData;                //!< Absolute idat payload interval when present.
};

/*!
  @brief Optional format admission checks and extension traversal for the standard item parser.

  The default policy adds no format restrictions and leaves unknown boxes opaque.
  Adapters may reject fields as they are decoded to retain their error precedence.
  Borrowed policy and reader references must outlive each parse or resolution call.
 */
class BmffItemPolicy {
 public:
  //! @brief Destroy a format policy through its private shared interface.
  virtual ~BmffItemPolicy() = default;

  //! @brief Report unsupported standard syntax; adapters may preserve their existing diagnostic.
  [[noreturn]] virtual void unsupported(std::string_view feature) const;

  //! @brief Check a decoded handler before the remaining hdlr fields are consumed.
  virtual void checkHandler(uint32_t handler) const;

  //! @brief Check the protection index before the remaining infe fields are consumed.
  virtual void checkProtection(uint16_t index) const;

  //! @brief Check required children after the meta box has been parsed.
  virtual void checkMeta(const BmffBox& box) const;

  //! @brief Check item admission after matching descriptions and locations, before range resolution.
  virtual void checkItems(const BmffItemModel& model) const;

  //! @brief Check format-specific ownership of a resolved extent within the source file.
  virtual void checkExtent(const BmffItemLocation& location, const BmffExtent& extent) const;

  /*!
    @brief Optionally parse an unknown meta child using the same reader and resource counters.
    @param reader Borrowed reader sharing the enclosing parse's I/O and nesting budgets.
    @param box Header whose children may be populated by the adapter.
    @param fields Cursor over this box's payload; prefixes may be consumed by the adapter.
    @param childDepth Nesting depth to use when visiting children of this box.

    The default leaves the box opaque. The adapter must not inspect bytes outside
    fields or restart resource counters when descending into a custom container.
   */
  virtual void parseExtension(BmffReader& reader, BmffBox& box, BmffCursor& fields, unsigned childDepth) const;
};

/*!
  @brief Parse one meta box's standard tables without interpreting codecs or metadata payloads.
  @param reader Open, seekable source reader shared with the enclosing file adapter.
  @param box Already decoded meta header; its full-box prefix and children are populated.
  @param fields Cursor at the start of box's payload, including the version and flags.
  @param childDepth Depth to use when visiting the meta children.
  @param limits Explicit bounds for work within this model.
  @param policy Borrowed format checks and extension traversal, or the neutral default policy.
  @return An owning table model with unresolved extent source spans.
  @throws Error For malformed tables, unsupported standard syntax, exceeded limits or I/O errors.

  No ftyp, brand, picture handler, primary item or property table is required.
  Unknown children remain opaque. Call resolveBmffItems after all source ranges
  are known, before reading payloads or using the model for rewriting. The input
  must remain unchanged while any returned spans are used; its position may change.
 */
BmffItemModel parseBmffItems(BmffReader& reader, BmffBox& box, BmffCursor& fields, unsigned childDepth,
                             const BmffItemLimits& limits, const BmffItemPolicy& policy = {});

/*!
  @brief Validate table links and resolve file-relative or idat-relative extents.
  @param model Parsed item model, updated with absolute source spans and aggregate sizes.
  @param fileSize Length of the unchanged source file, within BasicIo's signed seek range.
  @param policy Additional item and extent checks supplied by the format adapter.
  @throws Error For incomplete items, dangling references, invalid ranges or policy rejection.

  Method 0 may address any bytes in the source file. Adapters impose payload
  ownership restrictions when needed. Repeated resolution recomputes sizes.
 */
void resolveBmffItems(BmffItemModel& model, uint64_t fileSize, const BmffItemPolicy& policy = {});

/*!
  @brief Gather a resolved item's extents in their declared order, without interpreting the bytes.
  @param input Open, seekable source, unchanged since parsing and resolution.
  @param item Item with validated absolute extents and aggregate data size.
  @param limit Maximum allocation size in bytes, chosen by the caller.
  @return An owned contiguous copy of the item payload.
  @throws Error If the allocation limit is exceeded, ranges disagree, or source I/O fails.

  The stream position may change. This function does not decode protection schemes.
 */
std::vector<uint8_t> readBmffItem(BasicIo& input, const BmffItem& item, uint64_t limit);

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
#endif  // BMFFITEM_INT_HPP
