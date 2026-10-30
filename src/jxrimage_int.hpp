// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef JXRIMAGE_INT_HPP_
#define JXRIMAGE_INT_HPP_

#include "basicio.hpp"
#include "image.hpp"

namespace Exiv2::Internal {

/*! @brief Open an existing JPEG XR image through the private TIFF-based adapter.
 *  Takes ownership of io. Creation is unsupported and throws before serialization.
 */
Image::UniquePtr newJxrInstance(BasicIo::UniquePtr io, const ImageCtorParams& params);

/*! @brief Probe the version-1 JPEG XR signature, independently of the filename.
 *  Restores the stream position unless advance is true and the signature matches.
 *  Returns false on a short read; no metadata or compressed data is decoded.
 */
bool isJxrType(BasicIo& io, bool advance);

}  // namespace Exiv2::Internal

#endif
