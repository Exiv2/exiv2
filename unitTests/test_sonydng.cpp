// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>

#include <array>
#include <tuple>

using namespace Exiv2;

namespace {

// Append a fixed-width integer using the specified container byte order.
void appendShort(Blob& data, uint16_t value, ByteOrder order) {
  const auto offset = data.size();
  data.resize(offset + 2);
  us2Data(data.data() + offset, value, order);
}

// Append a file-relative offset or a record length without host-endian assumptions.
void appendLong(Blob& data, uint32_t value, ByteOrder order) {
  const auto offset = data.size();
  data.resize(offset + 4);
  ul2Data(data.data() + offset, value, order);
}

// Frame an Adobe private-data record, including its even-byte padding.
void appendRecord(Blob& data, const std::array<byte, 4>& name, const Blob& payload) {
  data.insert(data.end(), name.begin(), name.end());
  appendLong(data, static_cast<uint32_t>(payload.size()), bigEndian);
  data.insert(data.end(), payload.begin(), payload.end());
  if (payload.size() % 2)
    data.push_back(0);
}

// Build a Sony2 IFD with an inline correction flag and an encrypted byte array.
Blob sonyDirectory(ByteOrder order, uint32_t originalOffset) {
  Blob data;
  appendShort(data, 2, order);
  appendShort(data, 0x2011, order);
  appendShort(data, unsignedLong, order);
  appendLong(data, 1, order);
  appendLong(data, 0, order);
  appendShort(data, 0x9400, order);
  appendShort(data, undefined, order);
  appendLong(data, 86, order);
  appendLong(data, originalOffset + 30, order);
  appendLong(data, 0, order);

  // These are decrypted values; selector 215 encodes to supported selector 0x26.
  Blob array(86, 0);
  array[0] = 215;
  array[84] = 73;
  array[85] = 255;
  us2Data(array.data() + 71, 5304, littleEndian);
  for (auto value : array)
    data.push_back(value < 249 ? static_cast<byte>(value * value * value % 249) : value);
  return data;
}

// Preserve unknown and SR2 records around a relocated headerless Sony MakerNote.
Blob adobePrivateData(ByteOrder order, uint32_t originalOffset) {
  Blob data{'A', 'd', 'o', 'b', 'e', 0};
  appendRecord(data, {'T', 'e', 's', 't'}, {'o', 'd', 'd'});

  Blob makerNote(2, order == littleEndian ? 'I' : 'M');
  appendLong(makerNote, originalOffset, bigEndian);
  const auto directory = sonyDirectory(order, originalOffset);
  makerNote.insert(makerNote.end(), directory.begin(), directory.end());
  appendRecord(data, {'M', 'a', 'k', 'N'}, makerNote);
  appendRecord(data, {'S', 'R', '2', ' '}, {'k', 'e', 'e', 'p'});
  return data;
}

// Build a one-pixel TIFF with an opaque parent that the reader must discover.
Blob dngMetadata(ByteOrder outerOrder, const Blob& privateData, const Blob& native = {}) {
  ExifData data;
  data["Exif.Image.Make"] = "SONY";
  data["Exif.Image.Model"] = "ILCE-7RM2";
  auto value = Value::create(unsignedByte);
  value->read(privateData.data(), privateData.size(), outerOrder);
  data.add(ExifKey("Exif.Image.DNGPrivateData"), value.get());
  if (!native.empty()) {
    auto makerNote = Value::create(undefined);
    makerNote->read(native.data(), native.size(), outerOrder);
    data.add(ExifKey("Exif.Photo.MakerNote"), makerNote.get());
  }
  // Required image fields keep category-clearing tests readable after rewriting.
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

  MemIo io;
  TiffParser::encode(io, nullptr, 0, outerOrder, data, {}, {});
  io.seek(0, BasicIo::beg);
  const auto encoded = io.read(io.size());
  return {encoded.c_data(), encoded.c_data() + encoded.size()};
}

// Extract a raw value without relying on the tag's interpreted text.
Blob rawValue(const Exifdatum& datum) {
  Blob bytes(datum.size());
  datum.copy(bytes.data(), littleEndian);
  return bytes;
}

// Snapshot complete source bytes to prove that failed writes do not modify I/O.
Blob imageBytes(Image& image) {
  auto& io = image.io();
  if (io.open() != 0)
    throw Error(ErrorCode::kerFailedToReadImageData);
  IoCloser closer(io);
  const auto data = io.read(io.size());
  return {data.c_data(), data.c_data() + data.size()};
}

// Exercise independent outer/inner byte orders and saved original file positions.
class SonyDngMakerNotes : public ::testing::TestWithParam<std::tuple<ByteOrder, ByteOrder, uint32_t>> {};

// Resolve relocated array values and decrypt them within their MakN record.
TEST_P(SonyDngMakerNotes, ReadsWrappedMetadata) {
  const auto [outerOrder, innerOrder, originalOffset] = GetParam();
  const auto privateData = adobePrivateData(innerOrder, originalOffset);
  const auto encoded = dngMetadata(outerOrder, privateData);
  ExifData decoded;
  ASSERT_EQ(outerOrder, ExifParser::decode(decoded, encoded.data(), encoded.size(), DecodeParams{100}));

  const auto flag = decoded.findKey(ExifKey("Exif.Sony2.VignettingCorrection"));
  ASSERT_NE(decoded.end(), flag);
  EXPECT_EQ(0, flag->toInt64());
  const auto height = decoded.findKey(ExifKey("Exif.SonyMisc3c.SonyImageHeight"));
  ASSERT_NE(decoded.end(), height);
  EXPECT_EQ(5304, height->toInt64());
  EXPECT_EQ(73, decoded["Exif.SonyMisc3c.0x0054"].toInt64());
  EXPECT_EQ(255, decoded["Exif.SonyMisc3c.0x0055"].toInt64());
  EXPECT_EQ(privateData, rawValue(decoded["Exif.Image.DNGPrivateData"]));
  EXPECT_EQ(decoded.end(), decoded.findKey(ExifKey("Exif.Photo.MakerNote")));
}

// Ordinary and growing Exif edits must retain the whole opaque block and view.
TEST_P(SonyDngMakerNotes, PreservesPrivateDataWhileEditing) {
  const auto [outerOrder, innerOrder, originalOffset] = GetParam();
  const auto privateData = adobePrivateData(innerOrder, originalOffset);
  const auto encoded = dngMetadata(outerOrder, privateData);
  auto image = ImageFactory::open(encoded.data(), encoded.size());
  image->readMetadata();
  ASSERT_NE(image->exifData().end(), image->exifData().findKey(ExifKey("Exif.Sony2.VignettingCorrection")));

  // An unchanged write must not manufacture a competing native MakerNote.
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_EQ(encoded, imageBytes(*image));

  for (const auto& artist : {std::string("artist"), std::string(300, 'a')}) {
    image->exifData()["Exif.Image.Artist"] = artist;
    ASSERT_NO_THROW(image->writeMetadata());
    image->readMetadata();

    EXPECT_EQ(artist, image->exifData()["Exif.Image.Artist"].toString());
    EXPECT_EQ(privateData, rawValue(image->exifData()["Exif.Image.DNGPrivateData"]));
    EXPECT_EQ(5304, image->exifData()["Exif.SonyMisc3c.SonyImageHeight"].toInt64());
    EXPECT_EQ(image->exifData().end(), image->exifData().findKey(ExifKey("Exif.Photo.MakerNote")));
  }
}

// Reject read-only view changes before either in-place updates or source transfer.
TEST_P(SonyDngMakerNotes, RejectsWrappedFieldEdits) {
  const auto [outerOrder, innerOrder, originalOffset] = GetParam();
  const auto encoded = dngMetadata(outerOrder, adobePrivateData(innerOrder, originalOffset));
  auto image = ImageFactory::open(encoded.data(), encoded.size());
  image->readMetadata();
  const auto original = imageBytes(*image);

  image->exifData()["Exif.Sony2.VignettingCorrection"] = uint32_t{1};
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(original, imageBytes(*image));

  image->readMetadata();
  auto field = image->exifData().findKey(ExifKey("Exif.SonyMisc3c.0x0054"));
  ASSERT_NE(image->exifData().end(), field);
  image->exifData().erase(field);
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(original, imageBytes(*image));

  image->readMetadata();
  const auto duplicate = image->exifData()["Exif.SonyMisc3c.0x0054"];
  image->exifData().add(duplicate);
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(original, imageBytes(*image));
}

// Clearing the complete Exif category removes both the private parent and view.
TEST(SonyDng, ClearsExifCategory) {
  const auto encoded = dngMetadata(littleEndian, adobePrivateData(littleEndian, 1006));
  auto image = ImageFactory::open(encoded.data(), encoded.size());
  image->readMetadata();
  image->clearExifData();
  ASSERT_NO_THROW(image->writeMetadata());
  image->readMetadata();
  EXPECT_EQ(image->exifData().end(), image->exifData().findKey(ExifKey("Exif.Image.DNGPrivateData")));
  EXPECT_EQ(image->exifData().end(), image->exifData().findKey(ExifKey("Exif.Sony2.VignettingCorrection")));
  EXPECT_EQ(1, image->pixelWidth());
}

// Native Sony MakerNotes keep precedence when both representations are present.
TEST(SonyDng, PrefersNativeMakerNote) {
  Blob native;
  appendShort(native, 1, littleEndian);
  appendShort(native, 0x2011, littleEndian);
  appendShort(native, unsignedLong, littleEndian);
  appendLong(native, 1, littleEndian);
  appendLong(native, 1, littleEndian);
  appendLong(native, 0, littleEndian);
  const auto encoded = dngMetadata(littleEndian, adobePrivateData(littleEndian, 1006), native);
  ExifData decoded;
  ExifParser::decode(decoded, encoded.data(), encoded.size(), DecodeParams{100});

  ASSERT_NE(decoded.end(), decoded.findKey(ExifKey("Exif.Sony2.VignettingCorrection")));
  EXPECT_EQ(1, decoded["Exif.Sony2.VignettingCorrection"].toInt64());
  EXPECT_EQ(decoded.end(), decoded.findKey(ExifKey("Exif.SonyMisc3c.SonyImageHeight")));
}

// Malformed optional framing stays opaque without losing standard Exif values.
TEST(SonyDng, KeepsMalformedRecordsOpaque) {
  const auto original = adobePrivateData(littleEndian, 1006);
  std::vector<Blob> mutations;
  auto changed = original;
  changed.resize(10);
  mutations.push_back(changed);
  changed = original;
  ul2Data(changed.data() + 10, 0xffffffff, bigEndian);
  mutations.push_back(changed);
  changed = original;
  changed[26] = 'Z';
  mutations.push_back(changed);
  changed = original;
  ul2Data(changed.data() + 22, 5, bigEndian);
  mutations.push_back(changed);
  changed = original;
  us2Data(changed.data() + 32, 300, littleEndian);
  mutations.push_back(changed);
  changed = original;
  changed.erase(changed.begin() + 17);
  mutations.push_back(changed);
  changed = original;
  ul2Data(changed.data() + changed.size() - 8, 0xffffffff, bigEndian);
  mutations.push_back(changed);
  changed = original;
  appendRecord(changed, {'M', 'a', 'k', 'N'}, Blob(original.begin() + 26, original.end() - 12));
  mutations.push_back(changed);
  changed = {'A', 'd', 'o', 'b', 'e', 0};
  for (size_t i = 0; i < 257; ++i)
    appendRecord(changed, {'T', 'e', 's', 't'}, {});
  mutations.push_back(changed);

  for (size_t i = 0; i < mutations.size(); ++i) {
    SCOPED_TRACE(i);
    const auto encoded = dngMetadata(littleEndian, mutations[i]);
    ExifData decoded;
    ASSERT_NO_THROW(ExifParser::decode(decoded, encoded.data(), encoded.size(), DecodeParams{100}));
    EXPECT_EQ("SONY", decoded["Exif.Image.Make"].toString());
    EXPECT_EQ(mutations[i], rawValue(decoded["Exif.Image.DNGPrivateData"]));
    EXPECT_EQ(decoded.end(), decoded.findKey(ExifKey("Exif.Sony2.VignettingCorrection")));

    // Unrelated edits must still preserve malformed optional data opaquely.
    auto image = ImageFactory::open(encoded.data(), encoded.size());
    image->readMetadata();
    image->exifData()["Exif.Image.Artist"] = "artist";
    ASSERT_NO_THROW(image->writeMetadata());
    image->readMetadata();
    EXPECT_EQ(mutations[i], rawValue(image->exifData()["Exif.Image.DNGPrivateData"]));
  }
}

// Unsupported headered and non-MakN records remain available as opaque data.
TEST(SonyDng, PreservesUnsupportedLayouts) {
  Blob unknown{'A', 'd', 'o', 'b', 'e', 0};
  appendRecord(unknown, {'S', 'R', '2', ' '}, {'k', 'e', 'e', 'p'});
  Blob headered{'A', 'd', 'o', 'b', 'e', 0};
  Blob makerNote{'I', 'I', 0, 0, 3, 0xee, 'S', 'O', 'N', 'Y', ' ', 'D', 'S', 'C', ' ', 0, 0, 0};
  const auto directory = sonyDirectory(littleEndian, 1006);
  makerNote.insert(makerNote.end(), directory.begin(), directory.end());
  appendRecord(headered, {'M', 'a', 'k', 'N'}, makerNote);

  for (const auto& privateData : {unknown, headered}) {
    const auto encoded = dngMetadata(littleEndian, privateData);
    auto image = ImageFactory::open(encoded.data(), encoded.size());
    image->readMetadata();
    EXPECT_EQ(image->exifData().end(), image->exifData().findKey(ExifKey("Exif.Sony2.VignettingCorrection")));
    image->exifData()["Exif.Image.Artist"] = "artist";
    ASSERT_NO_THROW(image->writeMetadata());
    image->readMetadata();
    EXPECT_EQ(privateData, rawValue(image->exifData()["Exif.Image.DNGPrivateData"]));
  }
}

// Bound binary-array expansion even when the saved original offset is zero.
TEST(SonyDng, LimitsExpandedFields) {
  for (const auto origin : {0U, 1006U}) {
    auto directory = sonyDirectory(littleEndian, origin);
    ul2Data(directory.data() + 18, 66000, littleEndian);
    directory.resize(30 + 66000, 0);
    Blob makerNote{'I', 'I'};
    appendLong(makerNote, origin, bigEndian);
    makerNote.insert(makerNote.end(), directory.begin(), directory.end());
    Blob privateData{'A', 'd', 'o', 'b', 'e', 0};
    appendRecord(privateData, {'M', 'a', 'k', 'N'}, makerNote);
    const auto encoded = dngMetadata(littleEndian, privateData);
    ExifData decoded;
    ASSERT_NO_THROW(ExifParser::decode(decoded, encoded.data(), encoded.size(), DecodeParams{100}));
    EXPECT_EQ(decoded.end(), decoded.findKey(ExifKey("Exif.Sony2.VignettingCorrection")));
    EXPECT_EQ(privateData, rawValue(decoded["Exif.Image.DNGPrivateData"]));
  }
}

// Values must not resolve before MakN or into the next private-data record.
TEST(SonyDng, BoundsOffsetsToMakerNotePayload) {
  for (const auto offset : {1005U, 1006U + 110U}) {
    auto privateData = adobePrivateData(littleEndian, 1006);
    ul2Data(privateData.data() + 54, offset, littleEndian);
    const auto encoded = dngMetadata(littleEndian, privateData);
    ExifData decoded;
    ASSERT_NO_THROW(ExifParser::decode(decoded, encoded.data(), encoded.size(), DecodeParams{100}));
    EXPECT_EQ(0, decoded["Exif.Sony2.VignettingCorrection"].toInt64());
    EXPECT_EQ(decoded.end(), decoded.findKey(ExifKey("Exif.SonyMisc3c.SonyImageHeight")));
    EXPECT_EQ(privateData, rawValue(decoded["Exif.Image.DNGPrivateData"]));
  }
}

INSTANTIATE_TEST_SUITE_P(OrdersAndOffsets, SonyDngMakerNotes,
                         ::testing::Combine(::testing::Values(littleEndian, bigEndian),
                                            ::testing::Values(littleEndian, bigEndian),
                                            ::testing::Values(0U, 1006U, 0xffff0000U)));

}  // namespace
