// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>

#include <string>
#include <vector>

using namespace Exiv2;

namespace {
class Canon5DLens : public testing::Test {
 protected:
  static constexpr auto lensName = "Canon EF 24-105mm f/3.5-5.6 IS STM";
  ExifData exif;

  void cameraInfo(size_t size = 1024, uint16_t lensType = 4152, TypeId type = undefined) {
    std::vector<byte> bytes(size, 0);
    if (size > 0x97)
      bytes[0x97] = static_cast<byte>(lensType >> 8);
    if (size > 0x98)
      bytes[0x98] = static_cast<byte>(lensType & 0xff);
    auto value = Value::create(type);
    if (!bytes.empty()) {
      ASSERT_EQ(0, value->read(bytes.data(), bytes.size(), littleEndian));
    }
    exif["Exif.Canon.CameraInfo"].setValue(value.get());
  }

  std::string printLens() const {
    return exif.findKey(ExifKey("Exif.CanonCs.LensType"))->print(&exif);
  }

  void SetUp() override {
    // The metadata arrangement in IMG_9258.CR2 from RawTherapee issue #7496.
    exif["Exif.Image.Make"] = "Canon";
    exif["Exif.Image.Model"] = "Canon EOS 5D";
    exif["Exif.CanonCs.LensType"] = uint16_t{0};
    exif["Exif.CanonCs.Lens"] = "105 24 1";
    exif["Exif.CanonCs.MaxAperture"] = uint16_t{140};
    exif["Exif.Canon.LensModel"] = std::string(64, '\0');
    cameraInfo();
  }
};
}  // namespace

TEST_F(Canon5DLens, identifiesLensWithoutChangingRawMetadata) {
  EXPECT_EQ(lensName, printLens());
  const auto selected = Exiv2::lensName(exif);
  ASSERT_NE(exif.end(), selected);
  EXPECT_EQ("Exif.CanonCs.LensType", selected->key());
  EXPECT_EQ(lensName, selected->print(&exif));
  EXPECT_EQ(0, selected->toInt64());
  EXPECT_EQ(64u, exif["Exif.Canon.LensModel"].count());
  EXPECT_EQ("", exif["Exif.Canon.LensModel"].toString());
  EXPECT_EQ(undefined, exif["Exif.Canon.CameraInfo"].typeId());
  EXPECT_EQ(1024u, exif["Exif.Canon.CameraInfo"].count());
  EXPECT_EQ(0, exif["Exif.Canon.CameraInfo"].toInt64(0x0c));
  EXPECT_EQ(0x10, exif["Exif.Canon.CameraInfo"].toInt64(0x97));
  EXPECT_EQ(0x38, exif["Exif.Canon.CameraInfo"].toInt64(0x98));
}

TEST_F(Canon5DLens, readsBigEndianLensIdRegardlessOfExifByteOrder) {
  for (const auto order : {littleEndian, bigEndian}) {
    SCOPED_TRACE(order);
    Blob encoded;
    ExifParser::encode(encoded, order, exif);
    ExifData decoded;
    ASSERT_EQ(order, ExifParser::decode(decoded, encoded.data(), encoded.size(), DecodeParams{500}));
    const auto selected = Exiv2::lensName(decoded);
    ASSERT_NE(decoded.end(), selected);
    EXPECT_EQ(lensName, selected->print(&decoded));
    EXPECT_EQ(0, selected->toInt64());
  }
}

TEST_F(Canon5DLens, acceptsCameraInfoEndingImmediatelyAfterLensId) {
  cameraInfo(0x99);
  EXPECT_EQ(lensName, printLens());
}

TEST_F(Canon5DLens, handlesMissingCameraInfo) {
  exif.erase(exif.findKey(ExifKey("Exif.Canon.CameraInfo")));
  EXPECT_EQ("Unknown Lens (0)", printLens());
}

TEST_F(Canon5DLens, handlesEveryTruncatedCameraInfoLength) {
  for (size_t size = 0; size < 0x99; ++size) {
    SCOPED_TRACE(size);
    cameraInfo(size);
    EXPECT_EQ("Unknown Lens (0)", printLens());
  }
}

TEST_F(Canon5DLens, ignoresCameraInfoWithUnexpectedType) {
  for (const auto type : {unsignedByte, unsignedShort, asciiString}) {
    SCOPED_TRACE(type);
    cameraInfo(1024, 4152, type);
    EXPECT_EQ("Unknown Lens (0)", printLens());
  }
}

TEST_F(Canon5DLens, ignoresOtherCameraModels) {
  for (const auto model :
       {"Canon EOS 5D Mark II", "Canon EOS 5D Mark III", "Canon EOS 5DS", "Canon EOS 30D", "EOS 5D", ""}) {
    SCOPED_TRACE(model);
    exif["Exif.Image.Model"] = model;
    EXPECT_EQ("Unknown Lens (0)", printLens());
  }
  exif.erase(exif.findKey(ExifKey("Exif.Image.Model")));
  EXPECT_EQ("Unknown Lens (0)", printLens());
}

TEST_F(Canon5DLens, preservesNonzeroCameraSettingsLensId) {
  exif["Exif.CanonCs.LensType"] = uint16_t{4152};
  cameraInfo(1024, 0x1234);
  EXPECT_EQ(lensName, printLens());

  exif["Exif.CanonCs.LensType"] = uint16_t{0x1234};
  cameraInfo();
  EXPECT_EQ("Unknown Lens (4660)", printLens());

  exif["Exif.CanonCs.LensType"] = uint16_t{0xffff};
  EXPECT_EQ("n/a", printLens());
}

TEST_F(Canon5DLens, ignoresUnsetCameraInfoLensId) {
  for (const uint16_t id : {0, 0xffff}) {
    SCOPED_TRACE(id);
    cameraInfo(1024, id);
    EXPECT_EQ("Unknown Lens (0)", printLens());
  }
}

TEST_F(Canon5DLens, reportsUnknownAlternateLensId) {
  cameraInfo(1024, 0x1234);
  EXPECT_EQ("Unknown Lens (4660)", printLens());
}

TEST_F(Canon5DLens, stillChecksFocalLengthAndAperture) {
  exif["Exif.CanonCs.Lens"] = "200 70 1";
  EXPECT_EQ("Unknown Lens (4152)", printLens());
  exif["Exif.CanonCs.Lens"] = "105 24 1";
  exif["Exif.CanonCs.MaxAperture"] = uint16_t{32};
  EXPECT_EQ("Unknown Lens (4152)", printLens());
}

TEST_F(Canon5DLens, handlesMissingLensMetadata) {
  for (const auto key : {"Exif.CanonCs.Lens", "Exif.CanonCs.MaxAperture"}) {
    SCOPED_TRACE(key);
    auto incomplete = exif;
    incomplete.erase(incomplete.findKey(ExifKey(key)));
    EXPECT_EQ("Unknown Lens (4152)", incomplete["Exif.CanonCs.LensType"].print(&incomplete));
  }
}
