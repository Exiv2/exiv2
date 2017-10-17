// File under test
#include <exiv2/futils.hpp>

// Auxiliary headers
#include <fstream>
#include <cstdio>
#include <cerrno>

#include <gtest/gtest.h>

using namespace Exiv2;

TEST(strError, returnSuccessAfterClosingFile)
{
    std::string tmpFile("tmp.dat");
    std::ofstream auxFile(tmpFile.c_str());
    auxFile.close();
    ASSERT_STREQ("Success (errno = 0)", strError().c_str());
    std::remove(tmpFile.c_str());
}

TEST(strError, returnNoSuchFileOrDirectoryWhenTryingToOpenNonExistingFile)
{
    std::ifstream auxFile("nonExistingFile");
    ASSERT_STREQ("No such file or directory (errno = 2)", strError().c_str());
}

TEST(strError, doNotRecognizeUnknownError)
{
    errno = 9999;
    ASSERT_STREQ("Unknown error 9999 (errno = 9999)", strError().c_str());
}
