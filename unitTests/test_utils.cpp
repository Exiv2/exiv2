#include <utils.hpp>

// Auxiliary headers

#include <gtest/gtest.h>

namespace  {
    const std::string pathLinux("/home/luis/file.txt");
    const std::string pathWindows("c:/luis/file.txt");
}

#ifdef _WIN32

TEST(dirname, returnsDirNameWithValidPathOnWindows)
{
    ASSERT_EQ("c:\\luis", Util::dirname(pathWindows));
}
TEST(basename, returnsStemWithExtensionWithValidPathOnWindows)
{
    const bool delSuffix = false;
    ASSERT_EQ("file.txt", Util::basename(pathWindows, delSuffix));
}

TEST(basename, returnsStemWithoutExtensionWithValidPathOnWindows)
{
    const bool delSuffix = true;
    ASSERT_EQ("file", Util::basename(pathWindows, delSuffix));
}

#else

TEST(dirname, returnsDirNameWithValidPathOnLinux)
{
    ASSERT_EQ("/home/luis", Util::dirname(pathLinux));
    ASSERT_EQ("/tmp", Util::dirname("/tmp/file.jpg"));
}


TEST(dirname, returnsDotWithRelativePath)
{
    ASSERT_EQ(".", Util::dirname("file.txt"));
}

TEST(dirname, returnsDotEmptyString)
{
    ASSERT_EQ(".", Util::dirname(""));
}

/// \bug the logic for delsuffix is actually reverted
TEST(basename, returnsStemWithExtensionWithValidPathOnLinux)
{
    const bool delSuffix = false;
    ASSERT_EQ("file.txt", Util::basename(pathLinux, delSuffix));
}

TEST(basename, returnsStemWithoutExtensionWithValidPathOnLinux)
{
    const bool delSuffix = true;
    ASSERT_EQ("file", Util::basename(pathLinux, delSuffix));
}

#endif
