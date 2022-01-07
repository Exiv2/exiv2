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
#include <memory>
#include <sstream>

using namespace Exiv2;

TEST(PngChunk, keyTxtChunkExtractsKeywordCorrectlyInPresenceOfNullChar)
{
  // The following data is: '\0\0"AzTXtRaw profile type exif\0\0x'
  const std::array<std::uint8_t, 32> data{0x00, 0x00, 0x22, 0x41, 0x7a, 0x54, 0x58, 0x74,
                                          0x52, 0x61, 0x77, 0x20, 0x70, 0x72, 0x6f, 0x66,
                                          0x69, 0x6c, 0x65, 0x20, 0x74, 0x79, 0x70, 0x65,
                                          0x20, 0x65, 0x78, 0x69, 0x66, 0x00, 0x00, 0x78};

  DataBuf chunkBuf(data.data(), static_cast<long>(data.size()));
  DataBuf key = Internal::PngChunk::keyTXTChunk(chunkBuf, true);
  ASSERT_EQ(21, key.size());

  ASSERT_TRUE(std::equal(key.data(), key.data()+key.size(), data.data()+8));
}


TEST(PngChunk, keyTxtChunkThrowsExceptionWhenThereIsNoNullChar)
{
  // The following data is: '\0\0"AzTXtRaw profile type exifx'
  const std::array<std::uint8_t, 30> data{0x00, 0x00, 0x22, 0x41, 0x7a, 0x54, 0x58, 0x74,
                                          0x52, 0x61, 0x77, 0x20, 0x70, 0x72, 0x6f, 0x66,
                                          0x69, 0x6c, 0x65, 0x20, 0x74, 0x79, 0x70, 0x65,
                                          0x20, 0x65, 0x78, 0x69, 0x66, 0x78};

  DataBuf chunkBuf(data.data(), static_cast<long>(data.size()));
  ASSERT_THROW(Internal::PngChunk::keyTXTChunk(chunkBuf, true), Exiv2::Error);
}

TEST(PngChunk, keyTxtChunkThrowsIfSizeIsNotEnough)
{
  const std::array<std::uint8_t, 4> data{0x00, 0x00, 0x22, 0x41};
  DataBuf chunkBuf(data.data(), static_cast<long>(data.size()));
  ASSERT_THROW(Internal::PngChunk::keyTXTChunk(chunkBuf, true), Exiv2::Error);

  DataBuf emptyChunk(data.data(), 0);
  ASSERT_THROW(Internal::PngChunk::keyTXTChunk(emptyChunk, false), Exiv2::Error);
}


TEST(PngImage, canBeCreatedFromScratch)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {true};
    ASSERT_NO_THROW(PngImage png(std::move(memIo), create));
}

TEST(PngImage, canBeOpenedEvenWithAnEmptyMemIo)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {false};
    ASSERT_NO_THROW(PngImage png(std::move(memIo), create));
}

TEST(PngImage, mimeTypeIsPng)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {true};
    PngImage png(std::move(memIo), create);

    ASSERT_EQ("image/png", png.mimeType());
}

TEST(PngImage, printStructurePrintsNothingWithKpsNone)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {true};
    PngImage png(std::move(memIo), create);

    std::ostringstream stream;
    png.printStructure(stream, Exiv2::kpsNone, 1);

    ASSERT_TRUE(stream.str().empty());
}

TEST(PngImage, printStructurePrintsDataWithKpsBasic)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {true};
    PngImage png(std::move(memIo), create);

    std::ostringstream stream;
    png.printStructure(stream, Exiv2::kpsBasic, 1);

    ASSERT_FALSE(stream.str().empty());
}

TEST(PngImage, cannotReadMetadataFromEmptyIo)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {false};
    PngImage png(std::move(memIo), create);

    try {
      png.readMetadata();
      FAIL();
    }  catch (const Exiv2::Error& e) {
      ASSERT_EQ(kerNotAnImage, e.code());
      ASSERT_STREQ("This does not look like a PNG image", e.what());
    }
}

TEST(PngImage, cannotReadMetadataFromIoWhichCannotBeOpened)
{
    auto memIo = std::make_unique<FileIo>("NonExistingPath.png");
    const bool create {false};
    PngImage png(std::move(memIo), create);

    try {
      png.readMetadata();
      FAIL();
    }  catch (const Exiv2::Error& e) {
      ASSERT_EQ(kerDataSourceOpenFailed, e.code());
    }
}

TEST(PngImage, cannotWriteMetadataToEmptyIo)
{
    auto memIo = std::make_unique<MemIo>();
    const bool create {false};
    PngImage png(std::move(memIo), create);

    try {
      png.writeMetadata();
      FAIL();
    }  catch (const Exiv2::Error& e) {
      ASSERT_EQ(kerNoImageInInputData, e.code());
    }
}

TEST(PngImage, cannotWriteMetadataToIoWhichCannotBeOpened)
{
    auto memIo = std::make_unique<FileIo>("NonExistingPath.png");
    const bool create {false};
    PngImage png(std::move(memIo), create);

    try {
      png.readMetadata();
      FAIL();
    }  catch (const Exiv2::Error& e) {
      ASSERT_EQ(kerDataSourceOpenFailed, e.code());
    }
}


TEST(isPngType, withValidSignatureReturnsTrue)
{
    const unsigned char pngSignature[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    MemIo memIo(pngSignature, 8);
    ASSERT_TRUE(isPngType(memIo, false));
}

TEST(isPngType, withInvalidSignatureReturnsFalse)
{
    const unsigned char pngSignature[8] = { 0x69, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    MemIo memIo(pngSignature, 8);
    ASSERT_FALSE(isPngType(memIo, false));
}

TEST(isPngType, withShorterDataReturnsFalse)
{
    const unsigned char pngSignature[6] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A};
    MemIo memIo(pngSignature, 6);
    ASSERT_FALSE(isPngType(memIo, false));
}

TEST(isPngType, withEmptyDataReturnsFalse)
{
    MemIo memIo;
    ASSERT_FALSE(isPngType(memIo, false));
}

TEST(isPngType, withMemIoInErroneousStatusThrows)
{
    MemIo memIo;
    memIo.getb();

    try {
      isPngType(memIo, false);
      FAIL();
    }  catch (const Exiv2::Error& e) {
      ASSERT_EQ(kerInputDataReadFailed, e.code());
    }
}
