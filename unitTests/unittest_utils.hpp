// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef UNITTEST_UTILS_HPP_
#define UNITTEST_UTILS_HPP_

#include <exiv2/image.hpp>

constexpr size_t MAX_RECURSION_DEPTH = 500;

Exiv2::ImageCtorParams defaultImageCtorParams(bool create);
Exiv2::DecodeParams defaultDecodeParams();

#endif  // #ifndef UNITTEST_UTILS_HPP_
