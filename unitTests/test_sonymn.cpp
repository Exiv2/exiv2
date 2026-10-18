// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>

#include <algorithm>
#include <vector>

using namespace Exiv2;

namespace {

// Add a typed value without relying on the tag's default type.
void addSonyValue(ExifData& data, const char* key, TypeId type, const char* text) {
  auto value = Value::create(type);
  ASSERT_EQ(0, value->read(text));
  data.add(ExifKey(key), value.get());
}

// Construct mixed-width Sony data with a recognized encrypted array selector.
ExifData sonyFixture() {
  ExifData data;
  data["Exif.Image.Make"] = "SONY";
  data["Exif.Image.Model"] = "ILCE-7RM3";

  // Decrypted 215 encodes to selector 0x26; neighboring bytes must not overlap.
  addSonyValue(data, "Exif.SonyMisc3c.0x0000", unsignedByte, "215");
  addSonyValue(data, "Exif.SonyMisc3c.0x0001", unsignedByte, "0");
  addSonyValue(data, "Exif.SonyMisc3c.0x0054", unsignedByte, "73");
  addSonyValue(data, "Exif.SonyMisc3c.0x0055", unsignedByte, "255");
  addSonyValue(data, "Exif.SonyMisc3c.SonyImageHeight", unsignedShort, "5304");
  addSonyValue(data, "Exif.SonyMisc3c.ShotNumberSincePowerUp", unsignedLong, "123456");
  return data;
}

// Compare the complete decoded array, including its unnamed gap bytes.
void expectSonyValuesEqual(const ExifData& expected, const ExifData& actual) {
  const auto inGroup = [](const auto& datum) { return datum.groupName() == "SonyMisc3c"; };
  const auto expectedCount = std::count_if(expected.begin(), expected.end(), inGroup);
  ASSERT_GT(expectedCount, 0);
  ASSERT_EQ(expectedCount, std::count_if(actual.begin(), actual.end(), inGroup));

  for (const auto& datum : expected) {
    if (!inGroup(datum))
      continue;

    SCOPED_TRACE(datum.key());
    const auto found = actual.findKey(ExifKey(datum.key()));
    ASSERT_NE(actual.end(), found);
    ASSERT_EQ(datum.typeId(), found->typeId());
    ASSERT_EQ(datum.count(), found->count());

    // Normalize byte order for a raw comparison of both byte and wider fields.
    std::vector<byte> original(datum.size());
    std::vector<byte> reopened(found->size());
    ASSERT_EQ(original.size(), datum.copy(original.data(), littleEndian));
    ASSERT_EQ(reopened.size(), found->copy(reopened.data(), littleEndian));
    EXPECT_EQ(original, reopened);
  }
}

// Run fresh serialization with either outer TIFF byte order.
class SonyMisc3cSerialization : public ::testing::TestWithParam<ByteOrder> {};

// Unknown offsets must use byte storage without changing named field widths.
TEST(SonyMisc3c, DefaultTypes) {
  EXPECT_EQ(unsignedByte, ExifKey("Exif.SonyMisc3c.0x0054").defaultTypeId());
  EXPECT_EQ(unsignedByte, ExifKey("Exif.SonyMisc3c.0x0055").defaultTypeId());
  EXPECT_EQ(unsignedShort, ExifKey("Exif.SonyMisc3c.SonyImageHeight").defaultTypeId());
  EXPECT_EQ(unsignedLong, ExifKey("Exif.SonyMisc3c.ShotNumberSincePowerUp").defaultTypeId());
}

// String setters must produce numeric bytes, including the byte boundaries.
TEST(SonyMisc3c, StringAssignment) {
  ExifData data;
  data["Exif.SonyMisc3c.0x0054"] = "0 73 255";
  const auto& datum = data["Exif.SonyMisc3c.0x0054"];
  ASSERT_EQ(unsignedByte, datum.typeId());
  ASSERT_EQ(3U, datum.count());
  EXPECT_EQ(0, datum.toInt64(0));
  EXPECT_EQ(73, datum.toInt64(1));
  EXPECT_EQ(255, datum.toInt64(2));
}

// Rebuilding decoded metadata through strings must preserve every array field.
TEST_P(SonyMisc3cSerialization, FreshStringCopy) {
  auto source = sonyFixture();
  Blob encoded;
  ASSERT_NO_THROW(ExifParser::encode(encoded, GetParam(), source));
  ExifData decoded;
  ASSERT_EQ(GetParam(), ExifParser::decode(decoded, encoded.data(), encoded.size(), DecodeParams{100}));

  // Rebuild the MakerNote from its decoded fields, not its raw parent buffer.
  ExifData copied;
  for (const auto& datum : decoded) {
    if (datum.key() == "Exif.Photo.MakerNote" || datum.groupName() == "MakerNote")
      continue;

    copied[datum.key()] = datum.toString();
  }

  Blob rewritten;
  ASSERT_NO_THROW(ExifParser::encode(rewritten, GetParam(), copied));
  ExifData reopened;
  ASSERT_EQ(GetParam(), ExifParser::decode(reopened, rewritten.data(), rewritten.size(), DecodeParams{100}));
  expectSonyValuesEqual(decoded, reopened);
}

INSTANTIATE_TEST_SUITE_P(OuterByteOrders, SonyMisc3cSerialization, ::testing::Values(littleEndian, bigEndian));

}  // namespace
