// SPDX-License-Identifier: GPL-2.0-or-later

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exiv2/basicio.hpp>
#include <exiv2/riffvideo.hpp>

#include "mock_basicio.hpp"

using namespace Exiv2;

TEST(RiffVideo, canBeOpenedWithEmptyMemIo) {
  auto memIo = std::make_unique<MemIo>();
  ASSERT_NO_THROW(RiffVideo riff(std::move(memIo)));
}

TEST(RiffVideo, mimeTypeIsRiff) {
  auto memIo = std::make_unique<MemIo>();
  RiffVideo riff(std::move(memIo));

  ASSERT_EQ("video/riff", riff.mimeType());
}

TEST(RiffVideo, isRiffTypewithEmptyDataReturnsFalse) {
  auto mockIo = makeMockIo();
  setupReadFailure(*mockIo);
  ASSERT_FALSE(isRiffType(*mockIo, false));
}

TEST(RiffVideo, isRiffTypeWithValidSignatureReturnsTrue) {
  auto mockIo = makeMockIo();
  setupRead(*mockIo, {'R', 'I', 'F', 'F'});
  ASSERT_TRUE(isRiffType(*mockIo, false));
}

TEST(RiffVideo, emptyThrowError) {
  auto memIo = std::make_unique<MemIo>();
  RiffVideo riff(std::move(memIo));
  ASSERT_THROW(riff.readMetadata(), Exiv2::Error);
}

TEST(RiffVideo, printStructurePrintsNothingAndthrowError) {
  auto memIo = std::make_unique<MemIo>();
  RiffVideo riff(std::move(memIo));

  std::ostringstream stream;

  ASSERT_THROW(riff.printStructure(stream, Exiv2::kpsNone, 1), Exiv2::Error);

  ASSERT_TRUE(stream.str().empty());
}

TEST(RiffVideo, readMetadata) {
  auto memIo = std::make_unique<MemIo>();
  RiffVideo riff(std::move(memIo));
  XmpData xmpData;
  xmpData["Xmp.video.TotalStream"] = 1000;
  xmpData["Xmp.video.TimecodeScale"] = 10001;
  xmpData["Xmp.video.AspectRatio"] = "4:3";
  ASSERT_NO_THROW(riff.setXmpData(xmpData));
  auto data = riff.xmpData();
  ASSERT_FALSE(data.empty());
  ASSERT_EQ(xmpData["Xmp.video.TotalStream"].count(), 4u);
}
