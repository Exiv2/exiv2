#include <gtest/gtest.h>
#include <cr2image_int.hpp>

using namespace Exiv2;

static const byte cr2LittleEndian[] = {0x49, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00,
                                       0x43, 0x52, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};

TEST(ACr2Header, hasExpectedValuesAfterCreation)
{
    Internal::Cr2Header header;
    ASSERT_EQ(42, header.tag());
    ASSERT_EQ(16u, header.size());
    ASSERT_EQ(littleEndian, header.byteOrder());
}

TEST(ACr2Header, sizeIs16Bytes)
{
    Internal::Cr2Header header;
    DataBuf buffer = header.write();
    ASSERT_EQ(header.size(), buffer.size_);
    ASSERT_EQ(0, memcmp(cr2LittleEndian, buffer.pData_, 16));
}


TEST(ACr2Header, readDataFromBufferWithCorrectSize)
{
    Internal::Cr2Header header;
    ASSERT_TRUE(header.read(cr2LittleEndian, 16));
}

TEST(ACr2Header, failToReadDataFromBufferWithSizeDifferentThan16)
{
    Internal::Cr2Header header;
    ASSERT_FALSE(header.read(NULL, 15));
    ASSERT_FALSE(header.read(NULL, 17));
}

TEST(ACr2Header, failToReadDataFromBufferWithInvalidByteOrder)
{
    static const byte bufferInvalidByteOrder[] =  {0x88, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00,
                                                   0x43, 0x52, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
    Internal::Cr2Header header;
    ASSERT_FALSE(header.read(bufferInvalidByteOrder, 16));
}

TEST(ACr2Header, failToReadDataFromBufferWithInvalidTag)
{
    static const byte bufferInvalidTag[] = {0x49, 0x49, 0x29, 0x00, 0x10, 0x00, 0x00, 0x00,
                                            0x43, 0x52, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
    Internal::Cr2Header header;
    ASSERT_FALSE(header.read(bufferInvalidTag, 16));
}
