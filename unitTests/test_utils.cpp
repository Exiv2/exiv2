// SPDX-License-Identifier: GPL-2.0-or-later

#include "utils.hpp"

#include <gtest/gtest.h>

using namespace Exiv2::Internal;

TEST(stringUtils, startsWithReturnsTrue) {
  std::string_view s = "Exiv2 rocks";
  ASSERT_TRUE(s.starts_with("Exiv2"));
}

TEST(stringUtils, startsWithReturnsFlase) {
  std::string_view s = "Exiv2 rocks";
  ASSERT_FALSE(s.starts_with("exiv2"));
}

TEST(stringUtils, upperTransformStringToUpperCase) {
  ASSERT_EQ("EXIV2 ROCKS", upper("Exiv2 rocks"));
}

TEST(stringUtils, lowerTransformStringToLowerCase) {
  ASSERT_EQ("exiv2 rocks", lower("EXIV2 ROCKS"));
}
