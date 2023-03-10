// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <array>
#include <exiv2/asfvideo.hpp>

using namespace Exiv2;

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
  MemIo memIo;
  ASSERT_FALSE(isAsfType(memIo, false));
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
  ASSERT_EQ(xmpData["Xmp.video.TotalStream"].count(), 4);
}
