// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>
#include <sonymn_int.hpp>
#include <tiffcomposite_int.hpp>

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <tuple>

using namespace Exiv2;

namespace {

// Describe a documented camera layout independently of the production selector.
struct LensLayout {
  const char* model;
  size_t vignetting;
  size_t aberration;
};

constexpr LensLayout layouts[] = {
    {"ILCA-68", 0x34a, 0x37c},    {"ILCA-77M2", 0x34a, 0x37c},  {"ILCE-5000", 0x34a, 0x37c},
    {"ILCE-5100", 0x34a, 0x37c},  {"ILCE-6000", 0x34a, 0x37c},  {"ILCE-7", 0x34a, 0x37c},
    {"ILCE-7R", 0x34a, 0x37c},    {"ILCE-7S", 0x34a, 0x37c},    {"ILCE-QX1", 0x34a, 0x37c},
    {"Lusso", 0x34a, 0x37c},      {"ILCE-7M2", 0x350, 0x384},   {"ILCE-6300", 0x368, 0x39c},
    {"ILCE-7RM2", 0x368, 0x39c},  {"ILCE-7SM2", 0x368, 0x39c},  {"ILCA-99M2", 0x35c, 0x3b0},
    {"ILCE-6500", 0x35c, 0x3b0},  {"ILCE-6100", 0x35c, 0x3b8},  {"ILCE-6100A", 0x35c, 0x3b8},
    {"ILCE-6400", 0x35c, 0x3b8},  {"ILCE-6400A", 0x35c, 0x3b8}, {"ILCE-6600", 0x35c, 0x3b8},
    {"ILCE-7C", 0x35c, 0x3b8},    {"ILCE-7M3", 0x35c, 0x3b8},   {"ILCE-7RM3", 0x35c, 0x3b8},
    {"ILCE-7RM3A", 0x35c, 0x3b8}, {"ILCE-7RM4", 0x35c, 0x3b8},  {"ILCE-7RM4A", 0x35c, 0x3b8},
    {"ILCE-9", 0x35c, 0x3b8},     {"ILCE-9M2", 0x35c, 0x3b8},   {"ZV-E10", 0x35c, 0x3b8},
};

// Include signed limits and values whose encrypted bytes exercise the full cipher.
int16_t coefficient(size_t array, size_t index) {
  constexpr std::array<int16_t, 8> values{-32768, -249, -1, 0, 1, 248, 255, 32767};
  return values[(array + index) % values.size()];
}

// Construct the encrypted parent with unrelated trailing bytes to preserve.
Blob lensBlock(const LensLayout& layout, size_t size = 2048, byte selector = 0x25) {
  Blob block(2048, 0xa7);
  for (size_t a = 0; a < 3; ++a) {
    const size_t start = a == 0 ? 0x64 : a == 1 ? layout.vignetting : layout.aberration;
    for (size_t i = 0; i < (a == 2 ? 32U : 16U); ++i)
      us2Data(block.data() + start + i * 2, static_cast<uint16_t>(coefficient(a, i)), littleEndian);
  }

  // Encode independently rather than using the production deciphering routine.
  for (auto& value : block)
    value = value < 249 ? static_cast<byte>(value * value * value % 249) : value;
  block[0] = selector;
  block.resize(size, 0x5a);
  return block;
}

// Extract bytes without depending on printable metadata representations.
Blob rawValue(const Exifdatum& datum) {
  Blob bytes(datum.size());
  datum.copy(bytes.data(), littleEndian);
  return bytes;
}

// Decode a complete TIFF through the same public entry point as applications.
ExifData readExif(const Blob& encoded) {
  ExifData data;
  ExifParser::decode(data, encoded.data(), encoded.size(), DecodeParams{100});
  return data;
}

// Serialize a one-pixel TIFF while leaving unknown Sony parents opaque.
Blob nativeFixture(const LensLayout& layout, const std::string& group, ByteOrder order, const Blob& block,
                   TypeId type = undefined, bool duplicate = false) {
  ExifData data;
  data["Exif.Image.Make"] = "SONY";
  if (layout.model)
    data["Exif.Image.Model"] = layout.model;
  data["Exif.Image.ImageWidth"] = uint32_t{1};
  data["Exif.Image.ImageLength"] = uint32_t{1};
  data["Exif.Image.BitsPerSample"] = uint16_t{8};
  data["Exif.Image.Compression"] = uint16_t{1};
  data["Exif.Image.PhotometricInterpretation"] = uint16_t{1};
  data["Exif.Image.SamplesPerPixel"] = uint16_t{1};
  data["Exif.Image.RowsPerStrip"] = uint32_t{1};
  data["Exif.Image.StripOffsets"] = uint32_t{0};
  data["Exif.Image.StripByteCounts"] = uint32_t{1};
  const byte pixel = 127;
  data["Exif.Image.StripOffsets"].setDataArea(&pixel, 1);
  auto value = Value::create(type);
  value->read(block.data(), block.size(), order);
  data.add(ExifKey("Exif." + group + ".0x9405"), value.get());
  if (duplicate)
    data.add(ExifKey("Exif." + group + ".0x9406"), value.get());

  MemIo io;
  TiffParser::encode(io, nullptr, 0, order, data, {}, {});
  io.seek(0, BasicIo::beg);
  const auto bytes = io.read(io.size());
  Blob encoded(bytes.c_data(), bytes.c_data() + bytes.size());
  if (duplicate) {
    // Mutate the second opaque entry into a duplicate without serializer merging.
    const auto decoded = readExif(encoded);
    size_t start = decoded.findKey(ExifKey("Exif.MakerNote.Offset"))->toUint32();
    if (group == "Sony1")
      start += 12;
    const auto count = getUShort(encoded.data() + start, order);
    for (size_t i = 0; i < count; ++i) {
      auto entry = encoded.data() + start + 2 + i * 12;
      if (getUShort(entry, order) == 0x9406)
        us2Data(entry, 0x9405, order);
    }
  }
  return encoded;
}

// Wrap a headerless Sony2 MakerNote at its original, independently ordered offset.
Blob wrappedFixture(const LensLayout& layout, ByteOrder outer, ByteOrder inner, uint32_t origin) {
  const auto block = lensBlock(layout);
  Blob record(6 + 18 + block.size(), 0);
  record[0] = record[1] = inner == littleEndian ? 'I' : 'M';
  ul2Data(record.data() + 2, origin, bigEndian);
  us2Data(record.data() + 6, 1, inner);
  us2Data(record.data() + 8, 0x9405, inner);
  us2Data(record.data() + 10, undefined, inner);
  ul2Data(record.data() + 12, static_cast<uint32_t>(block.size()), inner);
  ul2Data(record.data() + 16, origin + 18, inner);
  std::copy(block.begin(), block.end(), record.begin() + 24);
  Blob privateData{'A', 'd', 'o', 'b', 'e', 0, 'M', 'a', 'k', 'N', 0, 0, 0, 0};
  ul2Data(privateData.data() + 10, static_cast<uint32_t>(record.size()), bigEndian);
  privateData.insert(privateData.end(), record.begin(), record.end());

  ExifData data;
  data["Exif.Image.Make"] = "SONY";
  data["Exif.Image.Model"] = layout.model;
  auto value = Value::create(unsignedByte);
  value->read(privateData.data(), privateData.size(), outer);
  data.add(ExifKey("Exif.Image.DNGPrivateData"), value.get());
  MemIo io;
  TiffParser::encode(io, nullptr, 0, outer, data, {}, {});
  io.seek(0, BasicIo::beg);
  const auto bytes = io.read(io.size());
  return {bytes.c_data(), bytes.c_data() + bytes.size()};
}

// Count derived values without requiring the new group to exist in baseline builds.
size_t lensCount(const ExifData& data) {
  return std::count_if(data.begin(), data.end(), [](const auto& d) { return d.groupName() == "Sony9405b"; });
}

// Compare every coefficient, type, count, and stable logical identifier.
void expectLens(const ExifData& data, size_t copies = 1) {
  ASSERT_EQ(3 * copies, lensCount(data));
  constexpr const char* names[]{"DistortionCorrParams", "VignettingCorrParams", "ChromaticAberrationCorrParams"};
  for (size_t a = 0; a < 3; ++a) {
    size_t found = 0;
    for (const auto& datum : data) {
      if (datum.key() != std::string("Exif.Sony9405b.") + names[a])
        continue;
      ++found;
      EXPECT_EQ(a + 1, datum.tag());
      EXPECT_EQ(signedShort, datum.typeId());
      ASSERT_EQ(a == 2 ? 32U : 16U, datum.count());
      for (size_t i = 0; i < datum.count(); ++i)
        EXPECT_EQ(coefficient(a, i), datum.toInt64(i));
    }
    EXPECT_EQ(copies, found);
  }
}

// Snapshot complete I/O to verify no-op output and failure before source changes.
Blob imageBytes(Image& image) {
  auto& io = image.io();
  if (io.open() != 0)
    throw Error(ErrorCode::kerFailedToReadImageData);
  IoCloser closer(io);
  const auto bytes = io.read(io.size());
  return {bytes.c_data(), bytes.c_data() + bytes.size()};
}

// Cover all documented names independently of group and TIFF byte order.
class Sony9405bNative : public ::testing::TestWithParam<std::tuple<LensLayout, std::string, ByteOrder>> {};

// Read arrays while retaining the exact opaque parent and its existing key.
TEST_P(Sony9405bNative, ReadsNative) {
  const auto [layout, group, order] = GetParam();
  const auto block = lensBlock(layout);
  const auto data = readExif(nativeFixture(layout, group, order, block));
  expectLens(data);
  const auto raw = data.findKey(ExifKey("Exif." + group + ".0x9405"));
  ASSERT_NE(data.end(), raw);
  EXPECT_EQ(block, rawValue(*raw));
}

// Preserve each occurrence rather than selecting or merging duplicate parents.
TEST(Sony9405b, ReadsDuplicateParents) {
  for (const auto& group : {"Sony1", "Sony2"}) {
    const auto data =
        readExif(nativeFixture(layouts[12], group, littleEndian, lensBlock(layouts[12]), undefined, true));
    expectLens(data, 2);
  }
}

// Keep optional unknown or incomplete blocks opaque without losing ordinary Exif.
TEST(Sony9405b, RejectsUnsupportedAndTruncatedViews) {
  for (const auto& layout : {layouts[0], layouts[10], layouts[12], layouts[14], layouts[16]}) {
    for (size_t size : {0U, 1U, 0x64U, 0x83U, 0x369U, 0x39bU, 0x3afU}) {
      const auto data = readExif(nativeFixture(layout, "Sony1", littleEndian, lensBlock(layout, size)));
      EXPECT_EQ(0U, lensCount(data));
      EXPECT_EQ(layout.model, data.findKey(ExifKey("Exif.Image.Model"))->toString());
    }
    expectLens(readExif(nativeFixture(layout, "Sony2", bigEndian, lensBlock(layout, layout.aberration + 64))));
  }
  for (const auto* model : {"", "DSC-HX60V", "ILCE-7RM5", "ILCE-7M20", "ILCE-63000", "ILCE-7RM2unknown"}) {
    const LensLayout unknown{model, 0x368, 0x39c};
    EXPECT_EQ(0U, lensCount(readExif(nativeFixture(unknown, "Sony2", littleEndian, lensBlock(unknown)))));
  }
  for (auto selector : {0U, 0x1bU, 0xffU})
    EXPECT_EQ(0U, lensCount(readExif(nativeFixture(layouts[12], "Sony2", littleEndian,
                                                   lensBlock(layouts[12], 2048, static_cast<byte>(selector))))));
  for (auto type : {asciiString, unsignedShort, unsignedLong})
    EXPECT_EQ(0U, lensCount(readExif(nativeFixture(layouts[12], "Sony2", littleEndian, lensBlock(layouts[12]), type))));
}

// Recognize every documented encrypted discriminator and accept byte parents.
TEST(Sony9405b, RecognizesSelectorsAndByteParents) {
  for (auto selector : {0x3a, 0xb3, 0x7e, 0x9a, 0x25, 0xe1, 0x76, 0x8b})
    expectLens(readExif(nativeFixture(layouts[12], "Sony2", bigEndian,
                                      lensBlock(layouts[12], 8192, static_cast<byte>(selector)), unsignedByte)));
}

// Missing model context cannot select a lens layout from the discriminator alone.
TEST(Sony9405b, KeepsMissingModelOpaque) {
  const LensLayout missing{nullptr, 0x368, 0x39c};
  EXPECT_EQ(0U, lensCount(readExif(nativeFixture(missing, "Sony2", littleEndian, lensBlock(missing)))));
}

// Named fields retain signed defaults when callers reconstruct values from strings.
TEST(Sony9405b, RegistersSignedDefaults) {
  ExifData data;
  data["Exif.Sony9405b.DistortionCorrParams"] = "-32768 0 32767";
  const auto& datum = data["Exif.Sony9405b.DistortionCorrParams"];
  EXPECT_EQ(signedShort, datum.typeId());
  EXPECT_EQ(16, ExifTags::defaultCount(ExifKey(datum.key())));
  EXPECT_EQ(-32768, datum.toInt64(0));
  EXPECT_EQ(32767, datum.toInt64(2));
  EXPECT_TRUE(ExifTags::isMakerGroup("Sony9405b"));
}

// Resolve inner offsets without treating outer byte order as coefficient order.
TEST(Sony9405b, ReadsWrappedLayouts) {
  for (const auto& layout : {layouts[0], layouts[10], layouts[12], layouts[14], layouts[16]}) {
    for (auto outer : {littleEndian, bigEndian}) {
      for (auto inner : {littleEndian, bigEndian}) {
        for (auto origin : {0U, 1006U, 0xffff0000U})
          expectLens(readExif(wrappedFixture(layout, outer, inner, origin)));
      }
    }
  }
}

// Preserve encrypted bytes through no-op, intrusive, and repeated native edits.
TEST(Sony9405b, PreservesParentDuringWrites) {
  for (const auto& group : {"Sony1", "Sony2"}) {
    const auto block = lensBlock(layouts[12]);
    const auto encoded = nativeFixture(layouts[12], group, littleEndian, block);
    auto image = ImageFactory::open(encoded.data(), encoded.size());
    image->readMetadata();
    ASSERT_EQ(3U, lensCount(image->exifData()));
    ASSERT_NO_THROW(image->writeMetadata());
    EXPECT_EQ(encoded, imageBytes(*image));

    for (const auto& artist : {std::string("artist"), std::string(600, 'a')}) {
      image->exifData()["Exif.Image.Artist"] = artist;
      ASSERT_NO_THROW(image->writeMetadata());
      image->readMetadata();
      expectLens(image->exifData());
      EXPECT_EQ(block, rawValue(image->exifData()["Exif." + std::string(group) + ".0x9405"]));
      EXPECT_EQ(artist, image->exifData()["Exif.Image.Artist"].toString());
      const auto bytes = imageBytes(*image);
      const auto offset = image->exifData()["Exif.Image.StripOffsets"].toUint32();
      ASSERT_LT(offset, bytes.size());
      EXPECT_EQ(127, bytes[offset]);
    }
  }
}

// Reject changes, deletion, duplicate additions, and orphaned synthesized entries.
TEST(Sony9405b, RejectsDerivedEditsBeforeWriting) {
  const auto encoded = nativeFixture(layouts[12], "Sony2", littleEndian, lensBlock(layouts[12]));
  auto image = ImageFactory::open(encoded.data(), encoded.size());
  image->readMetadata();
  ASSERT_EQ(3U, lensCount(image->exifData()));
  for (int mutation = 0; mutation < 6; ++mutation) {
    image->readMetadata();
    auto& data = image->exifData();
    auto pos = data.findKey(ExifKey("Exif.Sony9405b.DistortionCorrParams"));
    ASSERT_NE(data.end(), pos);
    if (mutation == 0)
      pos->setValue("1 2 3");
    else if (mutation == 1)
      data.erase(pos);
    else if (mutation == 2)
      data.add(*pos);
    else if (mutation == 3)
      data["Exif.Sony9405b.0x0042"] = "unsupported";
    else if (mutation == 4)
      data.erase(std::remove_if(data.begin(), data.end(), [](const auto& d) { return d.groupName() == "Sony9405b"; }),
                 data.end());
    else
      data.erase(data.findKey(ExifKey("Exif.Sony2.0x9405")));
    EXPECT_THROW(image->writeMetadata(), Error);
    EXPECT_EQ(encoded, imageBytes(*image));
  }
}

// Copying and self-copy keep views consistent; complete Exif removal remains valid.
TEST(Sony9405b, CopiesAndClearsMetadata) {
  const auto encoded = nativeFixture(layouts[12], "Sony1", bigEndian, lensBlock(layouts[12]));
  auto source = ImageFactory::open(encoded.data(), encoded.size());
  auto destination = ImageFactory::open(encoded.data(), encoded.size());
  source->readMetadata();
  ASSERT_EQ(3U, lensCount(source->exifData()));
  destination->readMetadata();
  destination->setExifData(source->exifData());
  ASSERT_NO_THROW(destination->writeMetadata());
  destination->setExifData(destination->exifData());
  ASSERT_NO_THROW(destination->writeMetadata());
#ifdef EXV_HAVE_XMP_TOOLKIT
  // The general copying API also processes XMP, even when these fixtures have none.
  destination->setMetadata(*source);
  ASSERT_NO_THROW(destination->writeMetadata());
  destination->setMetadata(*destination);
  ASSERT_NO_THROW(destination->writeMetadata());
#endif
  destination->readMetadata();
  expectLens(destination->exifData());
  destination->clearExifData();
  ASSERT_NO_THROW(destination->writeMetadata());
  destination->readMetadata();
  EXPECT_EQ(0U, lensCount(destination->exifData()));
}

// Build original camera identification without relying on decoder-added metadata views.
std::unique_ptr<Internal::TiffDirectory> cameraTree(const char* make) {
  auto source = std::make_unique<Internal::TiffDirectory>(0, IfdId::ifd0Id, false);
  if (*make) {
    auto entry = std::make_shared<Internal::TiffEntry>(0x010f, IfdId::ifd0Id);
    auto value = Value::create(asciiString);
    value->read(make);
    entry->setValue(std::move(value));
    source->addChild(std::move(entry));
  }
  return source;
}

// Ordinary brands and opaque non-Sony data must bypass Sony-specific preparation.
TEST(Sony9405b, SkipsPreparationForOtherBrands) {
  for (const auto* make : {"", "Canon", "NIKON CORPORATION", "Panasonic", "FUJIFILM"}) {
    ExifData data;
    if (*make)
      data["Exif.Image.Make"] = make;
    data["Exif.Image.Artist"] = "other brand";
    data["Exif.Image.0x9405"] = uint16_t{1};
    data["Exif.Photo.MakerNote"] = "opaque";
    data["Exif.Image.DNGPrivateData"] = "Adobe";
    EXPECT_FALSE(Internal::hasSonyMetadata(data, nullptr));

    // Source detection also leaves another brand's Make and payloads alone.
    auto source = cameraTree(make);
    EXPECT_FALSE(Internal::hasSonyMetadata(data, source.get()));
    EXPECT_FALSE(Internal::hasSonyMetadata({}, source.get()));
  }
}

// Imported groups and orphan views need validation independently of camera Make.
TEST(Sony9405b, DetectsPreparationForIncomingSonyMetadata) {
  ExifData data;
  data["Exif.Image.Make"] = "SONY CORPORATION";
  EXPECT_TRUE(Internal::hasSonyMetadata(data, nullptr));
  for (const auto* key : {"Exif.Sony1.0x9405", "Exif.Sony2.0x9405", "Exif.Sony9405b.DistortionCorrParams"}) {
    data.clear();
    data[key] = "1";
    EXPECT_TRUE(Internal::hasSonyMetadata(data, nullptr));
    data["Exif.Image.Make"] = "Canon";
    EXPECT_TRUE(Internal::hasSonyMetadata(data, nullptr));
  }
}

// Changing or deleting all current Sony identification must retain source validation.
TEST(Sony9405b, DetectsPreparationFromOriginalSonyMake) {
  for (const auto* make : {"SONY", "SONY CORPORATION"}) {
    auto source = cameraTree(make);
    ExifData data;
    EXPECT_TRUE(Internal::hasSonyMetadata(data, source.get()));
    data["Exif.Image.Make"] = "Canon";
    EXPECT_TRUE(Internal::hasSonyMetadata(data, source.get()));
  }
}

// Removing a native parent and its view is valid; removing only the view is not.
TEST(Sony9405b, RemovesNativeParentWithView) {
  const auto encoded = nativeFixture(layouts[12], "Sony2", littleEndian, lensBlock(layouts[12]));
  auto image = ImageFactory::open(encoded.data(), encoded.size());
  image->readMetadata();
  auto& data = image->exifData();
  data.erase(
      std::remove_if(data.begin(), data.end(),
                     [](const auto& d) { return d.groupName() == "Sony9405b" || d.key() == "Exif.Sony2.0x9405"; }),
      data.end());
  ASSERT_NO_THROW(image->writeMetadata());
  image->readMetadata();
  EXPECT_EQ(0U, lensCount(image->exifData()));
  EXPECT_EQ(image->exifData().end(), image->exifData().findKey(ExifKey("Exif.Sony2.0x9405")));
}

// Wrapped coefficients obey the existing immutable Adobe view and opaque ownership.
TEST(Sony9405b, PreservesWrappedParentAndRejectsEdits) {
  const auto encoded = wrappedFixture(layouts[12], bigEndian, littleEndian, 0xffff0000);
  auto image = ImageFactory::open(encoded.data(), encoded.size());
  image->readMetadata();
  expectLens(image->exifData());
  const auto parent = rawValue(image->exifData()["Exif.Image.DNGPrivateData"]);
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_EQ(encoded, imageBytes(*image));
  image->exifData()["Exif.Image.Artist"] = std::string(400, 'a');
  ASSERT_NO_THROW(image->writeMetadata());
  image->readMetadata();
  expectLens(image->exifData());
  EXPECT_EQ(parent, rawValue(image->exifData()["Exif.Image.DNGPrivateData"]));
  const auto before = imageBytes(*image);
  image->exifData()["Exif.Sony9405b.VignettingCorrParams"] = "1 2 3";
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(before, imageBytes(*image));
}

// A competing standard correction array must retain its own values and identity.
TEST(Sony9405b, KeepsSeparateCorrectionCopies) {
  const auto encoded = nativeFixture(layouts[12], "Sony2", littleEndian, lensBlock(layouts[12]));
  auto image = ImageFactory::open(encoded.data(), encoded.size());
  image->readMetadata();
  image->exifData()["Exif.Image.DistortionCorrParams"] = "17 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16";
  ASSERT_NO_THROW(image->writeMetadata());
  image->readMetadata();
  expectLens(image->exifData());
  EXPECT_EQ("17 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16",
            image->exifData()["Exif.Image.DistortionCorrParams"].toString());
}

// Model edits and deletion keep opaque bytes while allowing their interpretation to change.
TEST(Sony9405b, PreservesParentsWhileChangingModel) {
  const auto block = lensBlock(layouts[12]);
  for (const auto& group : {"Sony1", "Sony2"}) {
    for (const auto* model : {"", "Unknown", "ILCA-68", "ILCE-7M3"}) {
      const auto encoded = nativeFixture(layouts[12], group, littleEndian, block);
      auto image = ImageFactory::open(encoded.data(), encoded.size());
      image->readMetadata();
      if (*model)
        image->exifData()["Exif.Image.Model"] = model;
      else
        image->exifData().erase(image->exifData().findKey(ExifKey("Exif.Image.Model")));
      ASSERT_NO_THROW(image->writeMetadata());
      image->readMetadata();
      EXPECT_EQ(block, rawValue(image->exifData()["Exif." + std::string(group) + ".0x9405"]));
      EXPECT_EQ(*model && std::string(model) != "Unknown" ? 3U : 0U, lensCount(image->exifData()));
      ASSERT_NO_THROW(image->writeMetadata());
    }
  }
}

// Copied metadata selects its incoming model rather than the destination's old layout.
TEST(Sony9405b, CopiesAcrossModelLayouts) {
  const auto incoming = nativeFixture(layouts[12], "Sony1", littleEndian, lensBlock(layouts[12]));
  const auto previous = nativeFixture(layouts[0], "Sony1", littleEndian, lensBlock(layouts[0]));
  auto source = ImageFactory::open(incoming.data(), incoming.size());
  auto destination = ImageFactory::open(previous.data(), previous.size());
  source->readMetadata();
  destination->readMetadata();
  destination->setExifData(source->exifData());
  ASSERT_NO_THROW(destination->writeMetadata());
  destination->readMetadata();
  expectLens(destination->exifData());
  EXPECT_EQ("ILCE-7RM2", destination->exifData()["Exif.Image.Model"].toString());
  EXPECT_EQ(lensBlock(layouts[12]), rawValue(destination->exifData()["Exif.Sony1.0x9405"]));
}

// Duplicate parents and views remain consistent during metadata rewriting.
TEST(Sony9405b, PreservesDuplicateParentsDuringWrites) {
  const auto block = lensBlock(layouts[12]);
  const auto encoded = nativeFixture(layouts[12], "Sony2", littleEndian, block, undefined, true);
  auto image = ImageFactory::open(encoded.data(), encoded.size());
  image->readMetadata();
  expectLens(image->exifData(), 2);
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_EQ(encoded, imageBytes(*image));
  image->exifData()["Exif.Image.Artist"] = std::string(400, 'd');
  ASSERT_NO_THROW(image->writeMetadata());
  image->readMetadata();
  expectLens(image->exifData(), 2);
  for (const auto& datum : image->exifData()) {
    if (datum.key() == "Exif.Sony2.0x9405") {
      EXPECT_EQ(block, rawValue(datum));
    }
  }
}

INSTANTIATE_TEST_SUITE_P(ModelsGroupsAndOrders, Sony9405bNative,
                         ::testing::Combine(::testing::ValuesIn(layouts),
                                            ::testing::Values(std::string("Sony1"), std::string("Sony2")),
                                            ::testing::Values(littleEndian, bigEndian)));

}  // namespace
