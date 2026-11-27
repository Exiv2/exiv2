// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef HEIFSTRUCTURE_INT_HPP
#define HEIFSTRUCTURE_INT_HPP

#include "bmffitem_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include <limits>

namespace Exiv2::Internal {

//! @brief HEIF-specific defaults; other adapters choose their own reader and item budgets.
struct HeifLimits {
  BmffReadLimits boxes{100000, 16 * 1024 * 1024, 16};  //!< Framing, I/O and nesting bounds.

  BmffItemLimits items{
      65536,
      1000000,
      1000000,
      1000000,
      1024 * 1024,
      65536,
      std::numeric_limits<uint64_t>::max()};  //!< Item bounds; maxEntries is also capped by boxes.maxBoxes.
};

//! @brief HEIF framing and item tables with the policy context required for metadata edits.
struct HeifDocument {
  BmffFile file;                           //!< Source framing, independent of item tables.
  BmffItemModel meta;                      //!< Standard item tables from the single top-level meta box.
  uint32_t majorBrand{};                   //!< Major brand from ftyp.
  uint32_t minorVersion{};                 //!< Minor version from ftyp.
  std::vector<uint32_t> compatibleBrands;  //!< Compatible brands in their original order.
  std::vector<BmffSpan> mediaData;         //!< Absolute mdat payload intervals allowed to own file-relative items.

  /*!
    @brief Select primary Exif or XMP items in iinf order, preserving merge precedence.
    @param type Exif or MIME item type; MIME selection requires application/rdf+xml.
    @return IDs whose cdsc references include the primary image item.
   */
  [[nodiscard]] std::vector<uint32_t> metadataItems(uint32_t type) const;
};

//! @brief HEIF metadata allocations are bounded independently of encoded image size.
inline constexpr uint64_t heifMetadataLimit = 64 * 1024 * 1024;

/*!
  @brief Parse the item layout admitted by the HEIF adapter, without decoding image data.
  @param io Borrowed input, already open and seekable; its position may change.
  @param limits Bounds on structural I/O, nesting and item tables.
  @return Owned framing and item tables whose source spans borrow the unchanged input bytes.
  @throws Error For malformed or unsupported layout, exceeded limits or failed I/O.

  This preserves the native HEIF reader's admission rules. Parsing does not
  authorize writing; enforceHeifWriteSupport also checks brands and relocation.
  Legacy fallback interpretation remains in BmffImage, outside the standard item model.
 */
HeifDocument parseHeif(BasicIo& io, const HeifLimits& limits = {});

/*!
  @brief Reject deferred brands and opaque structures whose relocation is not supported.
  @param document Validated model returned by parseHeif().
  @throws Error If the layout is outside the HEIF write envelope.

  The rewriter must additionally validate requested edits against shared references
  and overlapping ranges. No output is prepared by this check.
 */
void enforceHeifWriteSupport(const HeifDocument& document);

/*!
  @brief Read one resolved item with HEIF allocation limits and existing edit diagnostics.
  @param input Open, seekable input, unchanged since parsing; its position may change.
  @param item Item with validated extents and aggregate size.
  @param limit Remaining allocation budget for metadata, in bytes.
  @return Owned payload bytes, assembled in extent order.
  @throws Error For an exceeded limit, inconsistent ranges or failed input I/O.
 */
std::vector<uint8_t> readHeifItem(BasicIo& input, const BmffItem& item, uint64_t limit = heifMetadataLimit);

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
#endif  // HEIFSTRUCTURE_INT_HPP
