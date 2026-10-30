// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include "basicio.hpp"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;
using namespace Exiv2;

namespace {
constexpr auto imagePath = TESTDATA_PATH "/DSC_3079.jpg";
constexpr auto nonExistingImagePath = TESTDATA_PATH "/nonExisting.jpg";
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

TEST(AFileIO, mappedRegionStaysReadableWhileTheFileIsWritten) {
#ifdef _WIN32
  // write() reopens the file with _wfopen_s, which opens files without
  // sharing, while mmap() holds a duplicate of the read handle. Exiv2 itself
  // only writes to a mapped file after mmap(true).
  GTEST_SKIP() << "FileIo can't reopen a read-only mapped file for writing on Windows";
#endif
  const auto path = fs::path(::testing::TempDir()) / "mmap-test.jpg";
  fs::copy_file(imagePath, path, fs::copy_options::overwrite_existing);
  {
    FileIo file(path.string());
    ASSERT_EQ(0, file.open("rb"));
    const byte* pData = file.mmap();
    ASSERT_NE(nullptr, pData);
    Blob buf(pData, pData + file.size());

    // Switches the file to write mode while it is mapped
    ASSERT_EQ(buf.size(), file.write(buf.data(), buf.size()));

    EXPECT_TRUE(std::equal(buf.begin(), buf.end(), pData));
    file.close();
  }
  fs::remove(path);
}
