// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/basicio.hpp>

#include <array>

using namespace Exiv2;

TEST(MemIoDefault, readEReturns0) {
  std::array<byte, 10> buf;
  MemIo io;
  ASSERT_EQ(0, io.read(buf.data(), buf.size()));
}

TEST(MemIoDefault, isNotAtEof) {
  MemIo io;
  ASSERT_FALSE(io.eof());
}

TEST(MemIoDefault, seekBeyondBufferSizeReturns1AndSetsEofToTrue) {
  MemIo io;
  ASSERT_EQ(1, io.seek(1, BasicIo::beg));
  ASSERT_TRUE(io.eof());
}

TEST(MemIoDefault, seekBefore0Returns1ButItDoesNotSetEofToTrue) {
  MemIo io;
  ASSERT_EQ(1, io.seek(-1, BasicIo::beg));
  ASSERT_FALSE(io.eof());
}

TEST(MemIoDefault, seekToEndPositionDoesNotTriggerEof) {
  MemIo io;
  ASSERT_EQ(0, io.tell());
  ASSERT_EQ(0, io.seek(0, BasicIo::end));
  ASSERT_EQ(0, io.tell());
  ASSERT_FALSE(io.eof());
}

TEST(MemIoDefault, seekToEndPositionAndReadTriggersEof) {
  MemIo io;
  ASSERT_EQ(0, io.seek(0, BasicIo::end));
  ASSERT_EQ(0, io.tell());

  std::array<byte, 64> buf2 = {};
  ASSERT_EQ(0, io.read(buf2.data(), 1));  // Note that we cannot even read 1 byte being at the end
  ASSERT_TRUE(io.eof());
}

// -------------------------

TEST(MemIo, isNotAtEofInitially) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  ASSERT_FALSE(io.eof());
}

TEST(MemIo, seekBeyondBufferSizeReturns1AndSetsEofToTrue) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  ASSERT_EQ(1, io.seek(65, BasicIo::beg));
  ASSERT_TRUE(io.eof());
}

TEST(MemIo, seekBefore0Returns1ButItDoesNotSetEofToTrue) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  ASSERT_EQ(1, io.seek(-1, BasicIo::beg));
  ASSERT_FALSE(io.eof());
}

TEST(MemIo, seekBeyondBoundsDoesNotMoveThePosition) {
  std::array<byte, 64> buf;
  buf.fill(0);

  MemIo io(buf.data(), buf.size());
  ASSERT_EQ(0, io.tell());
  ASSERT_EQ(1, io.seek(65, BasicIo::beg));
  ASSERT_EQ(0, io.tell());
}

TEST(MemIo, seekInsideBoundsMoveThePosition) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  ASSERT_EQ(0, io.tell());
  ASSERT_EQ(0, io.seek(32, BasicIo::beg));
  ASSERT_EQ(32, io.tell());
}

TEST(MemIo, seekInsideBoundsUsingBegResetsThePosition) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  std::vector<std::int64_t> positions{0, 8, 16, 32, 64};
  for (auto pos : positions) {
    ASSERT_EQ(0, io.seek(pos, BasicIo::beg));
    ASSERT_EQ(pos, io.tell());
  }
}

TEST(MemIo, seekInsideBoundsUsingCurShiftThePosition) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  std::vector<std::int64_t> shifts{4, 4, 8, 16, 32};
  std::vector<std::int64_t> positions{4, 8, 16, 32, 64};
  for (size_t i = 0; i < shifts.size(); ++i) {
    ASSERT_EQ(0, io.seek(shifts[i], BasicIo::cur));
    ASSERT_EQ(positions[i], io.tell());
  }
}

TEST(MemIo, seekToEndPositionDoesNotTriggerEof) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  ASSERT_EQ(0, io.tell());
  ASSERT_EQ(0, io.seek(0, BasicIo::end));
  ASSERT_EQ(64, io.tell());
  ASSERT_FALSE(io.eof());
}

TEST(MemIo, seekToEndPositionAndReadTriggersEof) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  ASSERT_EQ(0, io.seek(0, BasicIo::end));
  ASSERT_EQ(64, io.tell());

  std::array<byte, 64> buf2 = {};
  ASSERT_EQ(0, io.read(buf2.data(), 1));  // Note that we cannot even read 1 byte being at the end
  ASSERT_TRUE(io.eof());
}

TEST(MemIo, readEmptyIoReturns0) {
  std::array<byte, 10> buf;
  MemIo io;
  ASSERT_EQ(0, io.read(buf.data(), buf.size()));
}

TEST(MemIo, readLessBytesThanAvailableReturnsRequestedBytes) {
  std::array<byte, 10> buf1;
  std::array<byte, 10> buf2 = {};
  buf1.fill(1);

  MemIo io(buf1.data(), buf1.size());
  ASSERT_EQ(5, io.read(buf2.data(), 5));
}

TEST(MemIo, readSameBytesThanAvailableReturnsRequestedBytes) {
  std::array<byte, 10> buf1;
  std::array<byte, 10> buf2 = {};
  buf1.fill(1);

  MemIo io(buf1.data(), buf1.size());
  ASSERT_EQ(10, io.read(buf2.data(), 10));
}

TEST(MemIo, readMoreBytesThanAvailableReturnsAvailableBytes) {
  std::array<byte, 10> buf1;
  std::array<byte, 10> buf2 = {};
  buf1.fill(1);

  MemIo io(buf1.data(), buf1.size());
  ASSERT_EQ(10, io.read(buf2.data(), 15));
}
