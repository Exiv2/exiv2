// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef HEIFWRITE_INT_HPP
#define HEIFWRITE_INT_HPP

#include "heifstructure_int.hpp"

#ifdef EXV_ENABLE_BMFF

namespace Exiv2::Internal {

//! @brief Serialized primary metadata requests, with distinct preserve, remove, and replace states.
struct HeifMetadataUpdate {
  //! @brief No value preserves the category; an empty value removes it from the primary image.
  std::optional<std::vector<uint8_t>> exif;

  //! @brief Raw MIME XMP packet, with the same preserve/remove/replace semantics as exif.
  std::optional<std::vector<uint8_t>> xmp;
};

/*!
  @brief Compact an item-based HEIF into a fresh, open, seekable output.
  @param input Borrowed source stream used to parse original; it must remain unchanged.
  @param output Borrowed, empty staging stream, distinct from input.
  @param original Validated structure and source ranges returned by parseHeif().
  @param update Serialized primary Exif/XMP updates; absent entries preserve their category.
  @throws Error For unsupported edits, invalid layout, I/O failure, or failed verification.

  Input must remain unchanged and must not alias output. No transfer to the input
  is performed. Both stream positions may change. The output may contain partial data after an I/O failure and must
  then be discarded. On success its parsed structure and copied bytes have been
  verified. Unknown relocation, shared-metadata edits and conflicting overlap are
  rejected before any output is written. This function does not serialize TIFF/XMP.
 */
void rewriteHeif(BasicIo& input, BasicIo& output, const HeifDocument& original, const HeifMetadataUpdate& update = {});

}  // namespace Exiv2::Internal
#endif
#endif
