// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <array>
#include <exiv2/bmpimage.hpp>

using namespace Exiv2;

TEST(BmpImage, canBeOpenedWithEmptyMemIo) {
  auto memIo = std::make_unique<MemIo>();
  ASSERT_NO_THROW(BmpImage bmp(std::move(memIo)));
}

TEST(BmpImage, mimeTypeIsBmp) {
  auto memIo = std::make_unique<MemIo>();
  BmpImage bmp(std::move(memIo));

  ASSERT_EQ("image/x-ms-bmp", bmp.mimeType());
}

TEST(BmpImage, writeMetadataIsNotImplemented) {
  auto memIo = std::make_unique<MemIo>();
  BmpImage bmp(std::move(memIo));

  try {
    bmp.writeMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerWritingImageFormatUnsupported, e.code());
    ASSERT_STREQ("Writing to BMP images is not supported", e.what());
  }
}

TEST(BmpImage, setExitDataIsNotImplemented) {
  auto memIo = std::make_unique<MemIo>();
  BmpImage bmp(std::move(memIo));

  try {
    ExifData data;
    bmp.setExifData(data);
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInvalidSettingForImage, e.code());
    ASSERT_STREQ("Setting Exif metadata in BMP images is not supported", e.what());
  }
}

TEST(BmpImage, setIptcDataIsNotImplemented) {
  auto memIo = std::make_unique<MemIo>();
  BmpImage bmp(std::move(memIo));

  try {
    IptcData data;
    bmp.setIptcData(data);
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInvalidSettingForImage, e.code());
    ASSERT_STREQ("Setting IPTC metadata in BMP images is not supported", e.what());
  }
}

TEST(BmpImage, setCommentIsNotImplemented) {
  auto memIo = std::make_unique<MemIo>();
  BmpImage bmp(std::move(memIo));

  try {
    bmp.setComment("random comment");
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInvalidSettingForImage, e.code());
    ASSERT_STREQ("Setting Image comment in BMP images is not supported", e.what());
  }
}

TEST(BmpImage, readMetadataReadsImageDimensionsWhenDataIsAvailable) {
  const std::array<unsigned char, 26> header{
      'B',  'M',               // Signature                                                         off:0   size:2
      0x4E, 0x47, 0x0D, 0x0A,  // Size of the BMP file in bytes                                     off:2,  size:4
      0x1A, 0x0A,              // Reserved                                                          off:6,  size:2
      0x00, 0x00,              // Reserved                                                          off:8,  size:2
      0x00, 0x00, 0x00, 0x00,  // Offset of the byte where the bitmap image data can be found       off:10, size:4
      0x00, 0x00, 0x00, 0x00,  // Size of this header                                               off:14, size:4
      0x00, 0x05, 0x00, 0x00,  // The bitmap width in pixels (unsigned 16 bit)                      off:18, size:4
      0x20, 0x03, 0x00, 0x00,  // The bitmap height in pixels (unsigned 16 bit)                     off:22, size:4
  };

  auto memIo = std::make_unique<MemIo>(header.data(), header.size());
  BmpImage bmp(std::move(memIo));
  ASSERT_NO_THROW(bmp.readMetadata());
  ASSERT_EQ(1280, bmp.pixelWidth());
  ASSERT_EQ(800, bmp.pixelHeight());
}

TEST(BmpImage, readMetadataThrowsWhenImageIsNotBMP) {
  const std::array<unsigned char, 26> header{
      'B',  'A',               // Signature                                                         off:0   size:2
      0x4E, 0x47, 0x0D, 0x0A,  // Size of the BMP file in bytes                                     off:2,  size:4
      0x1A, 0x0A,              // Reserved                                                          off:6,  size:2
      0x00, 0x00,              // Reserved                                                          off:8,  size:2
      0x00, 0x00, 0x00, 0x00,  // Offset of the byte where the bitmap image data can be found       off:10, size:4
      0x00, 0x00, 0x00, 0x00,  // Size of this header                                               off:14, size:4
      0x00, 0x05, 0x00, 0x00,  // The bitmap width in pixels (unsigned 16 bit)                      off:18, size:4
      0x20, 0x03, 0x00, 0x00,  // The bitmap height in pixels (unsigned 16 bit)                     off:22, size:4
  };

  auto memIo = std::make_unique<MemIo>(header.data(), header.size());
  BmpImage bmp(std::move(memIo));
  try {
    bmp.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerNotAnImage, e.code());
    ASSERT_STREQ("This does not look like a BMP image", e.what());
  }
}

TEST(BmpImage, readMetadataThrowsWhenThereIsNotEnoughInfoToRead) {
  const std::array<unsigned char, 1> header{'B'};
  auto memIo = std::make_unique<MemIo>(header.data(), header.size());
  BmpImage bmp(std::move(memIo));
  try {
    bmp.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerFailedToReadImageData, e.code());
    ASSERT_STREQ("Failed to read image data", e.what());
  }
}

TEST(BmpImage, readMetadataThrowsWhenIoCannotBeOpened) {
  auto fileIo = std::make_unique<FileIo>("NonExistingPath.png");
  BmpImage bmp(std::move(fileIo));
  try {
    bmp.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerDataSourceOpenFailed, e.code());
  }
}

TEST(newBmpInstance, createsValidInstace) {
  const std::array<unsigned char, 14> bitmapHeader{
      'B',  'M',               // Signature
      0x4E, 0x47, 0x0D, 0x0A,  // Size of the BMP file in bytes
      0x1A, 0x0A,              // Reserved
      0x00, 0x00,              // Reserved
      0x00, 0x00, 0x00, 0x00   // Offset of the byte where the bitmap image data can be found
  };
  auto memIo = std::make_unique<MemIo>(bitmapHeader.data(), bitmapHeader.size());
  auto img = newBmpInstance(std::move(memIo), false);
  ASSERT_TRUE(img->good());
}

TEST(newBmpInstance, createsInvalidInstaceWithNonExistingFilePath) {
  auto fileIo = std::make_unique<FileIo>("NonExistingPath.png");
  auto img = newBmpInstance(std::move(fileIo), false);
  ASSERT_FALSE(img);
}

TEST(isBmpType, withValidSignatureReturnsTrue) {
  const std::array<unsigned char, 14> bitmapHeader{
      'B',  'M',               // Signature
      0x4E, 0x47, 0x0D, 0x0A,  // Size of the BMP file in bytes
      0x1A, 0x0A,              // Reserved
      0x00, 0x00,              // Reserved
      0x00, 0x00, 0x00, 0x00   // Offset of the byte where the bitmap image data can be found
  };
  MemIo memIo(bitmapHeader.data(), bitmapHeader.size());
  ASSERT_TRUE(isBmpType(memIo, false));
}

TEST(isBmpType, withInvalidSignatureReturnsFalse) {
  const std::array<unsigned char, 14> bitmapHeader{
      'B',  'A',               // Signature
      0x4E, 0x47, 0x0D, 0x0A,  // Size of the BMP file in bytes
      0x1A, 0x0A,              // Reserved
      0x00, 0x00,              // Reserved
      0x00, 0x00, 0x00, 0x00   // Offset of the byte where the bitmap image data can be found
  };
  MemIo memIo(bitmapHeader.data(), bitmapHeader.size());
  ASSERT_FALSE(isBmpType(memIo, false));
}
