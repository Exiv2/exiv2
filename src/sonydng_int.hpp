// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIV2_SONYDNG_INT_HPP
#define EXIV2_SONYDNG_INT_HPP

#include "types.hpp"

#include <cstddef>

namespace Exiv2 {
class DecodeParams;
class ExifData;

namespace Internal {
class TiffComponent;

/*! @brief Recognize Adobe's private-data identifier within a bounded byte buffer. */
bool isAdobeDngPrivateData(const byte* data, size_t size);

/*!
  @brief Decode headerless Sony2 MakerNotes from an Adobe MakN record.
  @param data Private-data bytes, borrowed for this call only.
  @param size Number of readable bytes in data.
  @param context Camera Make/Model used by existing Sony decoding rules.
  @param params Existing metadata decoding limits.
  @return Decoded MakerNote fields; empty for an unsupported layout or absent MakN.
  @throws Error If Adobe framing or the root IFD is malformed.
  @note Offset resolution is confined to the original MakerNote payload. Other
        records remain opaque. No buffers are sized using saved original offsets.
        At most 256 records and a 16 MiB MakerNote payload are accepted; isolated
        TIFF reading bounds total binary-array expansion to 65536 elements.
 */
ExifData decodeSonyDngPrivateData(const byte* data, size_t size, const ExifData& context, const DecodeParams& params);

/*!
  @brief Remove unchanged Adobe-derived Sony views from an encoder's metadata copy.
  @param data Encoder-owned metadata; the caller's original container is unchanged.
  @param source Original TIFF tree, if present, borrowed for this call only.
  @throws Error Before source writes if a wrapped MakerNote field was edited.
  @note Retains the raw Adobe block for opaque writing. Native MakerNotes take
        precedence; raw-only imports and complete Exif removal remain supported.
 */
void filterSonyDngMetadata(ExifData& data, TiffComponent* source);

}  // namespace Internal
}  // namespace Exiv2

#endif  // EXIV2_SONYDNG_INT_HPP
