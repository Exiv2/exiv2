/*
 * Copyright (C) 2004-2022 Exiv2 authors
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

#include <exiv2/pngimage.hpp>
#include "pngchunk_int.hpp" // This is not part of the public API

#include <gtest/gtest.h>

#include <array>
#include <algorithm>

using namespace Exiv2;

TEST(PngChunk, keyTxtChunkExtractsKeywordCorrectlyInPresenceOfNullChar)
{
  // The following data is: '\0\0"AzTXtRaw profile type exif\0\0x'
  std::array<std::uint8_t, 32> data{0x00, 0x00, 0x22, 0x41, 0x7a, 0x54, 0x58, 0x74,
                                    0x52, 0x61, 0x77, 0x20, 0x70, 0x72, 0x6f, 0x66,
                                    0x69, 0x6c, 0x65, 0x20, 0x74, 0x79, 0x70, 0x65,
                                    0x20, 0x65, 0x78, 0x69, 0x66, 0x00, 0x00, 0x78};

  DataBuf chunkBuf(data.data(), static_cast<long>(data.size()));
  DataBuf key = Internal::PngChunk::keyTXTChunk(chunkBuf, true);
  ASSERT_EQ(21, key.size());

  ASSERT_TRUE(std::equal(key.data(), key.data()+key.size(), data.data()+8, data.data()+8+key.size()));
}


TEST(PngChunk, keyTxtChunkThrowsExceptionWhenThereIsNoNullChar)
{
  // The following data is: '\0\0"AzTXtRaw profile type exifx'
  std::array<std::uint8_t, 30> data{0x00, 0x00, 0x22, 0x41, 0x7a, 0x54, 0x58, 0x74,
                                    0x52, 0x61, 0x77, 0x20, 0x70, 0x72, 0x6f, 0x66,
                                    0x69, 0x6c, 0x65, 0x20, 0x74, 0x79, 0x70, 0x65,
                                    0x20, 0x65, 0x78, 0x69, 0x66, 0x78};

  DataBuf chunkBuf(data.data(), static_cast<long>(data.size()));
  ASSERT_THROW(Internal::PngChunk::keyTXTChunk(chunkBuf, true), Exiv2::Error);
}
