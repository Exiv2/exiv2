#include <exiv2/basicio.hpp>

#include <gtest/gtest.h>

using namespace Exiv2;

namespace
{
    const std::string testData{TESTDATA_PATH};
    const std::string jpegPath{testData + "/DSC_3079.jpg"};
}  // namespace

struct AFileIo : public testing::Test
{
    FileIo file{jpegPath};
};

TEST_F(AFileIo, tellReturnsFailureValueWithClosedFile)
{
    ASSERT_EQ(-1, file.tell());
}
