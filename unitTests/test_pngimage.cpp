// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/basicio.hpp>
#include <exiv2/pngimage.hpp>
#include "pngchunk_int.hpp"  // This is not part of the public API
#include "unittest_utils.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
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

TEST(PngChunk, makeMetadataChunkXmpUsesOnlyTheViewedBytes) {
  // The view covers only the packet; the backing storage continues with more bytes before its NUL.
  const std::string backing = "<x:xmpmeta/>TRAILING-BYTES-NOT-IN-VIEW";
  const std::string_view packet(backing.data(), 12);
  const std::string expected = Internal::PngChunk::makeMetadataChunk(std::string(packet), mdXmp);
  const std::string chunk = Internal::PngChunk::makeMetadataChunk(packet, mdXmp);
  // 4 (length) + "iTXt" + "XML:com.adobe.xmp" + 5 flag bytes + packet + 4 (CRC)
  ASSERT_EQ(4u + 4u + 17u + 5u + packet.size() + 4u, chunk.size());
  ASSERT_EQ(expected, chunk);
  ASSERT_EQ(std::string::npos, chunk.find("TRAILING"));
}

TEST(PngChunk, makeMetadataChunkXmpAcceptsViewWithoutNulTerminator) {
  std::array<char, 80> packet;
  packet.fill('x');  // no NUL anywhere in the storage
  // The old strlen()-based append over-read here; AddressSanitizer reports it. Without a sanitizer the result
  // depends on adjacent memory, so makeMetadataChunkXmpUsesOnlyTheViewedBytes is the deterministic check.
  const std::string_view view(packet.data(), packet.size());
  const std::string chunk = Internal::PngChunk::makeMetadataChunk(view, mdXmp);
  ASSERT_EQ(4u + 4u + 17u + 5u + packet.size() + 4u, chunk.size());
  ASSERT_EQ(std::string(view), chunk.substr(4 + 4 + 17 + 5, packet.size()));
}

TEST(PngImage, canBeCreatedFromScratch) {
  auto memIo = std::make_unique<MemIo>();
  ASSERT_NO_THROW(PngImage png(std::move(memIo), defaultImageCtorParams(true)));
}

TEST(PngImage, canBeOpenedEvenWithAnEmptyMemIo) {
  auto memIo = std::make_unique<MemIo>();
  ASSERT_NO_THROW(PngImage png(std::move(memIo), defaultImageCtorParams(false)));
}

TEST(PngImage, mimeTypeIsPng) {
  auto memIo = std::make_unique<MemIo>();
  PngImage png(std::move(memIo), defaultImageCtorParams(true));

  ASSERT_EQ("image/png", png.mimeType());
}

TEST(PngImage, printStructurePrintsNothingWithKpsNone) {
  auto memIo = std::make_unique<MemIo>();
  PngImage png(std::move(memIo), defaultImageCtorParams(true));

  std::ostringstream stream;
  png.printStructure(stream, Exiv2::kpsNone, 1);

  ASSERT_TRUE(stream.str().empty());
}

TEST(PngImage, printStructurePrintsDataWithKpsBasic) {
  auto memIo = std::make_unique<MemIo>();
  PngImage png(std::move(memIo), defaultImageCtorParams(true));

  std::ostringstream stream;
  png.printStructure(stream, Exiv2::kpsBasic, 1);

  ASSERT_FALSE(stream.str().empty());
}

TEST(PngImage, cannotReadMetadataFromEmptyIo) {
  auto memIo = std::make_unique<MemIo>();
  PngImage png(std::move(memIo), defaultImageCtorParams(false));

  try {
    png.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerNotAnImage, e.code());
    ASSERT_STREQ("This does not look like a PNG image", e.what());
  }
}

TEST(PngImage, cannotReadMetadataFromIoWhichCannotBeOpened) {
  auto memIo = std::make_unique<FileIo>("NonExistingPath.png");
  PngImage png(std::move(memIo), defaultImageCtorParams(false));

  try {
    png.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerDataSourceOpenFailed, e.code());
  }
}

TEST(PngImage, cannotWriteMetadataToEmptyIo) {
  auto memIo = std::make_unique<MemIo>();
  PngImage png(std::move(memIo), defaultImageCtorParams(false));

  try {
    png.writeMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerNoImageInInputData, e.code());
  }
}

TEST(PngImage, canWriteMetadataFromCreatedPngImage) {
  auto memIo = std::make_unique<MemIo>();
  PngImage png(std::move(memIo), defaultImageCtorParams(true));
  ASSERT_NO_THROW(png.writeMetadata());
}

TEST(PngImage, cannotWriteMetadataToIoWhichCannotBeOpened) {
  auto memIo = std::make_unique<FileIo>("NonExistingPath.png");
  PngImage png(std::move(memIo), defaultImageCtorParams(false));

  try {
    png.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerDataSourceOpenFailed, e.code());
  }
}

TEST(isPngType, withValidSignatureReturnsTrue) {
  const unsigned char pngSignature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
  MemIo memIo(pngSignature, 8);
  ASSERT_TRUE(isPngType(memIo, false));
}

TEST(isPngType, withInvalidSignatureReturnsFalse) {
  const unsigned char pngSignature[8] = {0x69, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
  MemIo memIo(pngSignature, 8);
  ASSERT_FALSE(isPngType(memIo, false));
}

TEST(isPngType, withShorterDataReturnsFalse) {
  const unsigned char pngSignature[6] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A};
  MemIo memIo(pngSignature, 6);
  ASSERT_FALSE(isPngType(memIo, false));
}

TEST(isPngType, withEmptyDataReturnsFalse) {
  MemIo memIo;
  ASSERT_FALSE(isPngType(memIo, false));
}

TEST(isPngType, withMemIoInErroneousStatusThrows) {
  MemIo memIo;
  memIo.getb();

  try {
    isPngType(memIo, false);
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInputDataReadFailed, e.code());
  }
}
