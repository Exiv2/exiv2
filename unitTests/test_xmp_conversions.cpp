// SPDX-License-Identifier: GPL-2.0-or-later

#include "../xmpsdk/src/XMPCore_Impl.hpp"

#include <gtest/gtest.h>
#include <cstdint>
#include <limits>

TEST(XmpConversions, checksStringLengthsAtTheApiBoundary) {
  constexpr auto maxLength = std::numeric_limits<XMP_StringLen>::max();
  EXPECT_EQ(maxLength, XMP_CheckedCast<XMP_StringLen>(uint64_t{maxLength}));
  EXPECT_THROW(XMP_CheckedCast<XMP_StringLen>(uint64_t{maxLength} + 1), XMP_Error);
  EXPECT_THROW(XMP_CheckedCast<XMP_StringLen>(int64_t{-1}), XMP_Error);
}

TEST(XmpConversions, checksSignedArrayIndexBounds) {
  constexpr auto maxIndex = std::numeric_limits<XMP_Index>::max();
  EXPECT_EQ(maxIndex, XMP_CheckedCast<XMP_Index>(uint64_t{maxIndex}));
  EXPECT_EQ(-1, XMP_CheckedCast<XMP_Index>(int64_t{-1}));
  EXPECT_THROW(XMP_CheckedCast<XMP_Index>(uint64_t{maxIndex} + 1), XMP_Error);
}
