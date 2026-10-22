// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BMFFITEMWRITE_INT_HPP
#define BMFFITEMWRITE_INT_HPP

#include "bmffitem_int.hpp"
#include "bmfflayout_int.hpp"

#ifdef EXV_ENABLE_BMFF

namespace Exiv2::Internal {

//! @brief Adapter-selected output positions for retained and replacement item data.
struct BmffItemPositions {
  std::vector<BmffRelocation> retained;       //!< Approved source intervals and absolute destinations.
  std::map<uint32_t, uint64_t> replacements;  //!< Absolute output positions of new single-extent items.
  uint64_t itemDataPosition{};                //!< Absolute idat payload origin for method 1.
};

/*!
  @brief Adapter hooks for building boxes and identifying approved item destinations.

  Implementations borrow the model passed to layoutBmffItems() and observe its
  current field widths and offsets on each build. They retain ownership of all
  payload buffers for the lifetime of the returned layout and its I/O operations.
 */
class BmffItemLayoutBuilder {
 public:
  //! @brief Destroy an adapter without affecting its borrowed model or payloads.
  virtual ~BmffItemLayoutBuilder() = default;

  //! @brief Build output boxes for the current item model; throw Error for an unsupported layout.
  virtual std::vector<BmffOutputBox> build() const = 0;

  //! @brief Identify approved item destinations in a measured layout without changing it.
  virtual BmffItemPositions positions(const std::vector<BmffOutputBox>& boxes) const = 0;
};

/*!
  @brief Normalize and converge item offsets before returning a measured output tree.
  @param model Nonempty validated model, updated to the final iloc representation.
  @param builder Adapter borrowing model and owning policy for retained and replacement data.
  @return Measured boxes with regenerated item locations; payload buffers remain borrowed.
  @throws Error For invalid mappings, overflow or a layout that fails to converge.

  Offset widths can promote from four to eight bytes once. Regenerate the boxes
  after relocation so their encoded fields agree with their measured positions.
  No I/O or transfer occurs; the caller emits and semantically verifies the result.
 */
std::vector<BmffOutputBox> layoutBmffItems(BmffItemModel& model, const BmffItemLayoutBuilder& builder);

/*!
  @brief Choose normalized iloc versions and field widths before output measurement.
  @param model Nonempty, validated local item model; owned by the caller.
  @throws Error If the model is empty.

  Retain construction methods and extent indices. Remove per-item base offsets,
  choose four-byte offsets initially, and widen lengths or item IDs as needed.
  Call once before iterating layout; subsequent offset promotion is monotonic.
 */
void normalizeBmffLocations(BmffItemModel& model);

/*!
  @brief Serialize the iloc FullBox prefix and item table in existing location order.
  @param model Normalized item model, with widths chosen for this layout pass.
  @param maxBytes Adapter-selected allocation limit for the complete encoded table.
  @return Owned bytes excluding the outer box header.
  @throws Error If a field cannot fit or the allocation budget is exceeded.

  Before the first relocation pass, out-of-range four-byte offsets are encoded
  as zero placeholders. Only a converged layout may be emitted to a file.
 */
std::vector<uint8_t> encodeBmffLocations(const BmffItemModel& model, uint64_t maxBytes);

/*!
  @brief Resolve iloc offsets from adapter-approved absolute output positions.
  @param model Normalized model whose extent offsets and offset width are updated.
  @param retained Relocations for retained source extents selected by the adapter.
  @param replacements Absolute positions for newly supplied, single-extent items.
  @param itemDataPosition Absolute output start of the idat payload, for method 1.
  @return True if offsets widened to eight bytes, requiring another layout pass.
  @throws Error For an unmapped extent or an output position preceding its origin.

  File-relative and idat-relative coordinates belong to this layer, not to the
  generic layout engine. The caller must rebuild offset-bearing bytes after
  relocation and remeasure until sizes converge before writing any output.
 */
bool relocateBmffItems(BmffItemModel& model, const BmffRelocations& retained,
                       const std::map<uint32_t, uint64_t>& replacements, uint64_t itemDataPosition);

}  // namespace Exiv2::Internal
#endif
#endif
