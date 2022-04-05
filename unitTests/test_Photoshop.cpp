// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/jpgimage.hpp>

#include <gtest/gtest.h>

using namespace Exiv2;

namespace {
constexpr std::array validMarkers{"8BIM", "AgHg", "DCSR", "PHUT"};
}

TEST(Photoshop_isIrb, returnsTrueWithValidMarkers) {
  for (const auto& marker : validMarkers) {
    ASSERT_TRUE(Photoshop::isIrb(reinterpret_cast<const byte*>(marker), 4));
  }
}


TEST(Photoshop_isIrb, returnsFalseWithInvalidMarkers) {
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("7BIM"), 4));
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("AGHg"), 4));
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("dcsr"), 4));
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("LUIS"), 4));
}

TEST(Photoshop_isIrb, returnsFalseWithInvalidSize) {
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("8BIM"), 3));
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("8BIM"), 0));
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("8BIM"), 5));
}
