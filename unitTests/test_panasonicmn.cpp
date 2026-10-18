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

// Every newly named field resolves to its unique identifier and encoded TIFF type.
TEST(PanasonicMetadata, AddedDefinitionsHaveExpectedTypes) {
  // Reference facts are explicit here so a typo cannot silently change the interface.
  struct Definition {
    uint16_t tag;
    const char* name;
    TypeId type;
  };
  const Definition definitions[] = {
      {0x0027, "VideoFrameRate", unsignedShort},
      {0x0038, "BatteryLevel", unsignedShort},
      {0x0043, "JPEGQuality", unsignedShort},
      {0x0076, "MergedImages", unsignedShort},
      {0x008b, "WBShiftIntelligentAuto", unsignedShort},
      {0x0092, "WBShiftCreativeControl", unsignedByte},
      {0x00a1, "FilterEffect", unsignedRational},
      {0x00a7, "OutputLUT", undefined},
      {0x00ac, "MonochromeFilterEffect", unsignedShort},
      {0x00ad, "HighlightShadow", unsignedShort},
      {0x00af, "TimeStamp", asciiString},
      {0x00b3, "VideoBurstResolution", unsignedShort},
      {0x00b4, "MultiExposure", unsignedShort},
      {0x00b9, "RedEyeRemoval", unsignedShort},
      {0x00bb, "VideoBurstMode", unsignedLong},
      {0x00bc, "DiffractionCorrection", unsignedShort},
      {0x00bd, "FocusBracket", unsignedShort},
      {0x00be, "LongExposureNRUsed", unsignedShort},
      {0x00bf, "PostFocusMerging", unsignedLong},
      {0x00c1, "VideoPreburst", unsignedShort},
      {0x00c4, "LensTypeMake", unsignedShort},
      {0x00c5, "LensTypeModel", unsignedShort},
      {0x00ca, "SensorType", unsignedShort},
      {0x00d1, "ISO", unsignedLong},
      {0x00d2, "MonochromeGrainEffect", unsignedShort},
      {0x00d4, "HybridLogGamma", unsignedShort},
      {0x00d6, "NoiseReductionStrength", signedRational},
      {0x00de, "AFAreaSize", unsignedRational},
      {0x00e4, "LensTypeModel2", unsignedShort},
      {0x00e8, "MinimumISO", unsignedLong},
      {0x00e9, "AFSubjectDetection", unsignedShort},
      {0x00ee, "DynamicRangeBoost", unsignedShort},
      {0x00f1, "LUT1Name", asciiString},
      {0x00f3, "LUT1Opacity", unsignedByte},
      {0x00f4, "LUT2Name", asciiString},
      {0x00f5, "LUT2Opacity", unsignedByte},
      {0x2003, "TimeInfo", undefined},
      {0x8002, "HighlightWarning", unsignedShort},
      {0x8003, "DarkFocusEnvironment", unsignedShort},
  };
  for (const auto& definition : definitions) {
    const ExifKey key(std::string("Exif.Panasonic.") + definition.name);
    EXPECT_EQ(definition.tag, key.tag()) << definition.name;
    EXPECT_EQ(definition.type, key.defaultTypeId()) << definition.name;
  }

  EXPECT_EQ("0x0063", ExifKey(0x0063, "Panasonic").tagName());
}

// Interpretation of signed shifts never changes their unsigned encoded values.
TEST(PanasonicMetadata, SignedDisplaysPreserveEncodedTypes) {
  auto metadata = panasonic();
  metadata["Exif.Panasonic.WBShiftIntelligentAuto"] = uint16_t{65527};
  metadata["Exif.Panasonic.FocusBracket"] = uint16_t{65535};
  auto shift = Value::create(unsignedByte);
  shift->read("255");
  metadata.add(ExifKey("Exif.Panasonic.WBShiftCreativeControl"), shift.get());

  for (const auto order : {littleEndian, bigEndian}) {
    auto decoded = roundTrip(metadata, order);
    EXPECT_EQ("-9", printed(decoded, "Exif.Panasonic.WBShiftIntelligentAuto"));
    EXPECT_EQ("-1", printed(decoded, "Exif.Panasonic.FocusBracket"));
    EXPECT_EQ("-1", printed(decoded, "Exif.Panasonic.WBShiftCreativeControl"));
    EXPECT_EQ(bytes(metadata["Exif.Panasonic.FocusBracket"]), bytes(decoded["Exif.Panasonic.FocusBracket"]));
    EXPECT_EQ(unsignedShort, decoded["Exif.Panasonic.FocusBracket"].typeId());
  }
}

// Unknown enumeration values remain visible rather than being mapped to a known mode.
TEST(PanasonicMetadata, NewEnumsAndSentinelsAreReadable) {
  auto metadata = panasonic();
  metadata["Exif.Panasonic.BatteryLevel"] = uint16_t{7};
  metadata["Exif.Panasonic.AFSubjectDetection"] = uint16_t{13};
  metadata["Exif.Panasonic.DynamicRangeBoost"] = uint16_t{1};
  metadata["Exif.Panasonic.VideoBurstMode"] = uint32_t{0x1001};
  metadata["Exif.Panasonic.ISO"] = uint32_t{0xffffffff};
  EXPECT_EQ("Near full", printed(metadata, "Exif.Panasonic.BatteryLevel"));
  EXPECT_EQ("Airplane (nose priority)", printed(metadata, "Exif.Panasonic.AFSubjectDetection"));
  EXPECT_EQ("On", printed(metadata, "Exif.Panasonic.DynamicRangeBoost"));
  EXPECT_EQ("High resolution mode", printed(metadata, "Exif.Panasonic.VideoBurstMode"));
  EXPECT_EQ("n/a", printed(metadata, "Exif.Panasonic.ISO"));

  metadata["Exif.Panasonic.BatteryLevel"] = uint16_t{999};
  EXPECT_EQ("(999)", printed(metadata, "Exif.Panasonic.BatteryLevel"));
}

// Older descriptions remain stable while newly documented enumeration values become readable.
TEST(PanasonicMetadata, ExtendsExistingEnumsWithoutChangingLegacyValues) {
  auto metadata = panasonic();
  metadata["Exif.Panasonic.WhiteBalance"] = uint16_t{14};
  metadata["Exif.Panasonic.ImageStabilization"] = uint16_t{12};
  metadata["Exif.Panasonic.PhotoStyle"] = uint16_t{17};
  metadata["Exif.Panasonic.TimerRecording"] = uint16_t{3};
  metadata["Exif.Panasonic.BurstMode"] = uint16_t{18};
  EXPECT_EQ("Manual 3", printed(metadata, "Exif.Panasonic.WhiteBalance"));
  EXPECT_EQ("Dual2 IS Panning", printed(metadata, "Exif.Panasonic.ImageStabilization"));
  EXPECT_EQ("V-Log", printed(metadata, "Exif.Panasonic.PhotoStyle"));
  EXPECT_EQ("Focus bracketing", printed(metadata, "Exif.Panasonic.TimerRecording"));
  EXPECT_EQ("Aperture bracketing", printed(metadata, "Exif.Panasonic.BurstMode"));

  metadata["Exif.Panasonic.ImageStabilization"] = uint16_t{2};
  metadata["Exif.Panasonic.BurstMode"] = uint16_t{2};
  EXPECT_EQ("On, Mode 1", printed(metadata, "Exif.Panasonic.ImageStabilization"));
  EXPECT_EQ("Infinite", printed(metadata, "Exif.Panasonic.BurstMode"));
}

// Identical AF bytes have a different documented meaning on the FZ10.
TEST(PanasonicMetadata, AfModeUsesCameraModelAndPreservesAmbiguityWithoutIt) {
  auto metadata = panasonic("DMC-FZ10");
  metadata["Exif.Panasonic.AFMode"].setValue("0 1");
  EXPECT_EQ("Spot mode on", printed(metadata, "Exif.Panasonic.AFMode"));
  metadata["Exif.Panasonic.AFMode"].setValue("0 16");
  EXPECT_EQ("Spot mode off", printed(metadata, "Exif.Panasonic.AFMode"));

  metadata["Exif.Image.Model"] = "DMC-FZ30";
  EXPECT_EQ("3-area (high speed)", printed(metadata, "Exif.Panasonic.AFMode"));
  metadata["Exif.Panasonic.AFMode"].setValue("0 1");
  EXPECT_EQ("9-area", printed(metadata, "Exif.Panasonic.AFMode"));
  metadata.erase(metadata.findKey(ExifKey("Exif.Image.Model")));
  EXPECT_EQ("Spot mode on or 9 area", printed(metadata, "Exif.Panasonic.AFMode"));
}

// Zone layouts and ambiguous tracking are selected only for documented camera models.
TEST(PanasonicMetadata, AfModeKeepsUnverifiedPairsRaw) {
  auto metadata = panasonic("DC-G9M2");
  metadata["Exif.Panasonic.AFMode"].setValue("32 16");
  EXPECT_EQ("Zone", printed(metadata, "Exif.Panasonic.AFMode"));
  metadata["Exif.Image.Model"] = "DC-GH6";
  EXPECT_EQ("32 16", printed(metadata, "Exif.Panasonic.AFMode"));
  metadata["Exif.Panasonic.AFMode"].setValue("32 0");
  metadata["Exif.Image.Model"] = "DMC-FS7";
  EXPECT_EQ("Face Detect", printed(metadata, "Exif.Panasonic.AFMode"));
  metadata["Exif.Image.Model"] = "DMC-FZ200";
  EXPECT_EQ("Face Detect or Tracking", printed(metadata, "Exif.Panasonic.AFMode"));

  metadata["Exif.Panasonic.AFMode"].setValue("32 16 1");
  EXPECT_EQ("32 16 1", printed(metadata, "Exif.Panasonic.AFMode"));
}
