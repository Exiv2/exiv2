// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef HEIFIMAGE_INT_HPP
#define HEIFIMAGE_INT_HPP

#include "config.h"

#ifdef EXV_ENABLE_BMFF
#include "image.hpp"

namespace Exiv2::Internal {

/*!
  @brief Create the private HEIF image implementation for the image factory.
  @param io Source stream whose ownership passes to the returned image.
  @param params Image construction and metadata decoding parameters.
  @return An owning image pointer, or nullptr if the image's initial good() check fails.
 */
Image::UniquePtr newHeifInstance(std::unique_ptr<BasicIo> io, const ImageCtorParams& params);

/*!
  @brief Probe the leading ftyp box for a supported HEIF major brand, excluding AVIF.
  @param io Borrowed, open, seekable input positioned at the candidate ftyp header.
  @param advance Keep the consumed position on a match; otherwise seek back to the starting position.
  @return True for a complete, bounded brand table accepted by the HEIF factory probe.

  A failed match also seeks back. This probe does not validate the complete item
  layout or promise that a later metadata write will be supported.
 */
bool isHeifType(BasicIo& io, bool advance);
}  // namespace Exiv2::Internal
#endif
#endif
