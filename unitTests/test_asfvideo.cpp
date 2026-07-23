// SPDX-License-Identifier: GPL-2.0-or-later

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exiv2/asfvideo.hpp>
#include <exiv2/basicio.hpp>

#include "mock_basicio.hpp"

#include <array>

using namespace Exiv2;

namespace {

// ASF Header GUID in little-endian byte order:
// data1(0x75B22630)=30 26 B2 75, data2(0x668E)=8E 66, data3(0x11CF)=CF 11,
// data4={A6 D9 00 AA 00 62 CE 6C}
constexpr std::array<byte, 16> kAsfHeaderSignature = {
    0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C,
};

}  // namespace

TEST(AsfVideo, canBeOpenedWithEmptyMemIo) {
  auto memIo = std::make_unique<MemIo>();
  ASSERT_NO_THROW(AsfVideo asf(std::move(memIo)));
}

TEST(AsfVideo, mimeTypeIsAsf) {
  auto memIo = std::make_unique<MemIo>();
  AsfVideo asf(std::move(memIo));

  ASSERT_EQ("video/asf", asf.mimeType());
}

TEST(AsfVideo, isAsfTypewithEmptyDataReturnsFalse) {
  auto mockIo = makeMockIo();
  setupReadFailure(*mockIo);
  ASSERT_FALSE(isAsfType(*mockIo, false));
}

TEST(AsfVideo, isAsfTypeWithValidSignatureReturnsTrue) {
  auto mockIo = makeMockIo();
  setupRead(*mockIo, kAsfHeaderSignature);
  ASSERT_TRUE(isAsfType(*mockIo, false));
}

TEST(AsfVideo, emptyThrowError) {
  auto memIo = std::make_unique<MemIo>();
  AsfVideo asf(std::move(memIo));
  ASSERT_THROW(asf.readMetadata(), Exiv2::Error);
}

TEST(AsfVideo, printStructurePrintsNothingAndthrowError) {
  auto memIo = std::make_unique<MemIo>();
  AsfVideo asf(std::move(memIo));

  std::ostringstream stream;

  ASSERT_THROW(asf.printStructure(stream, Exiv2::kpsNone, 1), Exiv2::Error);

  ASSERT_TRUE(stream.str().empty());
}

TEST(AsfVideo, readMetadata) {
  auto memIo = std::make_unique<MemIo>();
  AsfVideo asf(std::move(memIo));
  XmpData xmpData;
  xmpData["Xmp.video.TotalStream"] = 1000;
  xmpData["Xmp.video.TimecodeScale"] = 10001;
  xmpData["Xmp.video.AspectRatio"] = "4:3";
  ASSERT_NO_THROW(asf.setXmpData(xmpData));
  auto data = asf.xmpData();
  ASSERT_FALSE(data.empty());
  ASSERT_EQ(xmpData["Xmp.video.TotalStream"].count(), 4u);
}
