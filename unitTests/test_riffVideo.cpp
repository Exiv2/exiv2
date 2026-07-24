// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <exiv2/basicio.hpp>
#include <exiv2/riffvideo.hpp>
#include "unittest_utils.hpp"

using namespace Exiv2;

TEST(RiffVideo, canBeOpenedWithEmptyMemIo) {
  auto memIo = std::make_unique<MemIo>();
  ASSERT_NO_THROW(RiffVideo riff(std::move(memIo), defaultImageCtorParams(false)));
}

TEST(RiffVideo, mimeTypeIsRiff) {
  auto memIo = std::make_unique<MemIo>();
  RiffVideo riff(std::move(memIo), defaultImageCtorParams(false));

  ASSERT_EQ("video/riff", riff.mimeType());
}

TEST(RiffVideo, isRiffTypewithEmptyDataReturnsFalse) {
  MemIo memIo;
  ASSERT_FALSE(isRiffType(memIo, false));
}

TEST(RiffVideo, emptyThrowError) {
  auto memIo = std::make_unique<MemIo>();
  RiffVideo riff(std::move(memIo), defaultImageCtorParams(false));
  ASSERT_THROW(riff.readMetadata(), Exiv2::Error);
}

TEST(RiffVideo, printStructurePrintsNothingAndthrowError) {
  auto memIo = std::make_unique<MemIo>();
  RiffVideo riff(std::move(memIo), defaultImageCtorParams(false));

  std::ostringstream stream;

  ASSERT_THROW(riff.printStructure(stream, Exiv2::kpsNone, 1), Exiv2::Error);

  ASSERT_TRUE(stream.str().empty());
}

TEST(RiffVideo, readMetadata) {
  auto memIo = std::make_unique<MemIo>();
  RiffVideo riff(std::move(memIo), defaultImageCtorParams(false));
  XmpData xmpData;
  xmpData["Xmp.video.TotalStream"] = 1000;
  xmpData["Xmp.video.TimecodeScale"] = 10001;
  xmpData["Xmp.video.AspectRatio"] = "4:3";
  ASSERT_NO_THROW(riff.setXmpData(xmpData));
  auto data = riff.xmpData();
  ASSERT_FALSE(data.empty());
  ASSERT_EQ(xmpData["Xmp.video.TotalStream"].count(), 4u);
}
