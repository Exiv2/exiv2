// SPDX-License-Identifier: GPL-2.0-or-later

#include "unittest_utils.hpp"

Exiv2::ImageCtorParams defaultImageCtorParams(bool create) {
  return Exiv2::ImageCtorParams(create, 500);
}
