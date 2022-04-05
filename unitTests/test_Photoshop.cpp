// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/jpgimage.hpp>

#include <gtest/gtest.h>

using namespace Exiv2;

namespace {
constexpr std::array validMarkers{"8BIM", "AgHg", "DCSR", "PHUT"};
}

TEST(Photoshop_isIrb, returnsTrueWithValidMarkers) {
  for (const auto& marker : validMarkers) {
    ASSERT_TRUE(Photoshop::isIrb(reinterpret_cast<const byte*>(marker)));
  }
}


TEST(Photoshop_isIrb, returnsFalseWithInvalidMarkers) {
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("7BIM")));
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("AGHg")));
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("dcsr")));
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("LUIS")));
}

TEST(Photoshop_isIrb, returnsFalseWithNullPointer) {
  ASSERT_FALSE(Photoshop::isIrb(nullptr));
}

/// \note probably this is not safe and we need to remove it
TEST(Photoshop_isIrb, returnsFalseWithShorterMarker) {
  ASSERT_FALSE(Photoshop::isIrb(reinterpret_cast<const byte *>("8BI")));
}
