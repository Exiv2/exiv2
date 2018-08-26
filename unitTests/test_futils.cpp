// File under test
#include <exiv2/futils.hpp>

// Auxiliary headers
#include <fstream>
#include <cstdio>
#include <cerrno>
#include <stdexcept>

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

TEST(urlencode, encodesGivenUrlWithSpace)
{
    const std::string url = urlencode("http://www.geekhideout.com/url code.shtml");
    ASSERT_STREQ("http%3a%2f%2fwww.geekhideout.com%2furl+code.shtml", url.c_str());
}

TEST(urldecode, decodesGivenUrl)
{
    const std::string expectedDecodedUrl ("http://www.geekhideout.com/urlcode.shtml");
    const std::string url ("http%3a%2f%2fwww.geekhideout.com%2furlcode.shtml");
    char * url3 = urldecode(url.c_str());
    ASSERT_STREQ(expectedDecodedUrl.c_str(), url3);
    free(url3);
}

TEST(urldecode, decodesGivenUrlInPlace)
{
    const std::string expectedDecodedUrl ("http://www.geekhideout.com/urlcode.shtml");
    std::string url ("http%3a%2f%2fwww.geekhideout.com%2furlcode.shtml");
    urldecode(url);
    ASSERT_STREQ(expectedDecodedUrl.c_str(), url.c_str());
}

TEST(base64encode, encodesValidString)
{
    const std::string original ("This is a unit test");
    const std::string expected ("VGhpcyBpcyBhIHVuaXQgdGVzdA==");
    size_t encodeLength = ((original.size() + 2) / 3) * 4 + 1;
    char * result = new char [encodeLength];
    ASSERT_EQ(1, base64encode(original.c_str(), original.size(), result, encodeLength));
    ASSERT_STREQ(expected.c_str(), result);
    delete [] result;
}

TEST(base64encode, doesNotEncodeWithNotBigEnoughResultSize)
{
    const std::string original ("This is a unit test");
    size_t encodeLength = (original.size());
    char * result = new char [encodeLength];
    ASSERT_EQ(0, base64encode(original.c_str(), original.size(), result, encodeLength));
    delete [] result;
}

TEST(base64decode, decodesValidString)
{
    const std::string original ("VGhpcyBpcyBhIHVuaXQgdGVzdA==");
    const std::string expected ("This is a unit test");
    char * result = new char [original.size()];
    ASSERT_EQ(expected.size()+1, base64decode(original.c_str(), result, original.size()));
    ASSERT_STREQ(expected.c_str(), result);
    delete [] result;
}

TEST(AUri, parsesAndDecoreUrl)
{
    const std::string url("http://www.geekhideout.com/urlcode.shtml");
    Uri uri = Uri::Parse(url);

    ASSERT_EQ("", uri.QueryString);
    ASSERT_EQ("http", uri.Protocol);
    ASSERT_EQ("www.geekhideout.com", uri.Host);
    ASSERT_EQ("80", uri.Port);
    ASSERT_EQ("/urlcode.shtml", uri.Path);
    ASSERT_EQ("", uri.Username);
    ASSERT_EQ("", uri.Password);

    Uri::Decode(uri);
}