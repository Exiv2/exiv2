// SPDX-License-Identifier: GPL-2.0-or-later

#include <enforce.hpp>
#include <exiv2/exiv2.hpp>

#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

TEST(enforce, errMessageCanBeRetrievedFromErrorException) {
  try {
    Exiv2::Internal::enforce(false, Exiv2::ErrorCode::kerErrorMessage, "an error occurred");
  } catch (const Exiv2::Error& e) {
    ASSERT_STREQ(e.what(), "an error occurred");
  }
}

TEST(enforce, withTrueConditionDoesNotThrow) {
  ASSERT_NO_THROW(Exiv2::Internal::enforce(true, Exiv2::ErrorCode::kerErrorMessage));
}

TEST(enforce, withFalseConditionThrows) {
  ASSERT_THROW(Exiv2::Internal::enforce(false, Exiv2::ErrorCode::kerErrorMessage), Exiv2::Error);
  ASSERT_THROW(Exiv2::Internal::enforce(false, Exiv2::ErrorCode::kerErrorMessage, "error message"), Exiv2::Error);
  ASSERT_THROW(Exiv2::Internal::enforce(false, Exiv2::ErrorCode::kerDataSourceOpenFailed, "path", "strerror"),
               Exiv2::Error);
  ASSERT_THROW(Exiv2::Internal::enforce(false, Exiv2::ErrorCode::kerCallFailed, "path", "strerror", "function"),
               Exiv2::Error);
  ASSERT_THROW(Exiv2::Internal::enforce(false, Exiv2::ErrorCode::kerMallocFailed), Exiv2::Error);
  ASSERT_THROW(Exiv2::Internal::enforce<std::overflow_error>(false, "error message"), std::overflow_error);
}

TEST(checkedCast, rejectsValuesOutsideTheDestinationRange) {
  using Exiv2::Internal::checkedCast;
  constexpr auto max32 = std::numeric_limits<uint32_t>::max();
  EXPECT_EQ(max32, checkedCast<uint32_t>(uint64_t{max32}));
  EXPECT_THROW(checkedCast<uint32_t>(uint64_t{max32} + 1), Exiv2::Error);
  EXPECT_THROW(checkedCast<uint64_t>(int64_t{-1}), Exiv2::Error);
  EXPECT_THROW(checkedCast<int64_t>(std::numeric_limits<uint64_t>::max()), Exiv2::Error);
}
