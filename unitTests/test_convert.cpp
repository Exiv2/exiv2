// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <exiv2/config.h>
#include <exiv2/convert.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>
#include <exiv2/iptc.hpp>
#include <exiv2/tags.hpp>
#include <exiv2/value.hpp>
#include <exiv2/xmp_exiv2.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using namespace Exiv2;

TEST(Convert, exifSurvivesARoundTripThroughXmp) {
  const auto path = fs::path(::testing::TempDir()) / "convert-test.jpg";
  fs::copy_file(TESTDATA_PATH "/DSC_3079.jpg", path, fs::copy_options::overwrite_existing);
  {
    auto image = ImageFactory::open(path.string());
    image->readMetadata();

    XmpData xmpData;
    copyExifToXmp(image->exifData(), xmpData);
    EXPECT_EQ(xmpData["Xmp.tiff.Model"].toString(), "D5803");
    EXPECT_FLOAT_EQ(xmpData["Xmp.exif.FNumber"].toFloat(), 2.0F);

    ExifData exifData;
    copyXmpToExif(xmpData, exifData);
    image->setXmpData(xmpData);
    image->setExifData(exifData);
    ASSERT_NO_THROW(image->writeMetadata());
  }
  {
    auto image = ImageFactory::open(path.string());
    image->readMetadata();
    auto& exifData = image->exifData();
    EXPECT_EQ(exifData["Exif.Image.Make"].toString(), "Sony");
    EXPECT_EQ(exifData["Exif.Image.Model"].toString(), "D5803");
    EXPECT_EQ(exifData["Exif.Photo.DateTimeOriginal"].toString(), "2018:01:04 11:42:41");
    EXPECT_FLOAT_EQ(exifData["Exif.Photo.FNumber"].toFloat(), 2.0F);
    EXPECT_EQ(image->xmpData()["Xmp.tiff.Model"].toString(), "D5803");
  }
  fs::remove(path);
}

TEST(Convert, moveExifToXmpErasesTheExifSource) {
  ExifData exifData;
  exifData["Exif.Photo.FNumber"] = "28/10";
  exifData["Exif.Image.Artist"] = "Somebody";
  XmpData xmpData;
  moveExifToXmp(exifData, xmpData);
  EXPECT_FLOAT_EQ(xmpData["Xmp.exif.FNumber"].toFloat(), 2.8F);
  EXPECT_EQ(xmpData["Xmp.dc.creator"].toString(0), "Somebody");
  EXPECT_TRUE(exifData.empty());
}

TEST(Convert, moveXmpToExifErasesTheXmpSource) {
  XmpData xmpData;
  xmpData["Xmp.exif.FNumber"] = "28/10";
  xmpData["Xmp.tiff.Model"] = "Camera";
  ExifData exifData;
  moveXmpToExif(xmpData, exifData);
  EXPECT_FLOAT_EQ(exifData["Exif.Photo.FNumber"].toFloat(), 2.8F);
  EXPECT_EQ(exifData["Exif.Image.Model"].toString(), "Camera");
  EXPECT_TRUE(xmpData.empty());
}

TEST(Convert, copyOverwritesAnExistingTarget) {
  ExifData exifData;
  exifData["Exif.Photo.FNumber"] = "2/1";
  XmpData xmpData;
  xmpData["Xmp.exif.FNumber"] = "4/1";
  copyExifToXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.exif.FNumber"].toString(), "2/1");

  xmpData["Xmp.exif.FNumber"] = "8/1";
  copyXmpToExif(xmpData, exifData);
  EXPECT_EQ(exifData["Exif.Photo.FNumber"].toString(), "8/1");
}

TEST(Convert, flashBitsRoundTrip) {
  ExifData exifData;
  exifData["Exif.Photo.Flash"] = static_cast<uint16_t>(0x4f);  // fired, return 3, mode 1, red-eye
  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.exif.Flash/exif:Fired"].toString(), "True");
  EXPECT_EQ(xmpData["Xmp.exif.Flash/exif:Return"].toInt64(), 3);
  EXPECT_EQ(xmpData["Xmp.exif.Flash/exif:Mode"].toInt64(), 1);
  EXPECT_EQ(xmpData["Xmp.exif.Flash/exif:Function"].toString(), "False");
  EXPECT_EQ(xmpData["Xmp.exif.Flash/exif:RedEyeMode"].toString(), "True");

  ExifData back;
  copyXmpToExif(xmpData, back);
  EXPECT_EQ(back["Exif.Photo.Flash"].toInt64(), 0x4f);
}

TEST(Convert, versionsRoundTrip) {
  ExifData exifData;
  exifData["Exif.Photo.ExifVersion"] = "48 50 51 48";
  exifData["Exif.GPSInfo.GPSVersionID"] = "2 3 0 0";
  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.exif.ExifVersion"].toString(), "0230");
  EXPECT_EQ(xmpData["Xmp.exif.GPSVersionID"].toString(), "2.3.0.0");

  ExifData back;
  copyXmpToExif(xmpData, back);
  EXPECT_EQ(back["Exif.Photo.ExifVersion"].toString(), "48 50 51 48");
  EXPECT_EQ(back["Exif.GPSInfo.GPSVersionID"].toString(), "2 3 0 0");
}

TEST(Convert, xmpVersionTooShortIsNotConverted) {
  XmpData xmpData;
  xmpData["Xmp.exif.ExifVersion"] = "02";
  ExifData exifData;
  copyXmpToExif(xmpData, exifData);
  EXPECT_EQ(exifData.findKey(ExifKey("Exif.Photo.ExifVersion")), exifData.end());
}

TEST(Convert, gpsCoordinatesRoundTrip) {
  ExifData exifData;
  exifData["Exif.GPSInfo.GPSLatitude"] = "47/1 30/1 1800/100";
  exifData["Exif.GPSInfo.GPSLatitudeRef"] = "N";
  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.exif.GPSLatitude"].toString(), "47,30.3000000N");

  ExifData back;
  copyXmpToExif(xmpData, back);
  const auto& lat = back["Exif.GPSInfo.GPSLatitude"];
  ASSERT_EQ(lat.count(), 3U);
  EXPECT_FLOAT_EQ(lat.toFloat(0), 47.0F);
  EXPECT_FLOAT_EQ(lat.toFloat(1), 30.0F);
  EXPECT_NEAR(lat.toFloat(2), 18.0F, 0.001F);
  EXPECT_EQ(back["Exif.GPSInfo.GPSLatitudeRef"].toString(), "N");
}

TEST(Convert, xmpGpsCoordinateWithSeconds) {
  XmpData xmpData;
  xmpData["Xmp.exif.GPSLongitude"] = "12,34,56W";
  ExifData exifData;
  copyXmpToExif(xmpData, exifData);
  const auto& lon = exifData["Exif.GPSInfo.GPSLongitude"];
  ASSERT_EQ(lon.count(), 3U);
  EXPECT_FLOAT_EQ(lon.toFloat(0), 12.0F);
  EXPECT_FLOAT_EQ(lon.toFloat(1), 34.0F);
  EXPECT_FLOAT_EQ(lon.toFloat(2), 56.0F);
  EXPECT_EQ(exifData["Exif.GPSInfo.GPSLongitudeRef"].toString(), "W");
}

TEST(Convert, invalidGpsCoordinatesAreNotConverted) {
  XmpData xmpData;
  xmpData["Xmp.exif.GPSLatitude"] = "12,34.5X";   // bad reference
  xmpData["Xmp.exif.GPSLongitude"] = "12;34.5E";  // bad separator
  ExifData exifData;
  copyXmpToExif(xmpData, exifData);
  EXPECT_TRUE(exifData.empty());

  ExifData noRef;
  noRef["Exif.GPSInfo.GPSLatitude"] = "47/1 30/1 0/1";
  ExifData zeroDenominator;
  zeroDenominator["Exif.GPSInfo.GPSLatitude"] = "47/1 30/0 0/1";
  zeroDenominator["Exif.GPSInfo.GPSLatitudeRef"] = "N";
  XmpData out;
  copyExifToXmp(noRef, out);
  copyExifToXmp(zeroDenominator, out);
  EXPECT_TRUE(out.empty());
}

TEST(Convert, userCommentRoundTrip) {
  ExifData exifData;
  exifData["Exif.Photo.UserComment"] = "charset=Ascii A comment";
  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.exif.UserComment"].toString(), "lang=\"x-default\" A comment");

  ExifData back;
  copyXmpToExif(xmpData, back);
  const auto& comment = dynamic_cast<const CommentValue&>(back["Exif.Photo.UserComment"].value());
  EXPECT_EQ(comment.charsetId(), CommentValue::unicode);
  EXPECT_EQ(comment.comment(), "A comment");
}

#ifdef EXV_HAVE_XMP_TOOLKIT
TEST(Convert, dateWithSubSecondsRoundTrip) {
  ExifData exifData;
  exifData["Exif.Photo.DateTimeOriginal"] = "2021:03:04 05:06:07";
  exifData["Exif.Photo.SubSecTimeOriginal"] = "123";
  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.photoshop.DateCreated"].toString(), "2021-03-04T05:06:07.123");

  ExifData back;
  copyXmpToExif(xmpData, back);
  EXPECT_EQ(back["Exif.Photo.DateTimeOriginal"].toString(), "2021:03:04 05:06:07");
  EXPECT_EQ(back["Exif.Photo.SubSecTimeOriginal"].toString(), "123000000");
}

TEST(Convert, unparsableDatesAreNotConverted) {
  ExifData exifData;
  exifData["Exif.Image.DateTime"] = "not a date";
  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);
  EXPECT_EQ(xmpData.findKey(XmpKey("Xmp.xmp.ModifyDate")), xmpData.end());

  xmpData["Xmp.xmp.ModifyDate"] = "not a date";
  ExifData back;
  copyXmpToExif(xmpData, back);
  EXPECT_EQ(back.findKey(ExifKey("Exif.Image.DateTime")), back.end());
}

TEST(Convert, gpsTimeStampRoundTrip) {
  ExifData exifData;
  exifData["Exif.GPSInfo.GPSTimeStamp"] = "10/1 20/1 3050/100";
  exifData["Exif.GPSInfo.GPSDateStamp"] = "2020:01:02";
  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.exif.GPSTimeStamp"].toString(), "2020-01-02T10:20:30.500000000");

  ExifData back;
  copyXmpToExif(xmpData, back);
  const auto& time = back["Exif.GPSInfo.GPSTimeStamp"];
  ASSERT_EQ(time.count(), 3U);
  EXPECT_FLOAT_EQ(time.toFloat(0), 10.0F);
  EXPECT_FLOAT_EQ(time.toFloat(1) * 60 + time.toFloat(2), 1230.5F);
  EXPECT_EQ(back["Exif.GPSInfo.GPSDateStamp"].toString(), "2020:01:02");
}

TEST(Convert, gpsTimeStampNeedsADate) {
  ExifData exifData;
  exifData["Exif.GPSInfo.GPSTimeStamp"] = "10/1 20/1 30/1";
  XmpData xmpData;
  copyExifToXmp(exifData, xmpData);
  EXPECT_TRUE(xmpData.empty());
}

TEST(Convert, firstSyncDoesNotOverwriteExistingXmp) {
  ExifData exifData;
  exifData["Exif.Photo.FNumber"] = "2/1";
  exifData["Exif.Photo.ExposureTime"] = "1/100";
  XmpData xmpData;
  xmpData["Xmp.exif.FNumber"] = "4/1";
  syncExifWithXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.exif.FNumber"].toString(), "4/1");
  EXPECT_EQ(xmpData["Xmp.exif.ExposureTime"].toString(), "1/100");
}

TEST(Convert, syncExifWithXmpFollowsTheNativeDigests) {
  ExifData exifData;
  exifData["Exif.Photo.FNumber"] = "2/1";
  XmpData xmpData;

  // No digests yet: Exif is copied to XMP and the digests are written
  syncExifWithXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.exif.FNumber"].toString(), "2/1");
  EXPECT_NE(xmpData.findKey(XmpKey("Xmp.tiff.NativeDigest")), xmpData.end());
  EXPECT_NE(xmpData.findKey(XmpKey("Xmp.exif.NativeDigest")), xmpData.end());

  // Only XMP changed, so the digests still match Exif: XMP wins
  xmpData["Xmp.exif.FNumber"] = "4/1";
  syncExifWithXmp(exifData, xmpData);
  EXPECT_EQ(exifData["Exif.Photo.FNumber"].toString(), "4/1");

  // Exif changed after the digests were written: Exif wins
  exifData["Exif.Photo.FNumber"] = "8/1";
  syncExifWithXmp(exifData, xmpData);
  EXPECT_EQ(xmpData["Xmp.exif.FNumber"].toString(), "8/1");
}
#endif

TEST(Convert, iptcToXmpAndBack) {
  IptcData iptcData;
  iptcData.add(IptcKey("Iptc.Application2.Keywords"), StringValue("one").clone().get());
  iptcData.add(IptcKey("Iptc.Application2.Keywords"), StringValue("two").clone().get());
  iptcData["Iptc.Application2.City"] = "Z\xfcrich";
  iptcData["Iptc.Application2.ObjectName"] = "Title";
  XmpData xmpData;
  copyIptcToXmp(iptcData, xmpData, "ISO-8859-1");
  EXPECT_EQ(xmpData["Xmp.dc.subject"].count(), 2U);
  EXPECT_EQ(xmpData["Xmp.dc.subject"].toString(1), "two");
  EXPECT_EQ(xmpData["Xmp.dc.title"].toString(), "lang=\"x-default\" Title");
#ifdef EXV_HAVE_ICONV
  EXPECT_EQ(xmpData["Xmp.photoshop.City"].toString(), "Z\xc3\xbcrich");
#endif

  IptcData back;
  copyXmpToIptc(xmpData, back);
  EXPECT_EQ(back["Iptc.Envelope.CharacterSet"].toString(), "\033%G");
  EXPECT_EQ(back["Iptc.Application2.ObjectName"].toString(), "Title");
  size_t keywords = 0;
  for (const auto& d : back)
    keywords += d.key() == "Iptc.Application2.Keywords";
  EXPECT_EQ(keywords, 2U);
}

TEST(Convert, moveIptcToXmpAndBackErasesTheSource) {
  IptcData iptcData;
  iptcData["Iptc.Application2.Headline"] = "Headline";
  XmpData xmpData;
  moveIptcToXmp(iptcData, xmpData);
  EXPECT_EQ(xmpData["Xmp.photoshop.Headline"].toString(), "Headline");
  EXPECT_TRUE(iptcData.empty());

  moveXmpToIptc(xmpData, iptcData);
  EXPECT_EQ(iptcData["Iptc.Application2.Headline"].toString(), "Headline");
  EXPECT_TRUE(xmpData.empty());
}

TEST(Convert, copyXmpToIptcReplacesAllRepeatedTargets) {
  XmpData xmpData;
  xmpData["Xmp.dc.subject"] = "new";
  IptcData iptcData;
  iptcData.add(IptcKey("Iptc.Application2.Keywords"), StringValue("old1").clone().get());
  iptcData.add(IptcKey("Iptc.Application2.Keywords"), StringValue("old2").clone().get());
  copyXmpToIptc(xmpData, iptcData);
  std::vector<std::string> keywords;
  for (const auto& d : iptcData)
    if (d.key() == "Iptc.Application2.Keywords")
      keywords.push_back(d.toString());
  EXPECT_EQ(keywords, std::vector<std::string>{"new"});
}

#ifdef EXV_HAVE_ICONV
TEST(Convert, convertStringCharset) {
  std::string s = "Z\xfcrich";
  EXPECT_TRUE(convertStringCharset(s, "UTF-8", "UTF-8"));
  EXPECT_EQ(s, "Z\xfcrich");
  EXPECT_TRUE(convertStringCharset(s, "ISO-8859-1", "UTF-8"));
  EXPECT_EQ(s, "Z\xc3\xbcrich");
  EXPECT_FALSE(convertStringCharset(s, "NO-SUCH-CHARSET", "UTF-8"));
  EXPECT_EQ(s, "Z\xc3\xbcrich");
}
#endif
