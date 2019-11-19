#include <exiv2/basicio.hpp>

#include <gtest/gtest.h>

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
