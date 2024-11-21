// SPDX-License-Identifier: GPL-2.0-or-later

#include "utils.hpp"

#include <exiv2/exiv2.hpp>
// File under test
#include <exiv2/futils.hpp>

// Auxiliary headers
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <stdexcept>

#include <gtest/gtest.h>

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

using namespace Exiv2;

TEST(strError, returnSuccessAfterClosingFile) {
  // previous system calls can fail, but errno is not guaranteed to be reset
  // by a successful system call
  // -> reset errno so that a real failure is only detected here
  errno = 0;

  fs::path tmpFile("tmp.dat");
  std::ofstream auxFile(tmpFile);
  auxFile.close();
  fs::remove(tmpFile);
  ASSERT_TRUE(Internal::contains(strError(), "(errno = 0)"));
}

TEST(strError, returnNoSuchFileOrDirectoryWhenTryingToOpenNonExistingFile) {
  std::ifstream auxFile("nonExistingFile");
  ASSERT_TRUE(Internal::contains(strError(), "No such file or directory (errno = "));
}

TEST(strError, doNotRecognizeUnknownError) {
  errno = 9999;
  ASSERT_TRUE(Internal::contains(strError(), "(errno = 9999)"));
}

TEST(getEnv, getsDefaultValueWhenExpectedEnvVariableDoesNotExist) {
  ASSERT_STREQ("/exiv2.php", getEnv(envHTTPPOST).c_str());
  ASSERT_STREQ("40", getEnv(envTIMEOUT).c_str());
}

TEST(getEnv, getsProperValuesWhenExpectedEnvVariableExists) {
  const char* expectedValue = "test";
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

TEST(getEnv, throwsWhenKeyDoesNotExist) {
  ASSERT_THROW(getEnv(static_cast<EnVar>(3)), std::out_of_range);
}

TEST(urlencode, encodesGivenUrl) {
  const std::string url = urlencode("http://www.geekhideout.com/urlcode.shtml");
  ASSERT_STREQ("http%3a%2f%2fwww.geekhideout.com%2furlcode.shtml", url.c_str());
}

TEST(urlencode, encodesGivenUrlWithSpace) {
  const std::string url = urlencode("http://www.geekhideout.com/url code.shtml");
  ASSERT_STREQ("http%3a%2f%2fwww.geekhideout.com%2furl+code.shtml", url.c_str());
}

TEST(urldecode, decodesGivenUrlInPlace) {
  const std::string expectedDecodedUrl("http://www.geekhideout.com/urlcode.shtml");
  std::string url("http%3a%2f%2fwww.geekhideout.com%2furlcode.shtml");
  urldecode(url);
  ASSERT_STREQ(expectedDecodedUrl.c_str(), url.c_str());
}

TEST(base64encode, encodesValidString) {
  const std::string original("This is a unit test");
  const std::string expected("VGhpcyBpcyBhIHVuaXQgdGVzdA==");
  size_t encodeLength = ((original.size() + 2) / 3) * 4 + 1;
  std::vector<char> result(encodeLength);
  ASSERT_EQ(1, base64encode(original.c_str(), original.size(), result.data(), encodeLength));
  ASSERT_STREQ(expected.c_str(), result.data());
}

TEST(base64encode, doesNotEncodeWithNotBigEnoughResultSize) {
  const std::string original("This is a unit test");
  size_t encodeLength = (original.size());
  std::vector<char> result(encodeLength);
  ASSERT_EQ(0, base64encode(original.c_str(), original.size(), result.data(), encodeLength));
}

TEST(base64decode, decodesValidString) {
  const std::string original("VGhpcyBpcyBhIHVuaXQgdGVzdA==");
  const std::string expected("This is a unit test");
  std::vector<char> result(original.size());
  ASSERT_EQ(expected.size(), base64decode(original.c_str(), result.data(), original.size()));
  ASSERT_STREQ(expected.c_str(), result.data());
}

TEST(AUri, parsesAndDecoreUrl) {
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

#if 0
//1122 This has been removed for v0.27.3
//     On MinGW:
//     path     = C:\msys64\home\rmills\gnu\github\exiv2\buildserver\build\bin\unit_tests.exe
//     expected = bin
//     I don't know how this could work successfully on any platform!
TEST(getProcessPath, obtainPathOfUnitTestsExecutable)
{
#ifdef _WIN32
    const std::string expectedName("bin");
#else
    const std::string expectedName("bin");
#endif
    const std::string path = getProcessPath();

    FILE* f = fopen("/c//temp/test_futils.log","w");
    fprintf(f,"path     = %s\n",path.c_str()        );
    fprintf(f,"expected = %s\n",expectedName.c_str());
    fclose(f);

    ASSERT_FALSE(path.empty());
    const size_t idxStart = path.size() - expectedName.size();
    ASSERT_EQ(expectedName, path.substr(idxStart, expectedName.size()));
}
#endif
