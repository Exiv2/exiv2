#include <gtest/gtest.h>
#include <exiv2/types.hpp>

using namespace Exiv2;

TEST(ExivTime, getsTimeFromValidString)
{
    struct tm tmInstance;
    ASSERT_EQ(0, exifTime("2007:05:24 12:31:55", &tmInstance));
    /// \todo add more checks here
}
