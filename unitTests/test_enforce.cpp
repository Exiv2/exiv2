#include <enforce.hpp>

#include "gtestwrapper.h"

TEST(enforce, errMessage)
{
    try {
        enforce(false, Exiv2::kerErrorMessage, "an error occurred");
    } catch (const Exiv2::Error& e) {
        ASSERT_STREQ(e.what(), "an error occurred");
    }
}

TEST(enforce, exceptionThrown)
{
    ASSERT_NO_THROW(enforce(true, Exiv2::kerErrorMessage));

    ASSERT_THROW(enforce(false, Exiv2::kerErrorMessage), Exiv2::Error);
    ASSERT_THROW(enforce<std::overflow_error>(false, "error message"), std::overflow_error);
    ASSERT_THROW(enforce(false, Exiv2::kerMallocFailed), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::kerErrorMessage, "error message"), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::kerDataSourceOpenFailed, "path", "strerror"), Exiv2::Error);
    ASSERT_THROW(enforce(false, Exiv2::kerCallFailed, "path", "strerror", "function"), Exiv2::Error);
}
