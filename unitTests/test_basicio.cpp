// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */

#include <exiv2/basicio.hpp>
#include <gtest/gtest.h>

#include <array>

using namespace Exiv2;

TEST(MemIo, isNotAtEofInitially)
{
    std::array<byte, 64> buf;
    buf.fill(0);

    MemIo io(buf.data(), static_cast<long>(buf.size()));
    ASSERT_FALSE(io.eof());
}

TEST(MemIo, seekBeyondBufferSizeReturns1AndSetsEofToTrue)
{
    std::array<byte, 64> buf;
    buf.fill(0);

    MemIo io(buf.data(), static_cast<long>(buf.size()));
    ASSERT_EQ(1, io.seek(65, BasicIo::beg));
    ASSERT_TRUE(io.eof());
}

TEST(MemIo, seekBefore0Returns1ButItDoesNotSetEofToTrue)
{
    std::array<byte, 64> buf;
    buf.fill(0);

    MemIo io(buf.data(), static_cast<long>(buf.size()));
    ASSERT_EQ(1, io.seek(-1, BasicIo::beg));
    ASSERT_FALSE(io.eof());
}

TEST(MemIo, seekBeyondBoundsDoesNotMoveThePosition)
{
    std::array<byte, 64> buf;
    buf.fill(0);

    MemIo io(buf.data(), static_cast<long>(buf.size()));
    ASSERT_EQ(0, io.tell());
    ASSERT_EQ(1, io.seek(65, BasicIo::beg));
    ASSERT_EQ(0, io.tell());
}

TEST(MemIo, seekInsideBoundsMoveThePosition)
{
    std::array<byte, 64> buf;
    buf.fill(0);

    MemIo io(buf.data(), static_cast<long>(buf.size()));
    ASSERT_EQ(0, io.tell());
    ASSERT_EQ(0, io.seek(32, BasicIo::beg));
    ASSERT_EQ(32, io.tell());
}

TEST(MemIo, seekInsideBoundsUsingBeg_resetsThePosition)
{
    std::array<byte, 64> buf;
    buf.fill(0);

    MemIo io(buf.data(), static_cast<long>(buf.size()));
    std::vector<std::int64_t> positions {0, 8, 16, 32, 64};
    for(auto pos: positions) {
      ASSERT_EQ(0, io.seek(pos, BasicIo::beg));
      ASSERT_EQ(pos, io.tell());
    }
}

TEST(MemIo, seekInsideBoundsUsingCur_shiftThePosition)
{
    std::array<byte, 64> buf;
    buf.fill(0);

    MemIo io(buf.data(), static_cast<long>(buf.size()));
    std::vector<std::int64_t> shifts {4, 4, 8, 16, 32};
    std::vector<std::int64_t> positions {4, 8, 16, 32, 64};
    for (size_t i = 0; i < shifts.size(); ++i) {
      ASSERT_EQ(0, io.seek(shifts[i], BasicIo::cur));
      ASSERT_EQ(positions[i], io.tell());
    }
}

TEST(MemIo, seekToEndPosition_doesNotTriggerEof)
{
    std::array<byte, 64> buf;
    buf.fill(0);

    MemIo io(buf.data(), static_cast<long>(buf.size()));
    ASSERT_EQ(0, io.tell());
    ASSERT_EQ(0, io.seek(0, BasicIo::end));
    ASSERT_EQ(64, io.tell());
    ASSERT_FALSE(io.eof());
}

TEST(MemIo, seekToEndPositionAndReadTriggersEof)
{
    std::array<byte, 64> buf;
    buf.fill(0);

    MemIo io(buf.data(), static_cast<long>(buf.size()));
    ASSERT_EQ(0, io.seek(0, BasicIo::end));
    ASSERT_EQ(64, io.tell());

    std::array<byte, 64> buf2;
    buf2.fill(0);
    ASSERT_EQ(0, io.read(buf2.data(), 1)); // Note that we cannot even read 1 byte being at the end
    ASSERT_TRUE(io.eof());
}

TEST(MemIo, readEmptyIoReturns0)
{
    std::array<byte, 10> buf;
    MemIo io;
    ASSERT_EQ(0, io.read(buf.data(), static_cast<long>(buf.size())));
}

TEST(MemIo, readLessBytesThanAvailableReturnsRequestedBytes)
{
    std::array<byte, 10> buf1, buf2;
    buf1.fill(1);
    buf2.fill(0);

    MemIo io(buf1.data(), static_cast<long>(buf1.size()));
    ASSERT_EQ(5, io.read(buf2.data(), 5));
}

TEST(MemIo, readSameBytesThanAvailableReturnsRequestedBytes)
{
    std::array<byte, 10> buf1, buf2;
    buf1.fill(1);
    buf2.fill(0);

    MemIo io(buf1.data(), static_cast<long>(buf1.size()));
    ASSERT_EQ(10, io.read(buf2.data(), 10));
}

TEST(MemIo, readMoreBytesThanAvailableReturnsAvailableBytes)
{
    std::array<byte, 10> buf1, buf2;
    buf1.fill(1);
    buf2.fill(0);

    MemIo io(buf1.data(), static_cast<long>(buf1.size()));
    ASSERT_EQ(10, io.read(buf2.data(), 15));
}
