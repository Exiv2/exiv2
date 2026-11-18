// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>

#include <string>

using namespace Exiv2;

namespace {
std::string xmpValue(const XmpData& xmpData, const std::string& key) {
  auto pos = xmpData.findKey(XmpKey(key));
  return pos == xmpData.end() ? std::string("<missing>") : pos->toString();
}
}  // namespace

// Exif -> XMP: the UTC offset of each date (Exif 2.31 OffsetTime*) is part of the XMP date

TEST(ConvertExifDate, offsetOfEachDateGoesIntoTheXmpDate) {
  ExifData exifData;
  exifData["Exif.Image.DateTime"] = "2026:10:19 18:52:04";
  exifData["Exif.Photo.OffsetTime"] = "-05:00";
  exifData["Exif.Photo.DateTimeOriginal"] = "2026:10:16 08:56:59";
  exifData["Exif.Photo.SubSecTimeOriginal"] = "306";
  exifData["Exif.Photo.OffsetTimeOriginal"] = "-06:00";
  exifData["Exif.Photo.DateTimeDigitized"] = "2026:10:16 08:56:59";
  exifData["Exif.Photo.OffsetTimeDigitized"] = "+05:30";

  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);

  EXPECT_EQ(xmpValue(xmpData, "Xmp.xmp.ModifyDate"), "2026-10-19T18:52:04-05:00");
  EXPECT_EQ(xmpValue(xmpData, "Xmp.photoshop.DateCreated"), "2026-10-16T08:56:59.306-06:00");
  EXPECT_EQ(xmpValue(xmpData, "Xmp.xmp.CreateDate"), "2026-10-16T08:56:59+05:30");
}

TEST(ConvertExifDate, withoutOffsetTheXmpDateHasNone) {
  ExifData exifData;
  exifData["Exif.Photo.DateTimeOriginal"] = "2026:10:16 08:56:59";
  exifData["Exif.Photo.SubSecTimeOriginal"] = "306";

  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);

  EXPECT_EQ(xmpValue(xmpData, "Xmp.photoshop.DateCreated"), "2026-10-16T08:56:59.306");
}

TEST(ConvertExifDate, offsetOfAnotherDateIsNotUsed) {
  ExifData exifData;
  exifData["Exif.Photo.DateTimeOriginal"] = "2026:10:16 08:56:59";
  exifData["Exif.Photo.OffsetTime"] = "-06:00";

  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);

  EXPECT_EQ(xmpValue(xmpData, "Xmp.photoshop.DateCreated"), "2026-10-16T08:56:59");
}

TEST(ConvertExifDate, unknownOrInvalidOffsetIsIgnored) {
  for (const char* offset :
       {"   :  ", "", "-6:00", "-0600", "+24:00", "-05:60", "Z", "+05:30x", "++1:00", "+0a:00", " 05:00"}) {
    ExifData exifData;
    exifData["Exif.Photo.DateTimeOriginal"] = "2026:10:16 08:56:59";
    exifData["Exif.Photo.OffsetTimeOriginal"] = offset;

    XmpData xmpData;
    copyExifToXmp(exifData, xmpData);

    EXPECT_EQ(xmpValue(xmpData, "Xmp.photoshop.DateCreated"), "2026-10-16T08:56:59") << "offset '" << offset << "'";
  }
}

TEST(ConvertExifDate, utcAndExtremeOffsets) {
  for (const char* offset : {"+00:00", "-12:00", "+14:00", "+23:59"}) {
    ExifData exifData;
    exifData["Exif.Photo.DateTimeOriginal"] = "2026:10:16 08:56:59";
    exifData["Exif.Photo.OffsetTimeOriginal"] = offset;

    XmpData xmpData;
    copyExifToXmp(exifData, xmpData);

    EXPECT_EQ(xmpValue(xmpData, "Xmp.photoshop.DateCreated"), std::string("2026-10-16T08:56:59") + offset);
  }
}

TEST(ConvertExifDate, moveErasesTheOffsetWithTheDate) {
  ExifData exifData;
  exifData["Exif.Photo.DateTimeOriginal"] = "2026:10:16 08:56:59";
  exifData["Exif.Photo.OffsetTimeOriginal"] = "-06:00";
  exifData["Exif.Photo.OffsetTime"] = "-06:00";

  XmpData xmpData;
  moveExifToXmp(exifData, xmpData);

  EXPECT_EQ(xmpValue(xmpData, "Xmp.photoshop.DateCreated"), "2026-10-16T08:56:59-06:00");
  EXPECT_EQ(exifData.findKey(ExifKey("Exif.Photo.DateTimeOriginal")), exifData.end());
  EXPECT_EQ(exifData.findKey(ExifKey("Exif.Photo.OffsetTimeOriginal")), exifData.end());
  // Not converted (there's no Exif.Image.DateTime), so not moved
  EXPECT_NE(exifData.findKey(ExifKey("Exif.Photo.OffsetTime")), exifData.end());
}

// XMP -> Exif: a date with a time zone keeps its own local time, and the zone goes into the Exif offset

namespace {
std::string exifValue(const ExifData& exifData, const std::string& key) {
  auto pos = exifData.findKey(ExifKey(key));
  return pos == exifData.end() ? std::string("<missing>") : pos->toString();
}
}  // namespace

TEST(ConvertXmpDate, zoneGoesIntoTheOffsetOfEachDate) {
  XmpData xmpData;
  xmpData["Xmp.xmp.ModifyDate"] = "2026-10-19T18:52:04-05:00";
  xmpData["Xmp.photoshop.DateCreated"] = "2025-06-21T11:19:25.365-04:00";
  xmpData["Xmp.xmp.CreateDate"] = "2026-10-16T08:56:59+05:30";

  ExifData exifData;
  copyXmpToExif(xmpData, exifData);

  // Independent of the time zone of the machine
  EXPECT_EQ(exifValue(exifData, "Exif.Image.DateTime"), "2026:10:19 18:52:04");
  EXPECT_EQ(exifValue(exifData, "Exif.Photo.OffsetTime"), "-05:00");
  EXPECT_EQ(exifValue(exifData, "Exif.Photo.DateTimeOriginal"), "2025:06:21 11:19:25");
  EXPECT_EQ(exifValue(exifData, "Exif.Photo.OffsetTimeOriginal"), "-04:00");
  EXPECT_EQ(exifValue(exifData, "Exif.Photo.DateTimeDigitized"), "2026:10:16 08:56:59");
  EXPECT_EQ(exifValue(exifData, "Exif.Photo.OffsetTimeDigitized"), "+05:30");
}

TEST(ConvertXmpDate, utcIsOffsetZero) {
  XmpData xmpData;
  xmpData["Xmp.photoshop.DateCreated"] = "2026-10-16T14:56:59Z";

  ExifData exifData;
  copyXmpToExif(xmpData, exifData);

  EXPECT_EQ(exifValue(exifData, "Exif.Photo.DateTimeOriginal"), "2026:10:16 14:56:59");
  EXPECT_EQ(exifValue(exifData, "Exif.Photo.OffsetTimeOriginal"), "+00:00");
}

TEST(ConvertXmpDate, withoutZoneTheOffsetIsLeftAlone) {
  XmpData xmpData;
  xmpData["Xmp.photoshop.DateCreated"] = "2026-10-16T08:56:59";

  ExifData exifData;
  copyXmpToExif(xmpData, exifData);
  EXPECT_EQ(exifValue(exifData, "Exif.Photo.DateTimeOriginal"), "2026:10:16 08:56:59");
  EXPECT_EQ(exifValue(exifData, "Exif.Photo.OffsetTimeOriginal"), "<missing>");

  ExifData withOffset;
  withOffset["Exif.Photo.OffsetTimeOriginal"] = "-06:00";
  copyXmpToExif(xmpData, withOffset);
  EXPECT_EQ(exifValue(withOffset, "Exif.Photo.DateTimeOriginal"), "2026:10:16 08:56:59");
  EXPECT_EQ(exifValue(withOffset, "Exif.Photo.OffsetTimeOriginal"), "-06:00");
}

TEST(ConvertXmpDate, roundTripKeepsTimeAndOffset) {
  ExifData exifData;
  exifData["Exif.Photo.DateTimeOriginal"] = "2025:06:21 11:19:25";
  exifData["Exif.Photo.OffsetTimeOriginal"] = "-04:00";

  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);
  ExifData back;
  copyXmpToExif(xmpData, back);

  EXPECT_EQ(exifValue(back, "Exif.Photo.DateTimeOriginal"), "2025:06:21 11:19:25");
  EXPECT_EQ(exifValue(back, "Exif.Photo.OffsetTimeOriginal"), "-04:00");
}
