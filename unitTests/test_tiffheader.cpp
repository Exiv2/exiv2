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

#include <tiffimage_int.hpp>
#include <gtest/gtest.h>
#include <sstream>

using namespace Exiv2;

static const byte tiffLittleEndian[] = {0x49, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00};

class ATiffHeader: public ::testing::Test
{
public:
    Internal::TiffHeader header;
};

TEST_F(ATiffHeader, hasExpectedValuesAfterCreation)
{
    ASSERT_EQ(8U, header.size());
    ASSERT_EQ(42, header.tag());
    ASSERT_EQ(8U, header.offset());
    ASSERT_EQ(littleEndian, header.byteOrder());
}

TEST_F(ATiffHeader, canBeWrittenAndItsSizeIs8Bytes)
{
    DataBuf buffer = header.write();
    ASSERT_EQ(header.size(), buffer.size_);
    ASSERT_EQ(8U, header.size());
}

TEST_F(ATiffHeader, readDataFromBufferWithCorrectSize)
{
    ASSERT_TRUE(header.read(tiffLittleEndian, 8));
}

TEST_F(ATiffHeader, failToReadDataFromBufferWithCorrectSizeButNull)
{
    ASSERT_FALSE(header.read(NULL, 8));
}

TEST_F(ATiffHeader, failToReadDataFromBufferWithSizeDifferentThan8)
{
    ASSERT_FALSE(header.read(NULL, 7));
    ASSERT_FALSE(header.read(NULL, 9));
}

TEST_F(ATiffHeader, failToReadDataFromBufferWithInvalidByteOrder)
{
    static const byte bufferInvalidByteOrder[] =  {0x88, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00};
    ASSERT_FALSE(header.read(bufferInvalidByteOrder, 8));
}

TEST_F(ATiffHeader, failToReadDataFromBufferWithInvalidTag)
{
    static const byte bufferInvalidTag[] = {0x49, 0x49, 0x29, 0x00, 0x10, 0x00, 0x00, 0x00};
    ASSERT_FALSE(header.read(bufferInvalidTag, 8));
}

TEST_F(ATiffHeader, printsDefaultValues)
{
    std::ostringstream str;
    header.print(str, "");
    ASSERT_STREQ("TIFF header, offset = 0x00000008, little endian encoded\n", str.str().c_str());
}
