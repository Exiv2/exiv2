#include "value.hpp"

#include "gtestwrapper.h"

using namespace Exiv2;

TEST(ADateValue, isDefaultConstructed)
{
    const DateValue dateValue;
    ASSERT_EQ(0, dateValue.getDate().year);
    ASSERT_EQ(0, dateValue.getDate().month);
    ASSERT_EQ(0, dateValue.getDate().day);
}

TEST(ADateValue, isConstructedWithArgs)
{
    const DateValue dateValue (2018, 4, 2);
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
}


TEST(ADateValue, readFromByteBufferWithExpectedSize)
{
    DateValue dateValue;
    const byte date[8] = {0x32, 0x30, 0x31, 0x38, 0x30, 0x34, 0x30, 0x32 }; // 20180402
    ASSERT_EQ(0, dateValue.read(date, 8));
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
}

TEST(ADateValue, doNotReadFromByteBufferWithoutExpectedSize)
{
    DateValue dateValue;
    const byte date[8] = {0x32, 0x30, 0x31, 0x38, 0x30, 0x34, 0x30, 0x32 }; // 20180402
    ASSERT_EQ(1, dateValue.read(date, 9));
}

TEST(ADateValue, doNotReadFromByteBufferWithExpectedSizeButNotCorrectContent)
{
    DateValue dateValue;
    const byte date[8] = {0x32, 0x30, 0x31, 0x38, 0x30, 0x34, 0x23, 0x23 }; // 201804##
    ASSERT_EQ(1, dateValue.read(date, 8));
}


TEST(ADateValue, readFromStringWithExpectedSize)
{
    DateValue dateValue;
    const std::string date ("2018-04-02");
    ASSERT_EQ(0, dateValue.read(date));
    ASSERT_EQ(2018, dateValue.getDate().year);
    ASSERT_EQ(4, dateValue.getDate().month);
    ASSERT_EQ(2, dateValue.getDate().day);
}

TEST(ADateValue, doNotReadFromStringWithoutExpectedSize)
{
    DateValue dateValue;
    const std::string date ("20180402");
    ASSERT_EQ(1, dateValue.read(date));
}

TEST(ADateValue, doNotReadFromStringWithExpectedSizeButNotCorrectContent)
{
    DateValue dateValue;
    const std::string date ("2018-aa-bb");
    ASSERT_EQ(1, dateValue.read(date));
}



TEST(ADateValue, copyToByteBuffer)
{
    const DateValue dateValue (2018, 4, 2);
    byte buffer[9];
    ASSERT_EQ(8, dateValue.copy(buffer));
    ASSERT_STREQ("20180402", reinterpret_cast<const char *>(buffer));
}
