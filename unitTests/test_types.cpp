#include <gtest/gtest.h>
#include <exiv2/types.hpp>

using namespace Exiv2;

// More info about tm : http://www.cplusplus.com/reference/ctime/tm/

TEST(ExivTime, getsTimeFromValidString)
{
    struct tm tmInstance;
    ASSERT_EQ(0, exifTime("2007:05:24 12:31:55", &tmInstance));
    ASSERT_EQ(107,  tmInstance.tm_year); // Years since 1900
    ASSERT_EQ(4,    tmInstance.tm_mon);
    ASSERT_EQ(24,   tmInstance.tm_mday);
    ASSERT_EQ(12,   tmInstance.tm_hour);
    ASSERT_EQ(31,   tmInstance.tm_min);
    ASSERT_EQ(55,   tmInstance.tm_sec);
}

TEST(ExivTime, doesNotGetTimeWithBadFormedString)
{
    struct tm tmInstance;
    ASSERT_EQ(1, exifTime("007:a5:24 aa:bb:cc", &tmInstance));
}

TEST(DataBuf, pointsToNullByDefault)
{
    DataBuf instance;
    ASSERT_EQ(NULL, instance.pData_);
    ASSERT_EQ(0,    instance.size_);
}

TEST(DataBuf, allocatesDataWithNonEmptyConstructor)
{
    DataBuf instance (5);
    ASSERT_NE(static_cast<byte *>(NULL), instance.pData_); /// \todo use nullptr once we move to c++11
    ASSERT_EQ(5,    instance.size_);
}
