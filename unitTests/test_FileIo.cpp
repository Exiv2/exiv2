// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include "basicio.hpp"
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
