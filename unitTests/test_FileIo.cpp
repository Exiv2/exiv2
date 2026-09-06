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

// ---------------------------------------------------------------------
// Regression: issue #9482
// FileIo::transfer's generic branch used to open the target with "w+b"
// (which truncates) BEFORE writing the new content. If anything failed
// between the truncate and the write, the original image was gone: a
// zero-byte file with no backup. The fix writes the new bytes into a
// temporary file next to the target and renames it in atomically.
// ---------------------------------------------------------------------

#include <fstream>
#include <filesystem>

namespace {
// A source that behaves like a MemIo but refuses to open, simulating a
// mid-transfer failure BEFORE any bytes could reach the target.
class FailingOpenMemIo : public Exiv2::MemIo {
 public:
  int open() override { return -1; }
};
}  // namespace

TEST(AFileIO, transferGenericBranchPreservesOriginalOnSourceOpenFailure_9482) {
  namespace fs = std::filesystem;
  auto dir = fs::temp_directory_path() / "exiv2_9482_repro";
  std::error_code ec;
  fs::remove_all(dir, ec);
  fs::create_directories(dir);
  auto orig = dir / "victim.bin";
  const std::string original_bytes = "ORIGINAL_CONTENTS_9482";
  {
    std::ofstream ofs(orig, std::ios::binary);
    ofs << original_bytes;
  }
  ASSERT_TRUE(fs::exists(orig));
  ASSERT_EQ(fs::file_size(orig), original_bytes.size());

  FailingOpenMemIo src;  // else branch: not a FileIo, open() will fail
  Exiv2::FileIo victim(orig.string());
  EXPECT_THROW(victim.transfer(src), Exiv2::Error);

  // Regression invariant for #9482: the original file must still be readable
  // and byte-identical after a failed transfer through the generic branch.
  ASSERT_TRUE(fs::exists(orig));
  EXPECT_EQ(fs::file_size(orig), original_bytes.size());
  std::ifstream in(orig, std::ios::binary);
  std::string got((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  EXPECT_EQ(got, original_bytes);

  // No temp litter left behind.
  for (const auto& entry : fs::directory_iterator(dir)) {
    EXPECT_EQ(entry.path().filename(), std::string("victim.bin"));
  }
  fs::remove_all(dir, ec);
}
