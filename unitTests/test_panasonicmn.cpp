// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>

#include <gtest/gtest.h>

#include <array>
#include <sstream>
#include <string>
#include <vector>

namespace {
using namespace Exiv2;

// Serialize with the existing TIFF machinery and read a fresh metadata view.
ExifData roundTrip(ExifData metadata, ByteOrder order = littleEndian) {
  Blob encoded;
  ExifParser::encode(encoded, order, metadata);

  ExifData decoded;
  ExifParser::decode(decoded, encoded.data(), encoded.size(), DecodeParams(100));
  return decoded;
}

// Preserve the encoded value for comparisons independent of display formatting.
std::vector<byte> bytes(const Exifdatum& datum) {
  std::vector<byte> result(datum.size());
  datum.copy(result.data(), littleEndian);
  return result;
}

// Format one datum with the complete metadata available for model selection.
std::string printed(const ExifData& metadata, const char* key) {
  const auto position = metadata.findKey(ExifKey(key));
  if (position == metadata.end())
    return "<missing>";

  return position->print(&metadata);
}

// Supply the manufacturer required to construct a Panasonic MakerNote.
ExifData panasonic(const char* model = "DMC-FZ5") {
  ExifData metadata;
  metadata["Exif.Image.Make"] = "Panasonic";
  metadata["Exif.Image.Model"] = model;
  return metadata;
}
}  // namespace

// Keep established descriptive keys stable while adding newer definitions.
TEST(PanasonicMetadata, ExistingKeysRemainStable) {
  EXPECT_EQ(0x0001, ExifKey("Exif.Panasonic.Quality").tag());
  EXPECT_EQ(0x000f, ExifKey("Exif.Panasonic.AFMode").tag());
  EXPECT_EQ(0x0051, ExifKey("Exif.Panasonic.LensType").tag());
  EXPECT_EQ(0x0119, ExifKey("Exif.PanasonicRaw.DistortionInfo").tag());
}

// Named and unknown values retain their types and bytes in either TIFF byte order.
TEST(PanasonicMetadata, TiffRoundTripPreservesRawValues) {
  for (const auto order : {littleEndian, bigEndian}) {
    auto metadata = panasonic();
    metadata["Exif.Panasonic.LensType"] = "Existing lens description";
    metadata["Exif.Panasonic.0x7ffe"] = uint16_t{1234};
    auto decoded = roundTrip(metadata, order);

    ASSERT_NE(decoded.end(), decoded.findKey(ExifKey("Exif.Panasonic.0x7ffe")));
    EXPECT_EQ("Existing lens description", printed(decoded, "Exif.Panasonic.LensType"));
    EXPECT_EQ(unsignedShort, decoded["Exif.Panasonic.0x7ffe"].typeId());
    EXPECT_EQ(bytes(metadata["Exif.Panasonic.0x7ffe"]), bytes(decoded["Exif.Panasonic.0x7ffe"]));
  }
}

// Real repository JPEG data continues through the ordinary factory and TIFF reader.
TEST(PanasonicMetadata, ReadsExistingPanasonicJpeg) {
  auto image = ImageFactory::open(std::string(TESTDATA_PATH) + "/exiv2-panasonic-dmc-fz5.jpg");
  image->readMetadata();

  EXPECT_EQ("image/jpeg", image->mimeType());
  EXPECT_EQ("DMC-FZ5", image->exifData()["Exif.Image.Model"].toString());
  EXPECT_EQ("High", printed(image->exifData(), "Exif.Panasonic.Quality"));
}
