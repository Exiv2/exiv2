// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>

#include <gtest/gtest.h>

#include <algorithm>
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
// Install opaque parent bytes without manufacturing independently writable child fields.
void setBlock(ExifData& metadata, const char* key, const std::vector<byte>& data) {
  const DataValue value(data.data(), data.size());
  metadata[key].setValue(&value);
}

// Fill one complete rectangle using the file's byte order.
void rectangle(std::vector<byte>& data, size_t offset, ByteOrder order) {
  for (size_t i = 0; i < 4; ++i)
    us2Data(data.data() + offset + 2 * i, static_cast<uint16_t>(10 * (i + 1)), order);
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

// Decoding each parent view preserves byte order, rectangle coordinates and complete raw bytes.
TEST(PanasonicMetadata, StructuredFaceBlocksPreserveTheirParents) {
  for (const auto order : {littleEndian, bigEndian}) {
    auto metadata = panasonic();
    std::vector<byte> positions(42, 0);
    us2Data(positions.data(), 1, order);
    rectangle(positions, 2, order);
    setBlock(metadata, "Exif.Panasonic.FaceDetInfo", positions);

    std::vector<byte> recognized(148, 0);
    us2Data(recognized.data(), 1, order);
    std::copy_n("Alice", 5, recognized.begin() + 4);
    rectangle(recognized, 24, order);
    std::copy_n("0020:01:02 00:00:00", 19, recognized.begin() + 32);
    setBlock(metadata, "Exif.Panasonic.FaceRecInfo", recognized);
    auto decoded = roundTrip(metadata, order);

    EXPECT_EQ("Stored face positions: 1; Face 1: center (10, 20), size 30 x 40",
              printed(decoded, "Exif.Panasonic.FaceDetInfo"));
    EXPECT_EQ("Faces recognized: 1; Face 1: Alice, center (10, 20), size 30 x 40, age 0020:01:02 00:00:00",
              printed(decoded, "Exif.Panasonic.FaceRecInfo"));
    EXPECT_EQ(positions, bytes(decoded["Exif.Panasonic.FaceDetInfo"]));
    EXPECT_EQ(recognized, bytes(decoded["Exif.Panasonic.FaceRecInfo"]));

    decoded["Exif.Image.Artist"] = "Unrelated edit forcing growth";
    auto reopened = roundTrip(decoded, order);
    EXPECT_EQ(positions, bytes(reopened["Exif.Panasonic.FaceDetInfo"]));
    EXPECT_EQ(recognized, bytes(reopened["Exif.Panasonic.FaceRecInfo"]));
  }
}

// Invalid record counts, lengths and missing byte-order state retain the original raw view.
TEST(PanasonicMetadata, MalformedFaceBlocksRemainRaw) {
  auto metadata = panasonic();
  metadata["Exif.MakerNote.ByteOrder"] = "II";
  for (const auto length : {size_t{0}, size_t{1}, size_t{2}, size_t{41}}) {
    std::vector<byte> data(length, 0);
    if (length >= 2)
      us2Data(data.data(), 5, littleEndian);
    setBlock(metadata, "Exif.Panasonic.FaceDetInfo", data);
    EXPECT_EQ(metadata["Exif.Panasonic.FaceDetInfo"].toString(), printed(metadata, "Exif.Panasonic.FaceDetInfo"));
  }
  std::vector<byte> data(148, 0);
  us2Data(data.data(), 4, littleEndian);
  setBlock(metadata, "Exif.Panasonic.FaceRecInfo", data);
  EXPECT_EQ(metadata["Exif.Panasonic.FaceRecInfo"].toString(), printed(metadata, "Exif.Panasonic.FaceRecInfo"));

  us2Data(data.data(), 1, littleEndian);
  setBlock(metadata, "Exif.Panasonic.FaceRecInfo", data);
  metadata.erase(metadata.findKey(ExifKey("Exif.MakerNote.ByteOrder")));
  EXPECT_EQ(metadata["Exif.Panasonic.FaceRecInfo"].toString(), printed(metadata, "Exif.Panasonic.FaceRecInfo"));
}

// The packed-decimal date and endian-dependent shot counter have distinct representations.
TEST(PanasonicMetadata, TimeInfoPreservesPackedDateAndOpaqueGap) {
  for (const auto order : {littleEndian, bigEndian}) {
    auto metadata = panasonic();
    std::vector<byte> data{0x20, 0x24, 0x02, 0x29, 0x12, 0x34, 0x56, 0x78, 0xde, 0xad,
                           0xbe, 0xef, 1,    2,    3,    4,    0,    0,    0,    0};
    ul2Data(data.data() + 16, 123456, order);
    setBlock(metadata, "Exif.Panasonic.TimeInfo", data);
    auto decoded = roundTrip(metadata, order);
    EXPECT_EQ("Date/time: 2024:02:29 12:34:56.78; Time-lapse shot: 123456",
              printed(decoded, "Exif.Panasonic.TimeInfo"));
    EXPECT_EQ(data, bytes(decoded["Exif.Panasonic.TimeInfo"]));

    decoded["Exif.Image.Artist"] = "Another unrelated edit";
    auto reopened = roundTrip(decoded, order);
    EXPECT_EQ(data, bytes(reopened["Exif.Panasonic.TimeInfo"]));
  }
}

// Invalid BCD/calendar fields and truncated dates never become plausible timestamps.
TEST(PanasonicMetadata, MalformedTimeInfoRemainsRaw) {
  auto metadata = panasonic();
  metadata["Exif.MakerNote.ByteOrder"] = "II";
  for (const auto& data :
       {std::vector<byte>{0x20, 0x23, 0x02, 0x29, 0, 0, 0, 0}, std::vector<byte>{0x20, 0x24, 0x1a, 0x01, 0, 0, 0, 0},
        std::vector<byte>{0x20, 0x24, 0x01}}) {
    setBlock(metadata, "Exif.Panasonic.TimeInfo", data);
    EXPECT_EQ(metadata["Exif.Panasonic.TimeInfo"].toString(), printed(metadata, "Exif.Panasonic.TimeInfo"));
  }
}

// Packed settings are interpreted without changing types or dividing by zero.
TEST(PanasonicMetadata, PackedSettingsRetainRawRepresentation) {
  auto metadata = panasonic();
  metadata["Exif.Panasonic.FilterEffect"] = URational{0, 0};
  EXPECT_EQ("Off", printed(metadata, "Exif.Panasonic.FilterEffect"));
  metadata["Exif.Panasonic.FilterEffect"] = URational{0, 1024};
  EXPECT_EQ("Dynamic monochrome", printed(metadata, "Exif.Panasonic.FilterEffect"));
  metadata["Exif.Panasonic.FilterEffect"] = URational{1, 1024};
  EXPECT_EQ("1/1024", printed(metadata, "Exif.Panasonic.FilterEffect"));
  metadata["Exif.Panasonic.HighlightShadow"].setValue("65535 2");
  EXPECT_EQ("Highlight: -1; Shadow: 2", printed(metadata, "Exif.Panasonic.HighlightShadow"));
  metadata["Exif.Panasonic.PostFocusMerging"].setValue("0 0");
  EXPECT_EQ("Post focus auto merging or none", printed(metadata, "Exif.Panasonic.PostFocusMerging"));

  auto decoded = roundTrip(metadata);
  EXPECT_EQ(unsignedRational, decoded["Exif.Panasonic.FilterEffect"].typeId());
  EXPECT_EQ(bytes(metadata["Exif.Panasonic.FilterEffect"]), bytes(decoded["Exif.Panasonic.FilterEffect"]));
  EXPECT_EQ(bytes(metadata["Exif.Panasonic.HighlightShadow"]), bytes(decoded["Exif.Panasonic.HighlightShadow"]));
}

// Maximum supported records and nonterminated strings stay inside their fixed fields.
TEST(PanasonicMetadata, FaceRecordsRespectMaximumCountsAndStringBoundaries) {
  for (const auto order : {littleEndian, bigEndian}) {
    auto metadata = panasonic();
    std::vector<byte> positions(42, 0);
    us2Data(positions.data(), 5, order);
    for (size_t i = 0; i < 5; ++i)
      rectangle(positions, 2 + 8 * i, order);
    setBlock(metadata, "Exif.Panasonic.FaceDetInfo", positions);

    std::vector<byte> recognized(148, 'A');
    us2Data(recognized.data(), 3, order);
    for (size_t i = 0; i < 3; ++i)
      rectangle(recognized, 24 + 48 * i, order);
    setBlock(metadata, "Exif.Panasonic.FaceRecInfo", recognized);
    auto decoded = roundTrip(metadata, order);

    EXPECT_NE(std::string::npos, printed(decoded, "Exif.Panasonic.FaceDetInfo").find("Face 5:"));
    EXPECT_NE(std::string::npos,
              printed(decoded, "Exif.Panasonic.FaceRecInfo").find("Face 3: " + std::string(20, 'A')));
    EXPECT_EQ(positions, bytes(decoded["Exif.Panasonic.FaceDetInfo"]));
    EXPECT_EQ(recognized, bytes(decoded["Exif.Panasonic.FaceRecInfo"]));
  }
}

// Every truncated recognized-face record and excessive count falls back before emitting partial text.
TEST(PanasonicMetadata, TruncatedAndOversizedFaceCountsStayRaw) {
  auto metadata = panasonic();
  metadata["Exif.MakerNote.ByteOrder"] = "II";
  for (size_t length = 0; length < 52; ++length) {
    std::vector<byte> data(length, 0);
    if (length >= 2)
      us2Data(data.data(), 1, littleEndian);
    setBlock(metadata, "Exif.Panasonic.FaceRecInfo", data);
    EXPECT_EQ(metadata["Exif.Panasonic.FaceRecInfo"].toString(), printed(metadata, "Exif.Panasonic.FaceRecInfo"));
  }

  std::vector<byte> data(42, 0);
  us2Data(data.data(), 65535, littleEndian);
  setBlock(metadata, "Exif.Panasonic.FaceDetInfo", data);
  EXPECT_EQ(metadata["Exif.Panasonic.FaceDetInfo"].toString(), printed(metadata, "Exif.Panasonic.FaceDetInfo"));
}

// Date-only and unset records do not fabricate a counter, time zone or standard Exif date.
TEST(PanasonicMetadata, OptionalTimeFieldsAndInvalidByteOrderRemainExplicit) {
  auto metadata = panasonic();
  metadata["Exif.MakerNote.ByteOrder"] = "II";
  const std::vector<byte> date{0x20, 0x26, 0x10, 0x18, 0x12, 0x34, 0x56, 0};
  setBlock(metadata, "Exif.Panasonic.TimeInfo", date);
  EXPECT_EQ("Date/time: 2026:10:18 12:34:56.00", printed(metadata, "Exif.Panasonic.TimeInfo"));

  setBlock(metadata, "Exif.Panasonic.TimeInfo", std::vector<byte>(20, 0));
  EXPECT_EQ("Date/time: not set; Time-lapse shot: 0", printed(metadata, "Exif.Panasonic.TimeInfo"));
  EXPECT_EQ(metadata.end(), metadata.findKey(ExifKey("Exif.Photo.DateTimeOriginal")));
  metadata["Exif.MakerNote.ByteOrder"] = "unknown";
  EXPECT_EQ(metadata["Exif.Panasonic.TimeInfo"].toString(), printed(metadata, "Exif.Panasonic.TimeInfo"));
}

// A raw block of the wrong TIFF type and malformed signed settings are never reinterpreted.
TEST(PanasonicMetadata, UnexpectedTypesAndCountsRemainRaw) {
  auto metadata = panasonic();
  metadata["Exif.MakerNote.ByteOrder"] = "II";
  for (const auto* key : {"Exif.Panasonic.FaceDetInfo", "Exif.Panasonic.FaceRecInfo", "Exif.Panasonic.TimeInfo",
                          "Exif.Panasonic.HighlightShadow", "Exif.Panasonic.FilterEffect",
                          "Exif.Panasonic.PostFocusMerging", "Exif.Panasonic.WBShiftIntelligentAuto",
                          "Exif.Panasonic.WBShiftCreativeControl", "Exif.Panasonic.FocusBracket"}) {
    const AsciiValue value("unexpected");
    metadata[key].setValue(&value);
    EXPECT_EQ("unexpected", printed(metadata, key)) << key;
  }

  metadata["Exif.Panasonic.HighlightShadow"].setValue("1");
  EXPECT_EQ("1", printed(metadata, "Exif.Panasonic.HighlightShadow"));
}

// Even a large unknown trailer remains in the parent after a separate metadata edit.
TEST(PanasonicMetadata, StructuredDisplayDoesNotDiscardUninterpretedBytes) {
  auto metadata = panasonic();
  std::vector<byte> data(2048, 0xa5);
  us2Data(data.data(), 1, littleEndian);
  rectangle(data, 2, littleEndian);
  setBlock(metadata, "Exif.Panasonic.FaceDetInfo", data);
  auto decoded = roundTrip(metadata);
  EXPECT_NE(std::string::npos, printed(decoded, "Exif.Panasonic.FaceDetInfo").find("Stored face positions: 1"));

  decoded["Exif.Image.Artist"] = "An unrelated metadata edit";
  auto reopened = roundTrip(decoded);
  EXPECT_EQ(data, bytes(reopened["Exif.Panasonic.FaceDetInfo"]));
}
