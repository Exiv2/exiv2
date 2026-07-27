// SPDX-License-Identifier: GPL-2.0-or-later

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exiv2/basicio.hpp>
#include <exiv2/matroskavideo.hpp>

#include "mock_basicio.hpp"

using namespace Exiv2;

TEST(MatroskaVideo, canBeOpenedWithEmptyMemIo) {
  auto memIo = std::make_unique<MemIo>();
  ASSERT_NO_THROW(MatroskaVideo mkv(std::move(memIo)));
}

TEST(MatroskaVideo, mimeTypeIsMkv) {
  auto memIo = std::make_unique<MemIo>();
  MatroskaVideo mkv(std::move(memIo));

  ASSERT_EQ("video/matroska", mkv.mimeType());
}

TEST(MatroskaVideo, isMkvTypewithEmptyDataReturnsFalse) {
  auto mockIo = makeMockIo();
  setupReadFailure(*mockIo);
  ASSERT_FALSE(isMkvType(*mockIo, false));
}

TEST(MatroskaVideo, isMkvTypeWithValidSignatureReturnsTrue) {
  auto mockIo = makeMockIo();
  setupRead(*mockIo, {0x1a, 0x45, 0xdf, 0xa3});
  ASSERT_TRUE(isMkvType(*mockIo, false));
}

TEST(MatroskaVideo, emptyThrowError) {
  auto memIo = std::make_unique<MemIo>();
  MatroskaVideo mkv(std::move(memIo));
  ASSERT_THROW(mkv.readMetadata(), Exiv2::Error);
}

TEST(MatroskaVideo, printStructurePrintsNothingAndthrowError) {
  auto memIo = std::make_unique<MemIo>();
  MatroskaVideo mkv(std::move(memIo));

  std::ostringstream stream;

  ASSERT_THROW(mkv.printStructure(stream, Exiv2::kpsNone, 1), Exiv2::Error);

  ASSERT_TRUE(stream.str().empty());
}

TEST(MatroskaVideo, readMetadata) {
  auto memIo = std::make_unique<MemIo>();
  MatroskaVideo mkv(std::move(memIo));
  XmpData xmpData;
  xmpData["Xmp.video.TotalStream"] = 1000;
  xmpData["Xmp.video.TimecodeScale"] = 10001;
  xmpData["Xmp.video.AspectRatio"] = "4:3";
  ASSERT_NO_THROW(mkv.setXmpData(xmpData));
  auto data = mkv.xmpData();
  ASSERT_FALSE(data.empty());
  ASSERT_EQ(xmpData["Xmp.video.TotalStream"].count(), 4u);
}
