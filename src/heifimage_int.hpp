// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef HEIFIMAGE_INT_HPP
#define HEIFIMAGE_INT_HPP

#include "config.h"

#ifdef EXV_ENABLE_BMFF
#include "image.hpp"

namespace Exiv2::Internal {
Image::UniquePtr newHeifInstance(std::unique_ptr<BasicIo> io, const ImageCtorParams& params);
bool isHeifType(BasicIo& io, bool advance);
}  // namespace Exiv2::Internal
#endif
#endif
