// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>
#include <enforce.hpp>

#include <gtest/gtest.h>

#include <stdexcept>

TEST(enforce, errMessageCanBeRetrievedFromErrorException)
{
    try {
        enforce(false, Exiv2::ErrorCode::kerErrorMessage, "an error occurred");
    } catch (const Exiv2::Error& e) {
        ASSERT_STREQ(e.what(), "an error occurred");
    }
}

TEST(enforce, withTrueConditionDoesNotThrow)
{
    ASSERT_NO_THROW(enforce(true, Exiv2::ErrorCode::kerErrorMessage));
}

TEST(enforce, withFalseConditionThrows)
{
    ASSERT_THROW(enforce(false, Exiv2::ErrorCode::kerErrorMessage), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::ErrorCode::kerErrorMessage, "error message"), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::ErrorCode::kerDataSourceOpenFailed, "path", "strerror"), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::ErrorCode::kerCallFailed, "path", "strerror", "function"), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::ErrorCode::kerMallocFailed), Exiv2::Error);
    ASSERT_THROW(enforce<std::overflow_error>(false, "error message"), std::overflow_error);
}
