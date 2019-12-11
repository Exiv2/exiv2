#include <exiv2/basicio.hpp> // SUT
#include <exiv2/error.hpp>

#include <gtest/gtest.h>

#include <array>

using namespace Exiv2;

namespace
{
    const std::string testData{TESTDATA_PATH};
    const std::string jpegPath{testData + "/DSC_3079.jpg"};
    const std::string tmpPath{"tmp.jpg"};
    const size_t fileSize{118685};

    struct AClosedFileIo : public testing::Test
    {
        void SetUp() override
        {
            ASSERT_FALSE(file.isopen());
            ASSERT_EQ(0, file.error());
        }

        FileIo file{jpegPath};
    };

    struct AOpenedFileIo : public testing::Test
    {
        void SetUp() override
        {
            // In this setup we copy the test file to a temporary one, so that we do not risk destroying data
            FileIo original{jpegPath};
            ASSERT_EQ(0, original.open());
            ASSERT_EQ(0, file.open("wb"));

            ASSERT_EQ(original.size(), file.write(original));
            file.seek(0, BasicIo::beg);
        }

        void TearDown() override
        {
            if (file.isopen()) {
                ASSERT_EQ(0, file.close());
                ASSERT_EQ(0, std::remove(tmpPath.c_str()));
            }
        }

        FileIo file{tmpPath};
    };

}  // namespace

TEST_F(AClosedFileIo, tellReturnsFailureValue)
{
    ASSERT_EQ(-1, file.tell());
}

TEST_F(AClosedFileIo, sizeReturnsImageSize)
{
    ASSERT_EQ(fileSize, file.size());
}

TEST_F(AClosedFileIo, eofReturnsTrue)
{
    ASSERT_TRUE(file.eof());
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


// -------------------------------------------------------------------------


TEST_F(AOpenedFileIo, tellReturns0AfterFileIsOpened)
{
    ASSERT_EQ(0, file.tell());
}

TEST_F(AOpenedFileIo, tellReturnsExpectedPositionAfterUsingSeek)
{
    const int64 offset{666};
    file.seek(offset, BasicIo::beg);
    ASSERT_EQ(offset, file.tell());

    file.seek(4, BasicIo::cur);
    ASSERT_EQ(offset + 4, file.tell());

    file.seek(4, BasicIo::beg);
    ASSERT_EQ(4, file.tell());
}

TEST_F(AOpenedFileIo, sizeReturnsImageSize)
{
    ASSERT_EQ(fileSize, file.size());
}

TEST_F(AOpenedFileIo, readWorksWhenCountIsSmallerThanSize)
{
    const auto databuf = file.read(1000);
    ASSERT_EQ(1000, databuf.size_);
    ASSERT_FALSE(file.eof());
}

TEST_F(AOpenedFileIo, readFailsWhenCountIsBiggerThanSize_versionDataBuf)
{
    const auto databuf = file.read(200000);
    ASSERT_EQ(0, databuf.size_);
}

TEST_F(AOpenedFileIo, readFailsWhenCountIsBiggerThanSize_versionBuffer)
{
    std::array<byte, 200000> buf;
    ASSERT_EQ(file.size(), file.read(buf.data(), buf.size()));
    ASSERT_TRUE(file.eof());
}

TEST_F(AOpenedFileIo, readOrThrowThrowsWhenCountIsBiggerThanSize)
{
    std::array<byte, 200000> buf;
    ASSERT_THROW(file.readOrThrow(buf.data(), buf.size()), Error);
}

TEST_F(AOpenedFileIo, canBeTransferedToOtherFile)
{
    FileIo file2("tmp2.jpg");
    file2.transfer(file);

    ASSERT_FALSE(file.isopen());
    ASSERT_EQ(0, std::remove("tmp2.jpg"));
}

TEST_F(AOpenedFileIo, canBeTransferedToAMemIo)
{
    MemIo mem;
    mem.transfer(file);

    ASSERT_FALSE(file.isopen());
    ASSERT_EQ(fileSize, mem.size());
}
