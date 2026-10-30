// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include "basicio.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <limits>
#include <type_traits>
#include <utility>

using namespace Exiv2;

static_assert(std::is_same_v<decltype(std::declval<BasicIo&>().tell()), uint64_t>);
static_assert(std::is_same_v<decltype(std::declval<BasicIo&>().size()), uint64_t>);
static_assert(std::is_same_v<decltype(std::declval<BasicIo&>().write(std::declval<BasicIo&>())), uint64_t>);

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

TEST(AFileIO, failedSizeUsesThe64BitErrorSentinel) {
  FileIo file(nonExistingImagePath);
  EXPECT_EQ(std::numeric_limits<uint64_t>::max(), file.size());
}

#ifdef __linux__
TEST(AFileIO, retainsSparseFileSizesAndPositionsAboveFourGiB) {
  namespace fs = std::filesystem;
  struct TemporaryDirectory {
    fs::path path;
    ~TemporaryDirectory() {
      std::error_code ec;
      fs::remove_all(path, ec);
    }
  } directory{fs::temp_directory_path() /
              ("exiv2-io64-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))};
  ASSERT_TRUE(fs::create_directory(directory.path));
  const auto path = directory.path / "sparse.bin";
  std::ofstream(path, std::ios::binary).put('\0');
  constexpr uint64_t length = (uint64_t{1} << 33) + 123;
  fs::resize_file(path, length);

  FileIo file(path.string());
  ASSERT_EQ(length, file.size());
  ASSERT_EQ(0, file.open());
  ASSERT_EQ(0, file.seek(static_cast<int64_t>(length - 1), BasicIo::beg));
  ASSERT_EQ(length - 1, file.tell());
  ASSERT_EQ(0, file.getb());
  ASSERT_EQ(length, file.tell());

  // Switching from read-only access to writing reopens the file and restores its position.
  ASSERT_EQ(0x42, file.putb(0x42));
  ASSERT_EQ(length + 1, file.tell());
  ASSERT_EQ(length + 1, file.size());
  ASSERT_EQ(0, file.seek(-1, BasicIo::end));
  ASSERT_EQ(0x42, file.getb());

  if constexpr (sizeof(size_t) < sizeof(uint64_t)) {
    EXPECT_THROW(file.mmap(), Error);
    EXPECT_EQ(length + 1, file.size());
  }
}
#endif
