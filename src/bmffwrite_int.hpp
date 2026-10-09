// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BMFFWRITE_INT_HPP
#define BMFFWRITE_INT_HPP

#include "bmffimage_int.hpp"

#ifdef EXV_ENABLE_BMFF

namespace Exiv2::Internal {

//! @brief Metadata buffers are bounded independently of the encoded image size.
inline constexpr uint64_t bmffMetadataLimit = 64 * 1024 * 1024;

//! @brief Serialized primary metadata requests, with distinct preserve, remove, and replace states.
struct BmffMetadataUpdate {
  //! @brief No value preserves the category; an empty value removes it from the primary image.
  std::optional<std::vector<uint8_t>> exif;

  //! @brief Raw MIME XMP packet, with the same preserve/remove/replace semantics as exif.
  std::optional<std::vector<uint8_t>> xmp;
};

/*!
  @brief Gather a checked item's extents in their declared order.
  @param input Open, seekable input from which the item's source ranges were parsed.
  @param item Item with validated absolute extents and aggregate data size.
  @param limit Maximum number of bytes to allocate for the assembled metadata.
  @return An owned contiguous copy of the item payload.
  @throws Error If the allocation limit is exceeded, ranges disagree, or input I/O fails.

  The input must remain unchanged since parsing; its stream position may change.
 */
std::vector<uint8_t> readBmffItem(BasicIo& input, const BmffItem& item, uint64_t limit = bmffMetadataLimit);

/*!
  @brief Compact an item-based HEIF into a fresh, open, seekable output.
  @param input Borrowed source stream used to parse original; it must remain unchanged.
  @param output Borrowed, empty staging stream, distinct from input.
  @param original Validated structure and source ranges returned by parseBmff().
  @param update Serialized primary Exif/XMP updates; absent entries preserve their category.
  @throws Error For unsupported edits, invalid layout, I/O failure, or failed verification.

  Input must remain unchanged and must not alias output. No transfer to the input
  is performed. Both stream positions may change. The output may contain partial data after an I/O failure and must
  then be discarded. On success its parsed structure and copied bytes have been
  verified. Unknown relocation, shared-metadata edits and conflicting overlap are
  rejected before any output is written. This function does not serialize TIFF/XMP.
 */
void rewriteBmff(BasicIo& input, BasicIo& output, const BmffDocument& original, const BmffMetadataUpdate& update = {});

}  // namespace Exiv2::Internal
#endif
#endif
