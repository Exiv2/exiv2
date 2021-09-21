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

#include <exiv2/types.hpp>
#include <cmath>
#include <limits>
#include <gtest/gtest.h>
using namespace Exiv2;

// More info about tm : http://www.cplusplus.com/reference/ctime/tm/

TEST(ExivTime, getsTimeFromValidString)
{
    struct tm tmInstance;
    ASSERT_EQ(0, exifTime("2007:05:24 12:31:55", &tmInstance));
    ASSERT_EQ(107,  tmInstance.tm_year); // Years since 1900
    ASSERT_EQ(4,    tmInstance.tm_mon);
    ASSERT_EQ(24,   tmInstance.tm_mday);
    ASSERT_EQ(12,   tmInstance.tm_hour);
    ASSERT_EQ(31,   tmInstance.tm_min);
    ASSERT_EQ(55,   tmInstance.tm_sec);
}

TEST(ExivTime, doesNotGetTimeWithBadFormedString)
{
    struct tm tmInstance;
    ASSERT_EQ(1, exifTime("007:a5:24 aa:bb:cc", &tmInstance));
}

TEST(DataBuf, pointsToNullByDefault)
{
    DataBuf instance;
    ASSERT_EQ(nullptr, instance.c_data());
    ASSERT_EQ(0,    instance.size());
}

TEST(DataBuf, allocatesDataWithNonEmptyConstructor)
{
    DataBuf instance (5);
    ASSERT_NE(static_cast<byte *>(nullptr), instance.c_data());  /// \todo use nullptr once we move to c++11
    ASSERT_EQ(5,    instance.size());
}

// Test methods like DataBuf::read_uint32 and DataBuf::write_uint32.
TEST(DataBuf, read_write_endianess)
{
    DataBuf buf(4 + 1 + 2 + 4 + 8);
    buf.clear();

    // Big endian.
    buf.write_uint8(4, 0x01);
    buf.write_uint16(4 + 1, 0x0203, bigEndian);
    buf.write_uint32(4 + 1 + 2, 0x04050607, bigEndian);
    buf.write_uint64(4 + 1 + 2 + 4, 0x08090a0b0c0d0e0fULL, bigEndian);
    static const uint8_t expected_le[4 + 1 + 2 + 4 + 8] =
        {0, 0, 0, 0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf};
    ASSERT_EQ(0, buf.cmpBytes(0, expected_le, buf.size()));
    ASSERT_EQ(buf.read_uint8(4), 0x01);
    ASSERT_EQ(buf.read_uint16(4 + 1, bigEndian), 0x0203);
    ASSERT_EQ(buf.read_uint32(4 + 1 + 2, bigEndian), 0x04050607);
    ASSERT_EQ(buf.read_uint64(4 + 1 + 2 + 4, bigEndian), 0x08090a0b0c0d0e0fULL);

    // Little endian.
    buf.write_uint8(4, 0x01);
    buf.write_uint16(4 + 1, 0x0203, littleEndian);
    buf.write_uint32(4 + 1 + 2, 0x04050607, littleEndian);
    buf.write_uint64(4 + 1 + 2 + 4, 0x08090a0b0c0d0e0fULL, littleEndian);
    static const uint8_t expected_be[4 + 1 + 2 + 4 + 8] =
        {0, 0, 0, 0, 0x1, 0x3, 0x2, 0x7, 0x6, 0x5, 0x4, 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8};
    ASSERT_EQ(0, buf.cmpBytes(0, expected_be, buf.size()));
    ASSERT_EQ(buf.read_uint8(4), 0x01);
    ASSERT_EQ(buf.read_uint16(4 + 1, littleEndian), 0x0203);
    ASSERT_EQ(buf.read_uint32(4 + 1 + 2, littleEndian), 0x04050607);
    ASSERT_EQ(buf.read_uint64(4 + 1 + 2 + 4, littleEndian), 0x08090a0b0c0d0e0fULL);
}

TEST(Rational, floatToRationalCast)
{
    static const float floats[] = {0.5F, 0.015F, 0.0000625F};

    for (size_t i = 0; i < sizeof(floats) / sizeof(*floats); ++i) {
        const Rational r = floatToRationalCast(floats[i]);
        const float fraction = static_cast<float>(r.first) / static_cast<float>(r.second);
        ASSERT_TRUE(std::fabs((floats[i] - fraction) / floats[i]) < 0.01F);
    }

    const Rational plus_inf = floatToRationalCast(std::numeric_limits<float>::infinity());
    ASSERT_EQ(plus_inf.first, 1);
    ASSERT_EQ(plus_inf.second, 0);

    const Rational minus_inf = floatToRationalCast(-1 * std::numeric_limits<float>::infinity());
    ASSERT_EQ(minus_inf.first, -1);
    ASSERT_EQ(minus_inf.second, 0);
}
