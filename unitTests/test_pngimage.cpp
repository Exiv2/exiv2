// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/basicio.hpp>
#include <exiv2/pngimage.hpp>
#include "pngchunk_int.hpp"  // This is not part of the public API

#include "mock_basicio.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>
#include <sstream>

using namespace Exiv2;

TEST(PngChunk, keyTxtChunkExtractsKeywordCorrectlyInPresenceOfNullChar) {
  // The following data is: '\0\0"AzTXtRaw profile type exif\0\0x'
  const std::array<std::uint8_t, 32> data{0x00, 0x00, 0x22, 0x41, 0x7a, 0x54, 0x58, 0x74, 0x52, 0x61, 0x77,
                                          0x20, 0x70, 0x72, 0x6f, 0x66, 0x69, 0x6c, 0x65, 0x20, 0x74, 0x79,
                                          0x70, 0x65, 0x20, 0x65, 0x78, 0x69, 0x66, 0x00, 0x00, 0x78};

  DataBuf chunkBuf(data.data(), data.size());
  DataBuf key = Internal::PngChunk::keyTXTChunk(chunkBuf, true);
  ASSERT_EQ(21u, key.size());

  ASSERT_TRUE(std::equal(key.data(), key.data() + key.size(), data.data() + 8));
}

TEST(PngChunk, keyTxtChunkThrowsExceptionWhenThereIsNoNullChar) {
  // The following data is: '\0\0"AzTXtRaw profile type exifx'
  const std::array<std::uint8_t, 30> data{0x00, 0x00, 0x22, 0x41, 0x7a, 0x54, 0x58, 0x74, 0x52, 0x61,
                                          0x77, 0x20, 0x70, 0x72, 0x6f, 0x66, 0x69, 0x6c, 0x65, 0x20,
                                          0x74, 0x79, 0x70, 0x65, 0x20, 0x65, 0x78, 0x69, 0x66, 0x78};

  DataBuf chunkBuf(data.data(), data.size());
  ASSERT_THROW(Internal::PngChunk::keyTXTChunk(chunkBuf, true), Exiv2::Error);
}

TEST(PngChunk, keyTxtChunkThrowsIfSizeIsNotEnough) {
  const std::array<std::uint8_t, 4> data{0x00, 0x00, 0x22, 0x41};
  DataBuf chunkBuf(data.data(), data.size());
  ASSERT_THROW(Internal::PngChunk::keyTXTChunk(chunkBuf, true), Exiv2::Error);

  DataBuf emptyChunk(data.data(), 0);
  ASSERT_THROW(Internal::PngChunk::keyTXTChunk(emptyChunk, false), Exiv2::Error);
}

TEST(PngImage, canBeCreatedFromScratch) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  ASSERT_NO_THROW(PngImage png(std::move(memIo), create));
}

TEST(PngImage, canBeOpenedEvenWithAnEmptyMemIo) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{false};
  ASSERT_NO_THROW(PngImage png(std::move(memIo), create));
}

TEST(PngImage, mimeTypeIsPng) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  PngImage png(std::move(memIo), create);

  ASSERT_EQ("image/png", png.mimeType());
}

TEST(PngImage, printStructurePrintsNothingWithKpsNone) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  PngImage png(std::move(memIo), create);

  std::ostringstream stream;
  png.printStructure(stream, Exiv2::kpsNone, 1);

  ASSERT_TRUE(stream.str().empty());
}

TEST(PngImage, printStructurePrintsDataWithKpsBasic) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  PngImage png(std::move(memIo), create);

  std::ostringstream stream;
  png.printStructure(stream, Exiv2::kpsBasic, 1);

  ASSERT_FALSE(stream.str().empty());
}

TEST(PngImage, cannotReadMetadataFromEmptyIo) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{false};
  PngImage png(std::move(memIo), create);

  try {
    png.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerNotAnImage, e.code());
    ASSERT_STREQ("This does not look like a PNG image", e.what());
  }
}

TEST(PngImage, cannotReadMetadataFromIoWhichCannotBeOpened) {
  auto mockIo = makeMockIo();
  setupOpenFailure(*mockIo);
  PngImage png(std::move(mockIo), false);

  try {
    png.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerDataSourceOpenFailed, e.code());
  }
}

TEST(PngImage, cannotWriteMetadataToEmptyIo) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{false};
  PngImage png(std::move(memIo), create);

  try {
    png.writeMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerNoImageInInputData, e.code());
  }
}

TEST(PngImage, canWriteMetadataFromCreatedPngImage) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  PngImage png(std::move(memIo), create);
  ASSERT_NO_THROW(png.writeMetadata());
}

TEST(PngImage, cannotWriteMetadataToIoWhichCannotBeOpened) {
  auto mockIo = makeMockIo();
  setupOpenFailure(*mockIo);
  PngImage png(std::move(mockIo), false);

  try {
    png.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerDataSourceOpenFailed, e.code());
  }
}

TEST(isPngType, withValidSignatureReturnsTrue) {
  auto mockIo = makeMockIo();
  setupRead(*mockIo, {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A});
  ASSERT_TRUE(isPngType(*mockIo, false));
}

TEST(isPngType, withInvalidSignatureReturnsFalse) {
  auto mockIo = makeMockIo();
  setupRead(*mockIo, {0x69, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A});
  ASSERT_FALSE(isPngType(*mockIo, false));
}

TEST(isPngType, withShorterDataReturnsFalse) {
  auto mockIo = makeMockIo();
  setupRead(*mockIo, {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A});
  ASSERT_FALSE(isPngType(*mockIo, false));
}

TEST(isPngType, withReadFailureReturnsFalse) {
  auto mockIo = makeMockIo();
  setupReadFailure(*mockIo);
  ASSERT_FALSE(isPngType(*mockIo, false));
}
