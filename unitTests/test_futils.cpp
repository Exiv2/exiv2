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
    // previous system calls can fail, but errno is not guaranteed to be reset
    // by a successful system call
    // -> reset errno so that a real failure is only detected here
    errno = 0;
    std::string tmpFile("tmp.dat");
    std::ofstream auxFile(tmpFile.c_str());
    auxFile.close();
#ifdef _WIN32
    const char * expectedString = "No error (errno = 0)";
#elif __APPLE__
    const char * expectedString = "Undefined error: 0 (errno = 0)";
#else
    const char * expectedString = "Success (errno = 0)";
#endif

    ASSERT_STREQ(expectedString, strError().c_str());
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
#ifdef _WIN32
    const char * expectedString = "Unknown error (errno = 9999)";
#elif __APPLE__
    const char * expectedString = "Unknown error: 9999 (errno = 9999)";
#else
    const char * expectedString = "Unknown error 9999 (errno = 9999)";
#endif
    ASSERT_STREQ(expectedString, strError().c_str());
}

TEST(getEnv, getsDefaultValueWhenExpectedEnvVariableDoesNotExist)
{
    ASSERT_STREQ("/exiv2.php", getEnv(envHTTPPOST).c_str());
    ASSERT_STREQ("40", getEnv(envTIMEOUT).c_str());
}

TEST(getEnv, getsProperValuesWhenExpectedEnvVariableExists)
{
    const char * expectedValue = "test";
#ifdef _WIN32
    ASSERT_EQ(0, _putenv_s("EXIV2_HTTP_POST", expectedValue));
#else
    ASSERT_EQ(0, setenv("EXIV2_HTTP_POST", expectedValue, 1));
#endif

    ASSERT_STREQ(expectedValue, getEnv(envHTTPPOST).c_str());

#ifndef _WIN32
    ASSERT_EQ(0, unsetenv("EXIV2_HTTP_POST"));
#endif
}

TEST(getEnv, throwsWhenKeyDoesNotExist)
{
    ASSERT_THROW(getEnv(static_cast<EnVar>(3)), std::out_of_range);
}

TEST(urlencode, encodesGivenUrl)
{
    const std::string url = urlencode("http://www.geekhideout.com/urlcode.shtml");
    ASSERT_STREQ("http%3a%2f%2fwww.geekhideout.com%2furlcode.shtml", url.c_str());
}
