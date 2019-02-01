#include <enforce.hpp>

#include <gtest/gtest.h>

#include <stdexcept>

using namespace Exiv2;

TEST(enforce, errMessage)
{
    try {
        enforce(false, ErrorCode::kerErrorMessage, "an error occurred");
    } catch (const Error& e) {
        ASSERT_STREQ(e.what(), "an error occurred");
    }
}

TEST(enforce, exceptionThrown)
{
    ASSERT_NO_THROW(enforce(true, ErrorCode::kerErrorMessage));

    ASSERT_THROW(enforce(false, ErrorCode::kerErrorMessage), Error);
    ASSERT_THROW(enforce<std::overflow_error>(false, "error message"), std::overflow_error);
    ASSERT_THROW(enforce(false, ErrorCode::kerMallocFailed), Error);
    ASSERT_THROW(enforce(false, ErrorCode::kerErrorMessage, "error message"), Error);
    ASSERT_THROW(enforce(false, ErrorCode::kerDataSourceOpenFailed, "path", "strerror"), Error);
    ASSERT_THROW(enforce(false, ErrorCode::kerCallFailed, "path", "strerror", "function"), Error);
}
