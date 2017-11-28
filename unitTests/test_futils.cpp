// File under test
#include <exiv2/futils.hpp>

// Auxiliary headers
#include <fstream>
#include <cstdio>
#include <cerrno>

#include "gtestwrapper.h"

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

TEST(getEnv, getsDefaultValueWhenExpectedEnvVariableDoesNotExist)
{
    ASSERT_STREQ("/exiv2.php", getEnv(envHTTPPOST).c_str());
    ASSERT_STREQ("40", getEnv(envTIMEOUT).c_str());
}

TEST(getEnv, getsProperValuesWhenExpectedEnvVariableExists)
{
    const char * expectedValue = "test";
    ASSERT_EQ(0, setenv("EXIV2_HTTP_POST", expectedValue, 1));

    ASSERT_STREQ(expectedValue, getEnv(envHTTPPOST).c_str());

    ASSERT_EQ(0, unsetenv("EXIV2_HTTP_POST"));
}

TEST(getEnv, throwsWhenKeyDoesNotExist)
{
    // This is just a characterisation test that was written to see how the current implementation
    // works.
    ASSERT_THROW(getEnv(static_cast<EnVar>(3)), std::exception);
}

TEST(urlencode, encodesGivenUrl)
{
    char *url = urlencode("http://www.geekhideout.com/urlcode.shtml");
    ASSERT_STREQ("http%3a%2f%2fwww.geekhideout.com%2furlcode.shtml", url);
    free(url);
}
