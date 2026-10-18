// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef CR3PREVIEW_INT_HPP
#define CR3PREVIEW_INT_HPP

#include "config.h"

#ifdef EXV_ENABLE_BMFF

#include "image.hpp"

#include <optional>

namespace Exiv2::Internal {

/*!
  @brief Locate the largest supported single-sample JPEG track in a CR3 input.
  @param io Open, seekable, unchanged input, borrowed for this call only.
  @param maxDepth Caller-selected maximum box nesting depth.
  @return A native preview borrowing an absolute source range, or no value if
          optional discovery encounters unsupported/malformed structure or limits.
  @throws Error For failures reading or seeking the underlying input.

  Only crx-major-brand, locally referenced CRAW/JPEG tracks are accepted. The
  extra pass reads at most 4096 headers and 1 MiB of structural fields; image
  payloads remain in the input except for a two-byte signature check. The input
  must outlive use of the returned preview. The stream position may change.
 */
std::optional<NativePreview> findCr3JpegPreview(BasicIo& io, size_t maxDepth);

}  // namespace Exiv2::Internal
#endif  // EXV_ENABLE_BMFF
#endif  // CR3PREVIEW_INT_HPP
