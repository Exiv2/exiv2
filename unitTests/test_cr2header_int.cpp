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

#include "cr2header_int.hpp"
#include <gtest/gtest.h>

using namespace Exiv2;

static const byte cr2LittleEndian[] = {0x49, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00,
                                       0x43, 0x52, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};

TEST(ACr2Header, hasExpectedValuesAfterCreation)
{
    Internal::Cr2Header header;
    ASSERT_EQ(42, header.tag());
    ASSERT_EQ(16u, header.size());
    ASSERT_EQ(littleEndian, header.byteOrder());
}

TEST(ACr2Header, sizeIs16Bytes)
{
    Internal::Cr2Header header;
    DataBuf buffer = header.write();
    ASSERT_EQ(header.size(), buffer.size_);
    ASSERT_EQ(0, memcmp(cr2LittleEndian, buffer.pData_, 16));
}


TEST(ACr2Header, readDataFromBufferWithCorrectSize)
{
    Internal::Cr2Header header;
    ASSERT_TRUE(header.read(cr2LittleEndian, 16));
}

TEST(ACr2Header, failToReadDataFromBufferWithCorrectSizeButNull)
{
    Internal::Cr2Header header;
    ASSERT_FALSE(header.read(NULL, 16));
}

TEST(ACr2Header, failToReadDataFromBufferWithSizeDifferentThan16)
{
    Internal::Cr2Header header;
    ASSERT_FALSE(header.read(cr2LittleEndian, 15));
    ASSERT_FALSE(header.read(cr2LittleEndian, 0));
}

TEST(ACr2Header, failToReadDataFromBufferWithInvalidByteOrder)
{
    static const byte bufferInvalidByteOrder[] =  {0x88, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00,
                                                   0x43, 0x52, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
    Internal::Cr2Header header;
    ASSERT_FALSE(header.read(bufferInvalidByteOrder, 16));
}

TEST(ACr2Header, failToReadDataFromBufferWithInvalidTag)
{
    static const byte bufferInvalidTag[] = {0x49, 0x49, 0x29, 0x00, 0x10, 0x00, 0x00, 0x00,
                                            0x43, 0x52, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
    Internal::Cr2Header header;
    ASSERT_FALSE(header.read(bufferInvalidTag, 16));
}
