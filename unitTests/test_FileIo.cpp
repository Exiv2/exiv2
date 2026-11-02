// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include "basicio.hpp"

#ifdef _WIN32
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <optional>
#endif
using namespace Exiv2;

namespace {
const std::string testData(TESTDATA_PATH);
const std::string imagePath(testData + "/DSC_3079.jpg");
const std::string nonExistingImagePath(testData + "/nonExisting.jpg");
}  // namespace

TEST(AFileIO, canBeInstantiatedWithFilePath) {
  ASSERT_NO_THROW(FileIo file(imagePath));
}

TEST(AFileIO, canBeOpenInReadBinaryMode) {
  FileIo file(imagePath);
  ASSERT_EQ(0, file.open());
}

TEST(AFileIO, isOpenDoItsJob) {
  FileIo file(imagePath);
  ASSERT_FALSE(file.isopen());
  ASSERT_EQ(0, file.open());
  ASSERT_TRUE(file.isopen());
}

TEST(AFileIO, failsToOpenANonExistingFile) {
  FileIo file(nonExistingImagePath);
  ASSERT_FALSE(file.isopen());
  ASSERT_EQ(1, file.open());
  ASSERT_FALSE(file.isopen());
}

TEST(AFileIO, canChangeItsPathWithSetPath) {
  FileIo file(nonExistingImagePath);
  ASSERT_EQ(nonExistingImagePath, file.path());
  file.setPath(imagePath);
  ASSERT_EQ(imagePath, file.path());
}

TEST(AFileIO, returnsFileSizeIfItsOpened) {
  FileIo file(imagePath);
  file.open();
  ASSERT_EQ(118685UL, file.size());
}

TEST(AFileIO, returnsFileSizeEvenWhenFileItIsNotOpened) {
  FileIo file(imagePath);
  ASSERT_EQ(118685UL, file.size());
}

TEST(AFileIO, isOpenedAtPosition0) {
  FileIo file(imagePath);
  file.open();
  ASSERT_EQ(0u, file.tell());
}

TEST(AFileIO, canSeekToExistingPositions) {
  FileIo file(imagePath);
  file.open();

  ASSERT_EQ(0, file.seek(100, BasicIo::beg));
  ASSERT_EQ(0, file.seek(-50, BasicIo::cur));
  ASSERT_EQ(0, file.seek(-50, BasicIo::end));

  ASSERT_FALSE(file.error());
  ASSERT_FALSE(file.eof());
}

TEST(AFileIO, canSeekBeyondEOF) {
  FileIo file(imagePath);
  file.open();

  // POSIX allows seeking beyond the existing end of file.
  ASSERT_EQ(0, file.seek(200000, BasicIo::beg));
  ASSERT_FALSE(file.error());
  ASSERT_FALSE(file.eof());
}

#ifdef _WIN32
namespace {
// path in codePage (CP_UTF8, or CP_ACP - Windows' code page); nothing if codePage cannot name it
std::optional<std::string> narrowName(const std::wstring& path, UINT codePage) {
  BOOL usedDefaultChar = FALSE;
  BOOL* usedDefault = codePage == CP_UTF8 ? nullptr : &usedDefaultChar;  // CP_UTF8 takes none
  const int n =
      WideCharToMultiByte(codePage, 0, path.data(), static_cast<int>(path.size()), nullptr, 0, nullptr, usedDefault);
  std::string result(n, char{});
  WideCharToMultiByte(codePage, 0, path.data(), static_cast<int>(path.size()), result.data(), n, nullptr, usedDefault);
  if (usedDefaultChar)
    return std::nullopt;
  return result;
}

bool isUtf8(const std::string& s) {
  return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), static_cast<int>(s.size()), nullptr, 0) != 0;
}

// A 3-byte file, name in the temporary directory, while it lives
struct TemporaryFile {
  explicit TemporaryFile(const std::wstring& name) : path(std::filesystem::temp_directory_path() / name) {
    std::ofstream(path, std::ios::binary) << "abc";
  }
  ~TemporaryFile() {
    std::filesystem::remove(path);
  }
  std::filesystem::path path;
};
}  // namespace

// On Windows, a path in UTF-8 names any file - here one whose name is outside Windows-1252 (and most code pages)
TEST(AFileIO, opensAFileByItsUtf8NameOnWindows) {
  TemporaryFile f(L"exiv2-\u03a9mega.dat");
  FileIo file(*narrowName(f.path.wstring(), CP_UTF8));
  EXPECT_EQ(3u, file.size());  // by its name, before it is open
  ASSERT_EQ(0, file.open());
  EXPECT_EQ(3u, file.size());
}

// ... and one that is not UTF-8 is in the code page, as before
TEST(AFileIO, opensAFileByItsCodePageNameOnWindows) {
  TemporaryFile f(L"exiv2-Zo\u00eb.dat");
  auto name = narrowName(f.path.wstring(), CP_ACP);
  if (!name || isUtf8(*name))
    GTEST_SKIP() << "the code page cannot name it, or names it in valid UTF-8";
  FileIo file(*name);
  EXPECT_EQ(3u, file.size());
  ASSERT_EQ(0, file.open());
  EXPECT_EQ(3u, file.size());
}

// ... as is one in the code page that is also valid UTF-8, when its UTF-8 meaning names no file
TEST(AFileIO, opensAFileByItsCodePageNameThatIsAlsoUtf8OnWindows) {
  TemporaryFile f(L"exiv2-\u00c3\u00a9.dat");  // in Windows-1252, the UTF-8 of U+00E9
  auto name = narrowName(f.path.wstring(), CP_ACP);
  if (!name || !isUtf8(*name))
    GTEST_SKIP() << "the code page cannot name it, or not in valid UTF-8";
  FileIo file(*name);
  EXPECT_EQ(3u, file.size());
  ASSERT_EQ(0, file.open());
  EXPECT_EQ(3u, file.size());
}
#endif
