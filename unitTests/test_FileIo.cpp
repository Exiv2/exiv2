// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "basicio.hpp"
using namespace Exiv2;

namespace {
constexpr auto imagePath = TESTDATA_PATH "/DSC_3079.jpg";
constexpr auto nonExistingImagePath = TESTDATA_PATH "/nonExisting.jpg";

// A MemIo that always fails to open, standing in for any BasicIo source
// (MemIo in practice - see JpegBase::writeMetadata and friends) that fails
// partway through a metadata write, after FileIo::transfer() has already
// been called on the target.
class FailingOpenMemIo : public MemIo {
 public:
  int open() override {
    return -1;
  }
};
}  // namespace

// ---------------------------------------------------------------------
// Regression tests for issue #9482: FileIo::transfer()'s generic branch
// (source is not itself a FileIo, e.g. the MemIo that every image writer
// builds new metadata into) used to reopen the target with "w+b", which
// truncates it, before any new bytes were written. A failure between that
// truncate and the write completing left the target as an unrecoverable
// zero-byte file. The fix writes into a temporary file next to the target
// and renames it into place only once the write is known to have
// succeeded, so the target is never truncated in place.
// ---------------------------------------------------------------------

TEST(AFileIO, transferGenericBranchPreservesOriginalOnSourceOpenFailure) {
  namespace fs = std::filesystem;
  const auto dir = fs::temp_directory_path() / "exiv2_test_issue_9482_open_failure";
  std::error_code ec;
  fs::remove_all(dir, ec);
  fs::create_directories(dir);
  const auto victim = dir / "original.bin";
  const std::string originalBytes = "ORIGINAL-CONTENTS-9482";
  {
    std::ofstream ofs(victim, std::ios::binary);
    ofs << originalBytes;
  }
  ASSERT_TRUE(fs::exists(victim));

  FailingOpenMemIo src;  // not a FileIo, so transfer() takes the generic branch
  FileIo target(victim.string());
  EXPECT_THROW(target.transfer(src), Error);

  // The original file must be untouched: same size, same bytes.
  ASSERT_TRUE(fs::exists(victim));
  EXPECT_EQ(fs::file_size(victim), originalBytes.size());
  std::ifstream ifs(victim, std::ios::binary);
  const std::string gotBytes((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  EXPECT_EQ(gotBytes, originalBytes);

  // No leftover temporary file in the directory.
  size_t entries = 0;
  for (const auto& entry : fs::directory_iterator(dir)) {
    ++entries;
    EXPECT_EQ(entry.path().filename(), victim.filename());
  }
  EXPECT_EQ(entries, 1u);

  fs::remove_all(dir, ec);
}

TEST(AFileIO, transferGenericBranchReplacesContentOnSuccess) {
  namespace fs = std::filesystem;
  const auto dir = fs::temp_directory_path() / "exiv2_test_issue_9482_success";
  std::error_code ec;
  fs::remove_all(dir, ec);
  fs::create_directories(dir);
  const auto victim = dir / "original.bin";
  {
    std::ofstream ofs(victim, std::ios::binary);
    ofs << "OLD-CONTENTS";
  }

  const std::string newBytes = "NEW-CONTENTS-REPLACING-THE-OLD-ONE";
  MemIo src(reinterpret_cast<const byte*>(newBytes.data()), newBytes.size());
  FileIo target(victim.string());
  ASSERT_NO_THROW(target.transfer(src));

  ASSERT_TRUE(fs::exists(victim));
  EXPECT_EQ(fs::file_size(victim), newBytes.size());
  std::ifstream ifs(victim, std::ios::binary);
  const std::string gotBytes((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
  EXPECT_EQ(gotBytes, newBytes);

  // No leftover temporary file in the directory.
  size_t entries = 0;
  for (const auto& entry : fs::directory_iterator(dir)) {
    ++entries;
    EXPECT_EQ(entry.path().filename(), victim.filename());
  }
  EXPECT_EQ(entries, 1u);

  fs::remove_all(dir, ec);
}

// tests/bugfixes/redmine/test_issue_812.py requires that writing metadata
// through one hard-linked name is visible through every other hard link to
// the same file. Replacing the target via temp-file-plus-rename would give
// it a fresh inode and silently leave other hard links pointing at the old,
// unwritten content, so FileIo::transfer() must keep writing in place (not
// through a rename) whenever the target has other hard links.
TEST(AFileIO, transferGenericBranchPreservesHardLinks) {
  namespace fs = std::filesystem;
  const auto dir = fs::temp_directory_path() / "exiv2_test_issue_9482_hardlinks";
  std::error_code ec;
  fs::remove_all(dir, ec);
  fs::create_directories(dir);
  const auto victim = dir / "original.bin";
  const auto link = dir / "hardlink.bin";
  {
    std::ofstream ofs(victim, std::ios::binary);
    ofs << "OLD-CONTENTS";
  }
  fs::create_hard_link(victim, link, ec);
  ASSERT_FALSE(ec) << "test setup: hard links must be supported at " << dir;
  ASSERT_EQ(fs::hard_link_count(victim), 2u);

  const std::string newBytes = "NEW-CONTENTS-VIA-THE-FIRST-NAME";
  MemIo src(reinterpret_cast<const byte*>(newBytes.data()), newBytes.size());
  FileIo target(victim.string());
  ASSERT_NO_THROW(target.transfer(src));

  // Both names must still refer to the same file, and both must see the
  // new content.
  EXPECT_EQ(fs::hard_link_count(victim), 2u);
  for (const auto& name : {victim, link}) {
    ASSERT_TRUE(fs::exists(name));
    EXPECT_EQ(fs::file_size(name), newBytes.size());
    std::ifstream ifs(name, std::ios::binary);
    const std::string gotBytes((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_EQ(gotBytes, newBytes);
  }

  fs::remove_all(dir, ec);
}

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
