// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef BMFFWRITE_INT_HPP
#define BMFFWRITE_INT_HPP

#include "bmffimage_int.hpp"

#ifdef EXV_ENABLE_BMFF

namespace Exiv2::Internal {

//! Metadata buffers are bounded independently of the encoded image size.
inline constexpr uint64_t bmffMetadataLimit = 64 * 1024 * 1024;

struct BmffMetadataUpdate {
  //! No value preserves the category; an empty value removes it from the primary image.
  std::optional<std::vector<uint8_t>> exif;
  std::optional<std::vector<uint8_t>> xmp;
};

//! Gather a checked item's extents, with an explicit metadata allocation limit.
std::vector<uint8_t> readBmffItem(BasicIo& input, const BmffItem& item, uint64_t limit = bmffMetadataLimit);

/*!
  @brief Compact an item-based HEIF into a fresh, open, seekable output.
  Input must remain unchanged and must not alias output. No transfer to the input
  is performed. The output may contain partial data after an I/O failure and must
  then be discarded. On success its parsed structure and copied bytes have been
  verified. Unknown relocation, shared-metadata edits and conflicting overlap are
  rejected before any output is written. This function does not serialize TIFF/XMP.
 */
void rewriteBmff(BasicIo& input, BasicIo& output, const BmffDocument& original, const BmffMetadataUpdate& update = {});

}  // namespace Exiv2::Internal
#endif
#endif
