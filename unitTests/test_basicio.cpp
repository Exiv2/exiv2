// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/basicio.hpp>

#include <array>
#include <vector>

using namespace Exiv2;

TEST(MemIoDefault, readEReturns0) {
  std::array<byte, 10> buf;
  MemIo io;
  ASSERT_EQ(0u, io.read(buf.data(), buf.size()));
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
  ASSERT_EQ(0u, io.tell());
  ASSERT_EQ(0, io.seek(0, BasicIo::end));
  ASSERT_EQ(0u, io.tell());
  ASSERT_FALSE(io.eof());
}

TEST(MemIoDefault, seekToEndPositionAndReadTriggersEof) {
  MemIo io;
  ASSERT_EQ(0, io.seek(0, BasicIo::end));
  ASSERT_EQ(0u, io.tell());

  std::array<byte, 64> buf2 = {};
  ASSERT_EQ(0u, io.read(buf2.data(), 1));  // Note that we cannot even read 1 byte being at the end
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
  ASSERT_EQ(0u, io.tell());
  ASSERT_EQ(1, io.seek(65, BasicIo::beg));
  ASSERT_EQ(0u, io.tell());
}

TEST(MemIo, seekInsideBoundsMoveThePosition) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  ASSERT_EQ(0u, io.tell());
  ASSERT_EQ(0, io.seek(32, BasicIo::beg));
  ASSERT_EQ(32u, io.tell());
}

TEST(MemIo, seekInsideBoundsUsingBegResetsThePosition) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  std::vector<std::uint64_t> positions{0, 8, 16, 32, 64};
  for (auto pos : positions) {
    ASSERT_EQ(0, io.seek(pos, BasicIo::beg));
    ASSERT_EQ(pos, io.tell());
  }
}

TEST(MemIo, seekInsideBoundsUsingCurShiftThePosition) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  std::vector<std::int64_t> shifts{4, 4, 8, 16, 32};
  std::vector<std::uint64_t> positions{4, 8, 16, 32, 64};
  for (size_t i = 0; i < shifts.size(); ++i) {
    ASSERT_EQ(0, io.seek(shifts[i], BasicIo::cur));
    ASSERT_EQ(positions[i], io.tell());
  }
}

TEST(MemIo, seekToEndPositionDoesNotTriggerEof) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  ASSERT_EQ(0u, io.tell());
  ASSERT_EQ(0, io.seek(0, BasicIo::end));
  ASSERT_EQ(64u, io.tell());
  ASSERT_FALSE(io.eof());
}

TEST(MemIo, seekToEndPositionAndReadTriggersEof) {
  std::array<byte, 64> buf = {};

  MemIo io(buf.data(), buf.size());
  ASSERT_EQ(0, io.seek(0, BasicIo::end));
  ASSERT_EQ(64u, io.tell());

  std::array<byte, 64> buf2 = {};
  ASSERT_EQ(0u, io.read(buf2.data(), 1));  // Note that we cannot even read 1 byte being at the end
  ASSERT_TRUE(io.eof());
}

TEST(MemIo, readEmptyIoReturns0) {
  std::array<byte, 10> buf;
  MemIo io;
  ASSERT_EQ(0u, io.read(buf.data(), buf.size()));
}

TEST(MemIo, readLessBytesThanAvailableReturnsRequestedBytes) {
  std::array<byte, 10> buf1;
  std::array<byte, 10> buf2 = {};
  buf1.fill(1);

  MemIo io(buf1.data(), buf1.size());
  ASSERT_EQ(5u, io.read(buf2.data(), 5));
}

TEST(MemIo, readSameBytesThanAvailableReturnsRequestedBytes) {
  std::array<byte, 10> buf1;
  std::array<byte, 10> buf2 = {};
  buf1.fill(1);

  MemIo io(buf1.data(), buf1.size());
  ASSERT_EQ(10u, io.read(buf2.data(), 10));
}

TEST(MemIo, readMoreBytesThanAvailableReturnsAvailableBytes) {
  std::array<byte, 10> buf1;
  std::array<byte, 10> buf2 = {};
  buf1.fill(1);

  MemIo io(buf1.data(), buf1.size());
  ASSERT_EQ(10u, io.read(buf2.data(), 15));
}

TEST(MemIo, transferUpdatesAllocatedCapacityFromEmptyDestination) {
  MemIo src;
  std::vector<byte> initial(1000, 0x41);
  ASSERT_EQ(initial.size(), src.write(initial.data(), initial.size()));
  ASSERT_EQ(EOF, src.getb());
  ASSERT_TRUE(src.eof());

  MemIo dst;
  ASSERT_EQ(EOF, dst.getb());
  ASSERT_TRUE(dst.eof());

  dst.transfer(src);
  ASSERT_EQ(initial.size(), dst.size());
  ASSERT_EQ(0u, src.size());
  ASSERT_FALSE(dst.eof());
  ASSERT_FALSE(src.eof());

  // Growing dst beyond src's initial allocation must reallocate using src's transferred sizeAlloced_
  // rather than dividing by zero with dst's stale sizeAlloced_ == 0.
  std::vector<byte> growth(40000, 0x42);
  ASSERT_EQ(growth.size(), dst.write(growth.data(), growth.size()));
  ASSERT_EQ(growth.size(), dst.size());
}

TEST(MemIo, transferUpdatesAllocatedCapacityWhenShrinking) {
  MemIo dst;
  std::vector<byte> large(70000, 0x41);  // allocates > 32 KiB
  ASSERT_EQ(large.size(), dst.write(large.data(), large.size()));

  MemIo src;
  std::vector<byte> small(1000, 0x42);  // allocates 32 KiB
  ASSERT_EQ(small.size(), src.write(small.data(), small.size()));

  dst.transfer(src);
  ASSERT_EQ(small.size(), dst.size());

  // Writing more than src's 32 KiB allocation must reallocate rather than overflowing past the
  // adopted buffer because of dst's stale pre-transfer sizeAlloced_.
  std::vector<byte> payload(50000, 0x43);
  ASSERT_EQ(payload.size(), dst.write(payload.data(), payload.size()));
  ASSERT_EQ(payload.size(), dst.size());
  ASSERT_EQ(0, dst.seek(0, BasicIo::beg));
  std::vector<byte> readBack(payload.size(), 0);
  ASSERT_EQ(payload.size(), dst.read(readBack.data(), readBack.size()));
  ASSERT_EQ(payload, readBack);
}

TEST(MemIo, transferFromSelfPreservesContents) {
  MemIo io;
  std::vector<byte> data(64, 0x55);
  ASSERT_EQ(data.size(), io.write(data.data(), data.size()));
  ASSERT_EQ(EOF, io.getb());
  ASSERT_TRUE(io.eof());

  io.transfer(io);
  ASSERT_EQ(data.size(), io.size());
  ASSERT_EQ(0u, io.tell());
  ASSERT_FALSE(io.eof());
  std::vector<byte> readBack(data.size(), 0);
  ASSERT_EQ(data.size(), io.read(readBack.data(), readBack.size()));
  ASSERT_EQ(data, readBack);
}
