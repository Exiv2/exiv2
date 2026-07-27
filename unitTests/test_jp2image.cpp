// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/basicio.hpp>
#include <exiv2/jp2image.hpp>

#include "mock_basicio.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>

using namespace Exiv2;

namespace {

constexpr std::array<byte, 12> kJp2Signature = {
    0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a,
};

}  // namespace

TEST(Jp2Image, canBeCreatedFromScratch) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  ASSERT_NO_THROW(Jp2Image image(std::move(memIo), create));
}

TEST(Jp2Image, canBeOpenedEvenWithAnEmptyMemIo) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{false};
  ASSERT_NO_THROW(Jp2Image image(std::move(memIo), create));
}

TEST(Jp2Image, mimeTypeIsPng) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  Jp2Image image(std::move(memIo), create);

  ASSERT_EQ("image/jp2", image.mimeType());
}

TEST(Jp2Image, printStructurePrintsNothingWithKpsNone) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  Jp2Image image(std::move(memIo), create);

  std::ostringstream stream;
  image.printStructure(stream, Exiv2::kpsNone, 1);

  ASSERT_TRUE(stream.str().empty());
}

TEST(Jp2Image, printStructurePrintsDataWithKpsBasic) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  Jp2Image image(std::move(memIo), create);

  std::ostringstream stream;
  image.printStructure(stream, Exiv2::kpsBasic, 1);

  ASSERT_FALSE(stream.str().empty());
}

TEST(Jp2Image, cannotReadMetadataFromEmptyIo) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{false};
  Jp2Image image(std::move(memIo), create);

  try {
    image.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerNotAnImage, e.code());
    ASSERT_STREQ("This does not look like a JPEG-2000 image", e.what());
  }
}

TEST(Jp2Image, cannotReadMetadataFromIoWhichCannotBeOpened) {
  auto mockIo = makeMockIo();
  setupOpenFailure(*mockIo);
  Jp2Image image(std::move(mockIo), false);

  try {
    image.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerDataSourceOpenFailed, e.code());
  }
}

TEST(Jp2Image, cannotWriteMetadataToEmptyIo) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{false};
  Jp2Image image(std::move(memIo), create);

  try {
    image.writeMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerNoImageInInputData, e.code());
  }
}

TEST(isJp2Type, withValidSignatureReturnsTrue) {
  auto mockIo = makeMockIo();
  setupRead(*mockIo, kJp2Signature);
  ASSERT_TRUE(isJp2Type(*mockIo, false));
}

TEST(isJp2Type, withReadFailureReturnsFalse) {
  auto mockIo = makeMockIo();
  setupReadFailure(*mockIo);
  ASSERT_FALSE(isJp2Type(*mockIo, false));
}

TEST(Jp2Image, canWriteMetadataFromCreatedJp2Image) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  Jp2Image image(std::move(memIo), create);
  ASSERT_NO_THROW(image.writeMetadata());
}

TEST(Jp2Image, cannotWriteMetadataToIoWhichCannotBeOpened) {
  auto mockIo = makeMockIo();
  setupOpenFailure(*mockIo);
  Jp2Image image(std::move(mockIo), false);

  try {
    image.readMetadata();
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerDataSourceOpenFailed, e.code());
  }
}

TEST(Jp2Image, canWriteMetadataAndReadAfterwards) {
  auto memIo = std::make_unique<MemIo>();
  const bool create{true};
  Jp2Image image(std::move(memIo), create);
  ASSERT_NO_THROW(image.writeMetadata());
  ASSERT_NO_THROW(image.readMetadata());
}
