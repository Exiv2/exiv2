// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <exiv2/convert.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>
#include <exiv2/xmp_exiv2.hpp>

#include <filesystem>

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
