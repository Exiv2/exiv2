#include <utils.hpp>

// Auxiliary headers

#include <gtest/gtest.h>

namespace  {
    const std::string pathLinux("/home/luis/file.txt");
    const std::string pathWindows("c:\\luis\\file.txt");
}

TEST(dirname, returnsDirNameWithValidPathOnLinux)
{
    ASSERT_EQ("/home/luis", Util::dirname(pathLinux));
}

TEST(dirname, returnsDirNameWithValidPathOnWindows)
{
    ASSERT_EQ("c:\\luis", Util::dirname(pathWindows));
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

TEST(suffix, returnsExtensionWithValidLinuxPath)
{
    ASSERT_EQ(".txt", Util::suffix(pathLinux));
}


TEST(suffix, returnsExtensionWithValidWindowsPath)
{
    ASSERT_EQ(".txt", Util::suffix(pathWindows));
}

TEST(suffix, returnsEmptyStringWithFilesWithoutExtension)
{
    ASSERT_EQ("", Util::suffix("/home/luis/file"));
    ASSERT_EQ("", Util::suffix("c:\\luis\\file"));
}

TEST(startsWith, returnsTrueWhenReferenceStringStartsWithSpecifiedString)
{
    ASSERT_TRUE(Util::startsWith("aabbccdd", "aab"));
    ASSERT_TRUE(Util::startsWith("aabbccdd", "aa"));
    ASSERT_TRUE(Util::startsWith("aabbccdd", "a"));
}

TEST(startsWith, returnsFalseWhenReferenceStringDoesNotStartWithSpecifiedString)
{
    ASSERT_FALSE(Util::startsWith("aabbccdd", "b"));
    ASSERT_FALSE(Util::startsWith("aabbccdd", "ab"));
    ASSERT_FALSE(Util::startsWith("aabbccdd", "aac"));
}

TEST(startsWith, returnsFalseWhenTargetStringIsLongerThanReference)
{
    ASSERT_FALSE(Util::startsWith("aabb", "aabbC"));
}
