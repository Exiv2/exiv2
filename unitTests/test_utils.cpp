// SPDX-License-Identifier: GPL-2.0-or-later

#include "utils.hpp"

#include <gtest/gtest.h>

using namespace Exiv2::Internal;

TEST(stringUtils, startsWithReturnsTrue) {
  ASSERT_TRUE(startsWith("Exiv2 rocks", "Exiv2"));
}

TEST(stringUtils, startsWithReturnsFlase) {
  ASSERT_FALSE(startsWith("Exiv2 rocks", "exiv2"));
}

TEST(stringUtils, upperTransformStringToUpperCase) {
  ASSERT_EQ("EXIV2 ROCKS", upper("Exiv2 rocks"));
}

TEST(stringUtils, lowerTransformStringToLowerCase) {
  ASSERT_EQ("exiv2 rocks", lower("EXIV2 ROCKS"));
}
