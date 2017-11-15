#include <tiffimage_int.hpp>

#include <gtest/gtest.h>
#include <sstream>

using namespace Exiv2;

static const byte tiffLittleEndian[] = {0x49, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00};

class ATiffHeader: public ::testing::Test
{
public:
    Internal::TiffHeader header;
};

TEST_F(ATiffHeader, hasExpectedValuesAfterCreation)
{
    ASSERT_EQ(8, header.size());
    ASSERT_EQ(42, header.tag());
    ASSERT_EQ(8, header.offset());
    ASSERT_EQ(littleEndian, header.byteOrder());
}

TEST_F(ATiffHeader, canBeWrittenAndItsSizeIs8Bytes)
{
    DataBuf buffer = header.write();
    ASSERT_EQ(header.size(), buffer.size_);
    ASSERT_EQ(8, header.size());
}

TEST_F(ATiffHeader, readDataFromBufferWithCorrectSize)
{
    ASSERT_TRUE(header.read(tiffLittleEndian, 8));
}

TEST_F(ATiffHeader, failToReadDataFromBufferWithCorrectSizeButNull)
{
    ASSERT_FALSE(header.read(NULL, 8));
}

TEST_F(ATiffHeader, failToReadDataFromBufferWithSizeDifferentThan8)
{
    ASSERT_FALSE(header.read(NULL, 7));
    ASSERT_FALSE(header.read(NULL, 9));
}

TEST_F(ATiffHeader, failToReadDataFromBufferWithInvalidByteOrder)
{
    static const byte bufferInvalidByteOrder[] =  {0x88, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00};
    ASSERT_FALSE(header.read(bufferInvalidByteOrder, 8));
}

TEST_F(ATiffHeader, failToReadDataFromBufferWithInvalidTag)
{
    static const byte bufferInvalidTag[] = {0x49, 0x49, 0x29, 0x00, 0x10, 0x00, 0x00, 0x00};
    ASSERT_FALSE(header.read(bufferInvalidTag, 8));
}

TEST_F(ATiffHeader, printsDefaultValues)
{
    std::ostringstream str;
    header.print(str, "");
    ASSERT_STREQ("TIFF header, offset = 0x00000008, little endian encoded\n", str.str().c_str());
}
