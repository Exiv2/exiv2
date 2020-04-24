#include "basicio.hpp"

#include <gtest/gtest.h>

using namespace Exiv2;

namespace
{
    const std::string testData(TESTDATA_PATH);
    const std::string imagePath(testData + "/DSC_3079.jpg");
}  // namespace

TEST(AFileIO, canBeInstantiatedWithFilePath)
{
    ASSERT_NO_THROW(FileIo file(imagePath));
}

TEST(AFileIO, canBeOpenInReadBinaryMode)
{
    FileIo file(imagePath);
    ASSERT_EQ(0, file.open());
}

TEST(AFileIO, isOpenDoItsJob)
{
    FileIo file(imagePath);
    ASSERT_FALSE(file.isopen());
    file.open();
    ASSERT_TRUE(file.isopen());
}

TEST(AFileIO, returnsFileSizeIfItsOpened)
{
    FileIo file(imagePath);
    file.open();
    ASSERT_EQ(118685, file.size());
}

TEST(AFileIO, returnsFileSizeEvenWhenFileItIsNotOpened)
{
    FileIo file(imagePath);
    ASSERT_EQ(118685, file.size());
}

TEST(AFileIO, isOpenedAtPosition0)
{
    FileIo file(imagePath);
    file.open();
    ASSERT_EQ(0, file.tell());
}

TEST(AFileIO, canSeekToExistingPositions)
{
    FileIo file(imagePath);
    file.open();

    ASSERT_EQ(0, file.seek(100, BasicIo::beg));
    ASSERT_EQ(0, file.seek(-50, BasicIo::cur));
    ASSERT_EQ(0, file.seek(-50, BasicIo::end));

    ASSERT_FALSE(file.error());
    ASSERT_FALSE(file.eof());
}

TEST(AFileIO, canSeekBeyondEOF)
{
    FileIo file(imagePath);
    file.open();

    // POSIX allows seeking beyond the existing end of file.
    ASSERT_EQ(0, file.seek(200000, BasicIo::beg));
    ASSERT_FALSE(file.error());
    ASSERT_FALSE(file.eof());
}
