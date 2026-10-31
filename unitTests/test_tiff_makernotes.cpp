// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>

#include <algorithm>
#include <array>
#include <string>
#include <tuple>
#include <vector>

using namespace Exiv2;

namespace {
class TiffMakerNote : public testing::TestWithParam<std::tuple<ByteOrder, bool>> {
 protected:
  static constexpr size_t orientation = 30;
  static constexpr size_t exifDirectory = 64;
  static constexpr size_t makerNote = 96;
  static constexpr uint32_t makerNoteSize = 160;
  static constexpr size_t cameraId = makerNote + 128;
  std::vector<byte> data;
  ExifData exif;
  IptcData iptc;
  XmpData xmp;

  ByteOrder order() const {
    return std::get<0>(GetParam());
  }
  bool olympus2() const {
    return std::get<1>(GetParam());
  }
  std::string key(const char* name) const {
    return std::string(olympus2() ? "Exif.Olympus2." : "Exif.Olympus.") + name;
  }
  size_t makerDirectory() const {
    return makerNote + (olympus2() ? 12 : 8);
  }
  void field(size_t offset, uint16_t tag, uint16_t type, uint32_t count, uint32_t value) {
    us2Data(data.data() + offset, tag, order());
    us2Data(data.data() + offset + 2, type, order());
    ul2Data(data.data() + offset + 4, count, order());
    if (type == 3 && count == 1)
      us2Data(data.data() + offset + 8, static_cast<uint16_t>(value), order());
    else
      ul2Data(data.data() + offset + 8, value, order());
  }
  void decode(const byte* bytes, size_t size) {
    exif.clear();
    ASSERT_EQ(order(), TiffParser::decode(exif, iptc, xmp, bytes, size, DecodeParams{500}));
  }
  void SetUp() override {
    data.resize(makerNote + makerNoteSize);
    data[0] = data[1] = order() == littleEndian ? 'I' : 'M';
    us2Data(data.data() + 2, 42, order());
    ul2Data(data.data() + 4, 8, order());
    us2Data(data.data() + 8, 3, order());
    field(10, 0x010f, 2, 8, 52);
    field(22, 0x0112, 3, 1, 1);
    field(34, 0x8769, 4, 1, exifDirectory);
    std::copy_n("OLYMPUS", 8, data.begin() + 52);
    us2Data(data.data() + exifDirectory, 1, order());
    field(exifDirectory + 2, 0x927c, 7, makerNoteSize, makerNote);

    // Keep a real value beyond the compact MakerNote extent, separated by padding.
    std::fill(data.begin() + makerNote, data.end(), 0xa5);
    if (olympus2()) {
      constexpr std::array<byte, 12> header{'O', 'L', 'Y', 'M', 'P', 'U', 'S', 0, 'I', 'I', 3, 0};
      std::copy(header.begin(), header.end(), data.begin() + makerNote);
    } else {
      constexpr std::array<byte, 8> header{'O', 'L', 'Y', 'M', 'P', 0, 1, 0};
      std::copy(header.begin(), header.end(), data.begin() + makerNote);
    }
    const auto directory = makerDirectory();
    us2Data(data.data() + directory, 2, order());
    field(directory + 2, 0x0201, 3, 1, 2);
    field(directory + 14, 0x0209, 2, 6, olympus2() ? cameraId - makerNote : cameraId);
    ul2Data(data.data() + directory + 26, 0, order());
    std::copy_n("ABC12", 6, data.begin() + cameraId);
    decode(data.data(), data.size());
    ASSERT_EQ("ABC12", exif[key("CameraID")].toString());
    ASSERT_EQ(makerNoteSize, exif["Exif.Photo.MakerNote"].count());
  }
};
}  // namespace

TEST_P(TiffMakerNote, preservesEntireMakerNoteDuringRepeatedInPlaceEdits) {
  auto expected = data;
  for (uint16_t value : {8, 1, 3}) {
    exif["Exif.Image.Orientation"] = value;
    us2Data(expected.data() + orientation, value, order());
    MemIo output;
    ASSERT_EQ(wmNonIntrusive, TiffParser::encode(output, data.data(), data.size(), order(), exif, iptc, xmp));
    EXPECT_EQ(0u, output.size());
    EXPECT_EQ(expected, data);
    decode(data.data(), data.size());
    EXPECT_EQ(makerNoteSize, exif["Exif.Photo.MakerNote"].count());
    EXPECT_EQ("ABC12", exif[key("CameraID")].toString());
  }
}

TEST_P(TiffMakerNote, preservesOuterExtentWhenEditingDecodedMakerNoteValue) {
  auto expected = data;
  exif[key("Quality")] = uint16_t{3};
  us2Data(expected.data() + makerDirectory() + 10, 3, order());
  Blob output;
  ASSERT_EQ(wmNonIntrusive, ExifParser::encode(output, data.data(), data.size(), order(), exif));
  EXPECT_TRUE(output.empty());
  EXPECT_EQ(expected, data);
  decode(data.data(), data.size());
  EXPECT_EQ(makerNoteSize, exif["Exif.Photo.MakerNote"].count());
  EXPECT_EQ(3, exif[key("Quality")].toInt64());
  EXPECT_EQ("ABC12", exif[key("CameraID")].toString());
}

TEST_P(TiffMakerNote, preservesExtentWhenReconstructionWouldExpandSharedValues) {
  const auto directory = makerDirectory();
  const auto offset = olympus2() ? 80 : makerNote + 80;
  us2Data(data.data() + directory, 4, order());
  field(directory + 2, 0x0110, 7, 80, offset);
  field(directory + 14, 0x0111, 7, 80, offset);
  field(directory + 26, 0x0201, 3, 1, 2);
  field(directory + 38, 0x0209, 2, 6, olympus2() ? cameraId - makerNote : cameraId);
  ul2Data(data.data() + directory + 50, 0, order());
  decode(data.data(), data.size());
  auto expected = data;
  exif["Exif.Image.Orientation"] = uint16_t{8};
  us2Data(expected.data() + orientation, 8, order());
  MemIo output;
  ASSERT_EQ(wmNonIntrusive, TiffParser::encode(output, data.data(), data.size(), order(), exif, iptc, xmp));
  EXPECT_EQ(expected, data);
}

TEST_P(TiffMakerNote, preservesEntireMakerNoteInOrf) {
  us2Data(data.data() + 2, 0x4f52, order());
  exif.clear();
  ASSERT_EQ(order(), OrfParser::decode(exif, iptc, xmp, data.data(), data.size(), DecodeParams{500}));
  auto expected = data;
  exif["Exif.Image.Orientation"] = uint16_t{8};
  us2Data(expected.data() + orientation, 8, order());
  MemIo output;
  ASSERT_EQ(wmNonIntrusive, OrfParser::encode(output, data.data(), data.size(), order(), exif, iptc, xmp));
  EXPECT_EQ(expected, data);
  exif.clear();
  ASSERT_EQ(order(), OrfParser::decode(exif, iptc, xmp, data.data(), data.size(), DecodeParams{500}));
  EXPECT_EQ(makerNoteSize, exif["Exif.Photo.MakerNote"].count());
  EXPECT_EQ("ABC12", exif[key("CameraID")].toString());
}

TEST_P(TiffMakerNote, stillUpdatesOpaqueMakerNoteCountAndInlineData) {
  std::copy_n("UNKNOWN", 8, data.begin() + 52);
  decode(data.data(), data.size());
  auto replacement = Value::create(undefined);
  ASSERT_EQ(0, replacement->read("1 2 3"));
  exif["Exif.Photo.MakerNote"].setValue(replacement.get());
  MemIo output;
  ASSERT_EQ(wmNonIntrusive, TiffParser::encode(output, data.data(), data.size(), order(), exif, iptc, xmp));
  decode(data.data(), data.size());
  EXPECT_EQ(3u, exif["Exif.Photo.MakerNote"].count());
  EXPECT_EQ("1 2 3", exif["Exif.Photo.MakerNote"].toString());
}

TEST_P(TiffMakerNote, reconstructsConsistentMakerNoteForNewAndExistingBuffers) {
  const auto original = data;
  for (bool fresh : {false, true}) {
    for (const auto& value : {std::string("ABC12"), std::string(200, 'z')}) {
      data = original;
      decode(data.data(), data.size());
      exif["Exif.Image.Artist"] = "force reconstruction";
      exif[key("CameraID")] = value;
      MemIo output;
      ASSERT_EQ(wmIntrusive, TiffParser::encode(output, fresh ? nullptr : data.data(), fresh ? 0 : data.size(), order(),
                                                exif, iptc, xmp));
      decode(output.mmap(), output.size());
      EXPECT_EQ(value, exif[key("CameraID")].toString());
      EXPECT_EQ("force reconstruction", exif["Exif.Image.Artist"].toString());
      if (value.size() < makerNoteSize)
        EXPECT_LT(exif["Exif.Photo.MakerNote"].count(), makerNoteSize);
      else
        EXPECT_GT(exif["Exif.Photo.MakerNote"].count(), makerNoteSize);
    }
  }
}

INSTANTIATE_TEST_SUITE_P(OlympusHeadersAndByteOrders, TiffMakerNote,
                         testing::Combine(testing::Values(littleEndian, bigEndian), testing::Bool()));
