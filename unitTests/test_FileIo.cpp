#include <exiv2/basicio.hpp>

#include <gtest/gtest.h>

#include <array>

using namespace Exiv2;

namespace
{
    const std::string testData{TESTDATA_PATH};
    const std::string jpegPath{testData + "/DSC_3079.jpg"};
}  // namespace

struct AClosedFileIo : public testing::Test
{
    void SetUp() override
    {
        ASSERT_FALSE(file.isopen());
        ASSERT_EQ(0, file.error());
    }

    FileIo file{jpegPath};
};

TEST_F(AClosedFileIo, tellReturnsFailureValue)
{
    ASSERT_EQ(-1, file.tell());
}

TEST_F(AClosedFileIo, sizeReturnsImageSize)
{
    ASSERT_EQ(118685, file.size());
}

TEST_F(AClosedFileIo, eofReturnsFalse)
{
    ASSERT_FALSE(file.eof());
}

TEST_F(AClosedFileIo, pathReturnsInputPath)
{
    ASSERT_EQ(jpegPath, file.path());
}

TEST_F(AClosedFileIo, canBeClosed)
{
    ASSERT_EQ(0, file.close());
}

TEST_F(AClosedFileIo, writeWithRawDataFails)
{
    const std::array<byte, 4> buff{1, 2, 3, 4};
    ASSERT_EQ(0, file.write(buff.data(), buff.size()));
}

TEST_F(AClosedFileIo, writeWithOtherBasicIoClosedFails)
{
    FileIo file2{jpegPath};
    ASSERT_EQ(0, file.write(file2));
}

TEST_F(AClosedFileIo, putbFails)
{
    ASSERT_EQ(EOF, file.putb(5));
}

TEST_F(AClosedFileIo, readFails)
{
    auto buff = file.read(5);
    ASSERT_EQ(0, buff.size_);
}

TEST_F(AClosedFileIo, readIntoArrayFails)
{
    std::array<byte, 4> buff;
    ASSERT_EQ(0, file.read(buff.data(), buff.size()));
}

TEST_F(AClosedFileIo, getbFails)
{
    ASSERT_EQ(EOF, file.getb());
}
