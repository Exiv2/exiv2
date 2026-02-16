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
using namespace Exiv2;

TEST(MemIo_Default, readEReturns0)
{
    std::vector<byte> buf(10);
    MemIo io;
    ASSERT_EQ(0, io.read(buf.data(), (long)buf.size()));
}

TEST(MemIo_Default, isNotAtEof)
{
    MemIo io;
    ASSERT_FALSE(io.eof());
}

TEST(MemIo_Default, seekBeyondBufferSizeReturns1AndSetsEofToTrue)
{
    MemIo io;
    ASSERT_EQ(1, io.seek(1, BasicIo::beg));
    ASSERT_TRUE(io.eof());
}

TEST(MemIo_Default, seekBefore0Returns1ButItDoesNotSetEofToTrue)
{
    MemIo io;
    ASSERT_EQ(1, io.seek(-1, BasicIo::beg));
    ASSERT_FALSE(io.eof());
}

TEST(MemIo_Default, seekToEndPosition_doesNotTriggerEof)
{
    MemIo io;
    ASSERT_EQ(0, io.tell());
    ASSERT_EQ(0, io.seek(0, BasicIo::end));
    ASSERT_EQ(0, io.tell());
    ASSERT_FALSE(io.eof());
}

TEST(MemIo_Default, seekToEndPositionAndReadTriggersEof)
{
    MemIo io;
    ASSERT_EQ(0, io.seek(0, BasicIo::end));
    ASSERT_EQ(0, io.tell());

    std::vector<byte> buf2(64, 0);
    ASSERT_EQ(0, io.read(buf2.data(), 1)); // Note that we cannot even read 1 byte being at the end
    ASSERT_TRUE(io.eof());
}

// -------------------------

TEST(MemIo, seek_out_of_bounds_00)
{
    byte buf[1024];
    memset(buf, 0, sizeof(buf));

    MemIo io(buf, sizeof(buf));
    ASSERT_FALSE(io.eof());

    // Regression test for bug reported in https://github.com/Exiv2/exiv2/pull/945
    // The problem is that MemIo::seek() does not check that the new offset is
    // in bounds.
    byte tmp[16];
    ASSERT_EQ(io.seek(0x10000000, BasicIo::beg), 1);
    ASSERT_TRUE(io.eof());

    // The seek was invalid, so the offset didn't change and this read still works.
    const long sizeTmp = static_cast<long>(sizeof(sizeTmp));
    ASSERT_EQ(io.read(tmp, sizeTmp), sizeTmp);
}

TEST(MemIo, seek_out_of_bounds_01)
{
    byte buf[1024];
    memset(buf, 0, sizeof(buf));

    MemIo io(buf, sizeof(buf));
    ASSERT_FALSE(io.eof());

    byte tmp[16];

    // Seek to the end of the file.
    ASSERT_EQ(io.seek(0, BasicIo::end), 0);
    ASSERT_EQ(io.read(tmp, sizeof(tmp)), 0);

    // Try to seek past the end of the file.
    ASSERT_EQ(io.seek(0x10000000, BasicIo::end), 1);
    ASSERT_TRUE(io.eof());
    ASSERT_EQ(io.read(tmp, sizeof(tmp)), 0);
}

TEST(MemIo, seek_out_of_bounds_02)
{
    byte buf[1024];
    memset(buf, 0, sizeof(buf));

    MemIo io(buf, sizeof(buf));
    ASSERT_FALSE(io.eof());

    byte tmp[16];

    // Try to seek past the end of the file.
    ASSERT_EQ(io.seek(0x10000000, BasicIo::cur), 1);
    ASSERT_TRUE(io.eof());
    // The seek was invalid, so the offset didn't change and this read still works.
    const long sizeTmp = static_cast<long>(sizeof(sizeTmp));
    ASSERT_EQ(io.read(tmp, sizeTmp), sizeTmp);
}

TEST(MemIo, seek_out_of_bounds_03)
{
    byte buf[1024];
    memset(buf, 0, sizeof(buf));

    MemIo io(buf, sizeof(buf));
    ASSERT_FALSE(io.eof());

    byte tmp[16];

    // Try to seek past the beginning of the file.
    ASSERT_EQ(io.seek(-0x10000000, BasicIo::cur), 1);
    ASSERT_FALSE(io.eof());
    // The seek was invalid, so the offset didn't change and this read still works.
    const long sizeTmp = static_cast<long>(sizeof(sizeTmp));
    ASSERT_EQ(io.read(tmp, sizeTmp), sizeTmp);
}
