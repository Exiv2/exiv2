// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>
#include "config.h"
#include "unittest_utils.hpp"
#include "value_int.hpp"

#include <gtest/gtest.h>

#include <array>
#include <string>
#include <vector>

namespace {
using namespace Exiv2;
const ExifKey deviceKey("Exif.Photo.DeviceSettingDescription");
const std::string deviceXmp = "Xmp.exif.DeviceSettingDescription";

// Append a fixture code unit in the explicitly selected byte order.
void appendUnit(Blob& bytes, uint16_t value, ByteOrder order) {
  std::array<byte, 2> unit;
  us2Data(unit.data(), value, order);
  bytes.insert(bytes.end(), unit.begin(), unit.end());
}

// Construct native dimensions and independently signed UCS-2 setting strings.
Blob settingsBytes(ByteOrder order, uint16_t columns = 2, uint16_t rows = 1,
                   const std::vector<std::u16string>& settings = {u"Auto", u"\u017dlu\u0165"},
                   ByteOrder textOrder = bigEndian, bool signatures = true) {
  Blob bytes;
  appendUnit(bytes, columns, order);
  appendUnit(bytes, rows, order);
  for (const auto& text : settings) {
    if (signatures)
      appendUnit(bytes, 0xfeff, textOrder);
    for (const auto unit : text)
      appendUnit(bytes, unit, textOrder);
    appendUnit(bytes, 0, textOrder);
  }
  return bytes;
}

// Decode a real TIFF boundary so the fixture carries its original byte order.
ExifData decodedSettings(const Blob& bytes, ByteOrder order) {
  ExifData source;
  DataValue raw(bytes.data(), bytes.size());
  source.add(deviceKey, &raw);
  Blob tiff;
  ExifParser::encode(tiff, order, source);
  ExifData decoded;
  ExifParser::decode(decoded, tiff.data(), tiff.size(), defaultDecodeParams());
  return decoded;
}

// Build the standard structured representation without requiring the toolkit.
XmpData structuredSettings(const std::vector<std::string>& settings = {"Auto", "\xc5\xbdlu\xc5\xa5"},
                           const std::string& field = "Values") {
  XmpData xmp;
  XmpTextValue parent;
  parent.setXmpStruct();
  xmp[deviceXmp] = parent;
  xmp[deviceXmp + "/exif:Columns"] = "2";
  xmp[deviceXmp + "/exif:Rows"] = "1";
  if (!settings.empty()) {
    XmpArrayValue values(xmpSeq);
    for (const auto& text : settings)
      Internal::XmpArrayValueAccess::append(values, text);
    xmp[deviceXmp + "/exif:" + field] = values;
  }
  return xmp;
}

// Compare every record's key, type, and value, including unknown struct fields.
std::vector<std::string> snapshot(const XmpData& xmp) {
  std::vector<std::string> result;
  for (const auto& md : xmp)
    result.push_back(md.key() + "|" + md.typeName() + "|" + md.toString());
  return result;
}

// Read the exact native field bytes in an explicitly selected output order.
Blob nativeBytes(const ExifData& exif, ByteOrder order) {
  const auto pos = exif.findKey(deviceKey);
  if (pos == exif.end())
    return {};
  Blob bytes(pos->size());
  pos->copy(bytes.data(), order);
  return bytes;
}

// Require dimensions and the entire ordered list rather than serialized XML.
void expectSettings(const XmpData& xmp, const std::string& field = "Values") {
  const auto columns = xmp.findKey(XmpKey(deviceXmp + "/exif:Columns"));
  const auto rows = xmp.findKey(XmpKey(deviceXmp + "/exif:Rows"));
  const auto values = xmp.findKey(XmpKey(deviceXmp + "/exif:" + field));
  ASSERT_NE(xmp.end(), columns);
  ASSERT_NE(xmp.end(), rows);
  ASSERT_NE(xmp.end(), values);
  EXPECT_EQ("2", columns->toString());
  EXPECT_EQ("1", rows->toString());
  EXPECT_EQ(xmpSeq, values->typeId());
  ASSERT_EQ(2U, values->count());
  EXPECT_EQ("Auto", values->toString(0));
  EXPECT_EQ("\xc5\xbdlu\xc5\xa5", values->toString(1));
}
}  // namespace

// Export both TIFF orders after copying their metadata away from the image.
TEST(DeviceSettingsConversion, ExportsNativeValuesAfterMetadataCopies) {
  for (const auto order : {littleEndian, bigEndian}) {
    const auto source = decodedSettings(settingsBytes(order), order);
    ExifData copied(source);
    ExifData assigned;
    assigned = copied;
    XmpData xmp;
    copyExifToXmp(assigned, xmp);
    expectSettings(xmp);
    EXPECT_EQ(settingsBytes(order), nativeBytes(source, order));
  }
}

// Reconstruct a fresh native field and serialize it into either TIFF order.
TEST(DeviceSettingsConversion, ReconstructsFreshNativeExifInBothOrders) {
  const auto xmp = structuredSettings();
  const auto before = snapshot(xmp);
  for (const auto order : {littleEndian, bigEndian}) {
    ExifData reconstructed;
    copyXmpToExif(xmp, reconstructed);
    ASSERT_NE(reconstructed.end(), reconstructed.findKey(deviceKey));
    const auto reopened = decodedSettings(nativeBytes(reconstructed, order), order);
    XmpData result;
    copyExifToXmp(reopened, result);
    expectSettings(result);
  }
  EXPECT_EQ(before, snapshot(xmp));
}

// The reporter's zero-size structure must survive conversion and encoding.
TEST(DeviceSettingsConversion, PreservesZeroDimensionsAndStructuredEncoding) {
  auto exif = decodedSettings(settingsBytes(bigEndian, 0, 0, {}), bigEndian);
  auto xmp = structuredSettings({});
  xmp[deviceXmp + "/exif:Columns"] = "0";
  xmp[deviceXmp + "/exif:Rows"] = "0";
  copyExifToXmp(exif, xmp);
  ASSERT_NE(xmp.end(), xmp.findKey(XmpKey(deviceXmp + "/exif:Columns")));
  EXPECT_EQ("0", xmp[deviceXmp + "/exif:Columns"].toString());
#ifdef EXV_HAVE_XMP_TOOLKIT
  std::string packet;
  ASSERT_EQ(0, XmpParser::encode(packet, xmp));
  XmpData reopened;
  ASSERT_EQ(0, XmpParser::decode(reopened, packet, defaultDecodeParams()));
  EXPECT_EQ("0", reopened[deviceXmp + "/exif:Rows"].toString());
#endif
}

// Accept Adobe's deployed Settings spelling while keeping its ordered values.
TEST(DeviceSettingsConversion, AcceptsAdobeSettingsAlias) {
  const auto source = structuredSettings({"Auto", "\xc5\xbdlu\xc5\xa5"}, "Settings");
  ExifData exif;
  copyXmpToExif(source, exif);
  ASSERT_NE(exif.end(), exif.findKey(deviceKey));
  auto destination = source;
  copyExifToXmp(exif, destination);
  expectSettings(destination, "Settings");
}

// Preserve extra XMP fields when refreshing native-backed standard fields.
TEST(DeviceSettingsConversion, PreservesUnknownFieldsAndUnrelatedProperties) {
  auto xmp = structuredSettings();
  xmp[deviceXmp + "/exif:VendorSetting"] = "preserve-canary";
  xmp["Xmp.xmp.Label"] = "unrelated-canary";
  const auto exif = decodedSettings(settingsBytes(littleEndian), littleEndian);
  copyExifToXmp(exif, xmp);
  expectSettings(xmp);
  EXPECT_EQ("preserve-canary", xmp[deviceXmp + "/exif:VendorSetting"].toString());
  EXPECT_EQ("unrelated-canary", xmp["Xmp.xmp.Label"].toString());
}

// Move only successfully converted records; keep unrepresented extension data.
TEST(DeviceSettingsConversion, MovesConvertedValuesWithoutDiscardingExtensions) {
  auto exif = decodedSettings(settingsBytes(littleEndian), littleEndian);
  XmpData xmp;
  moveExifToXmp(exif, xmp);
  EXPECT_EQ(exif.end(), exif.findKey(deviceKey));
  expectSettings(xmp);
  xmp[deviceXmp + "/exif:VendorSetting"] = "keep";
  moveXmpToExif(xmp, exif);
  EXPECT_NE(exif.end(), exif.findKey(deviceKey));
  EXPECT_EQ("keep", xmp[deviceXmp + "/exif:VendorSetting"].toString());
  EXPECT_EQ(xmp.end(), xmp.findKey(XmpKey(deviceXmp + "/exif:Columns")));
}

// Invalid native payloads must not partially replace a valid XMP subtree.
TEST(DeviceSettingsConversion, RejectsMalformedNativeValuesWithoutMutation) {
  std::vector<Blob> malformed{
      {0, 0, 0}, {2, 0, 1, 0, 0}, {2, 0, 1, 0, 0xff, 0xfe, 'A', 0}, {2, 0, 1, 0, 0xff, 0xfe, 0, 0xd8, 0, 0}};
  for (const auto& bytes : malformed) {
    auto exif = decodedSettings(bytes, littleEndian);
    auto xmp = structuredSettings();
    const auto before = snapshot(xmp);
    moveExifToXmp(exif, xmp);
    EXPECT_EQ(before, snapshot(xmp));
    EXPECT_NE(exif.end(), exif.findKey(deviceKey));
  }
}

// Reject invalid dimensions, arrays, and text before changing either container.
TEST(DeviceSettingsConversion, RejectsMalformedXmpWithoutMutation) {
  for (const char* bad : {"-1", "65536", "1.5", "garbage"}) {
    auto xmp = structuredSettings();
    xmp[deviceXmp + "/exif:Columns"] = bad;
    const auto before = snapshot(xmp);
    auto exif = decodedSettings(settingsBytes(bigEndian), bigEndian);
    const auto native = nativeBytes(exif, bigEndian);
    moveXmpToExif(xmp, exif);
    EXPECT_EQ(before, snapshot(xmp));
    EXPECT_EQ(native, nativeBytes(exif, bigEndian));
  }
  for (const std::string& text : {std::string("embedded\0nul", 12), std::string("\xf0\x9f\x98\x80"),
                                  std::string("\xc0\xaf"), std::string("\x01")}) {
    auto xmp = structuredSettings({text});
    ExifData exif;
    moveXmpToExif(xmp, exif);
    EXPECT_TRUE(exif.empty());
    EXPECT_FALSE(xmp.empty());
  }
}

// Synchronization without digests preserves an existing structured destination.
TEST(DeviceSettingsConversion, SynchronizationPreservesExistingXmpWithoutDigests) {
  auto exif = decodedSettings(settingsBytes(littleEndian), littleEndian);
  auto xmp = structuredSettings({"existing"});
  syncExifWithXmp(exif, xmp);
  EXPECT_EQ("existing", xmp[deviceXmp + "/exif:Values"].toString(0));
  EXPECT_NE(exif.end(), exif.findKey(deviceKey));
}

// The old decimal byte-list representation remains importable without guessing.
TEST(DeviceSettingsConversion, ImportsLegacyScalarByteLists) {
  XmpData xmp;
  xmp[deviceXmp] = "0 0 0 0";
  ExifData exif;
  copyXmpToExif(xmp, exif);
  EXPECT_EQ((Blob{0, 0, 0, 0}), nativeBytes(exif, littleEndian));

  // A genuinely parsed destination supplies context for the old byte-list form.
  auto parsed = decodedSettings(settingsBytes(bigEndian), bigEndian);
  xmp[deviceXmp] = "0 7 0 3";
  copyXmpToExif(xmp, parsed);
  EXPECT_EQ((Blob{0, 7, 0, 3}), nativeBytes(parsed, bigEndian));

  // An untyped destination is no more evidence of byte order than a fresh one.
  const Blob original{9, 0, 3, 0};
  DataValue raw(original.data(), original.size());
  ExifData untyped;
  untyped.add(deviceKey, &raw);
  xmp[deviceXmp] = "2 0 1 0";
  const auto before = snapshot(xmp);
  moveXmpToExif(xmp, untyped);
  EXPECT_EQ(original, nativeBytes(untyped, littleEndian));
  EXPECT_EQ(before, snapshot(xmp));
  ExifData fresh;
  copyXmpToExif(xmp, fresh);
  EXPECT_EQ(fresh.end(), fresh.findKey(deviceKey));
}

// Real TIFF serialization must preserve values when either byte order changes.
TEST(DeviceSettingsConversion, RoundtripsTiffOrdersAndSignedOrLegacyText) {
  for (const auto sourceOrder : {littleEndian, bigEndian}) {
    for (const auto textOrder : {littleEndian, bigEndian}) {
      const auto original =
          decodedSettings(settingsBytes(sourceOrder, 2, 1, {u"Auto", u"\u017dlu\u0165"}, textOrder), sourceOrder);
      for (const auto targetOrder : {littleEndian, bigEndian}) {
        Blob tiff;
        auto writable = original;
        ExifParser::encode(tiff, targetOrder, writable);
        ExifData reopened;
        ExifParser::decode(reopened, tiff.data(), tiff.size(), defaultDecodeParams());
        XmpData xmp;
        copyExifToXmp(reopened, xmp);
        expectSettings(xmp);
      }
    }
    const auto unsignedText = decodedSettings(
        settingsBytes(sourceOrder, 2, 1, {u"Auto", u"\u017dlu\u0165"}, sourceOrder, false), sourceOrder);
    const auto target = sourceOrder == littleEndian ? bigEndian : littleEndian;
    Blob tiff;
    auto writable = unsignedText;
    ExifParser::encode(tiff, target, writable);
    ExifData reopened;
    ExifParser::decode(reopened, tiff.data(), tiff.size(), defaultDecodeParams());
    XmpData xmp;
    copyExifToXmp(reopened, xmp);
    expectSettings(xmp);
  }
}

// Empty text is a real list entry, and display dimensions do not imply list length.
TEST(DeviceSettingsConversion, PreservesEmptySettingsAndIndependentDimensions) {
  const auto source = decodedSettings(settingsBytes(bigEndian, 65535, 0, {u"", u"\u4e00"}), bigEndian);
  XmpData xmp;
  copyExifToXmp(source, xmp);
  EXPECT_EQ("65535", xmp[deviceXmp + "/exif:Columns"].toString());
  EXPECT_EQ("0", xmp[deviceXmp + "/exif:Rows"].toString());
  ASSERT_EQ(2U, xmp[deviceXmp + "/exif:Values"].count());
  EXPECT_EQ("", xmp[deviceXmp + "/exif:Values"].toString(0));
  EXPECT_EQ("\xe4\xb8\x80", xmp[deviceXmp + "/exif:Values"].toString(1));
  ExifData native;
  copyXmpToExif(xmp, native);
  ASSERT_NE(native.end(), native.findKey(deviceKey));
}

// Conflicting aliases, non-ordered arrays, missing dimensions, and duplicates are rejected atomically.
TEST(DeviceSettingsConversion, RejectsAmbiguousStructuresWithoutMutation) {
  std::vector<XmpData> invalid;
  auto aliases = structuredSettings();
  XmpArrayValue conflict(xmpSeq);
  conflict.read("conflicting-canary");
  aliases[deviceXmp + "/exif:Settings"] = conflict;
  invalid.push_back(aliases);
  auto unordered = structuredSettings();
  XmpArrayValue bag(xmpBag);
  bag.read("Auto");
  unordered[deviceXmp + "/exif:Values"] = bag;
  invalid.push_back(unordered);
  auto missing = structuredSettings();
  missing.erase(missing.findKey(XmpKey(deviceXmp + "/exif:Rows")));
  invalid.push_back(missing);
  auto duplicate = structuredSettings();
  duplicate.add(*duplicate.findKey(XmpKey(deviceXmp + "/exif:Rows")));
  invalid.push_back(duplicate);
  for (auto& xmp : invalid) {
    const auto before = snapshot(xmp);
    auto exif = decodedSettings(settingsBytes(bigEndian), bigEndian);
    const auto native = nativeBytes(exif, bigEndian);
    moveXmpToExif(xmp, exif);
    EXPECT_EQ(before, snapshot(xmp));
    EXPECT_EQ(native, nativeBytes(exif, bigEndian));
  }
}

// An implicit parent and a similar unrelated key must not interfere with conversion.
TEST(DeviceSettingsConversion, AcceptsDescendantsOnlyAndKeepsSimilarKeys) {
  auto xmp = structuredSettings();
  xmp.erase(xmp.findKey(XmpKey(deviceXmp)));
  xmp[deviceXmp + "Other"] = "similar-canary";
  ExifData exif;
  copyXmpToExif(xmp, exif);
  ASSERT_NE(exif.end(), exif.findKey(deviceKey));
  copyExifToXmp(exif, xmp);
  expectSettings(xmp);
  EXPECT_EQ("similar-canary", xmp[deviceXmp + "Other"].toString());
}

// Successful moves remove the mapped subtree completely when no extensions remain.
TEST(DeviceSettingsConversion, MovesCompleteSubtreeAndPreservesEquivalentNativeBytes) {
  auto xmp = structuredSettings();
  auto exif = decodedSettings(settingsBytes(bigEndian), bigEndian);
  const auto original = nativeBytes(exif, bigEndian);
  moveXmpToExif(xmp, exif);
  EXPECT_TRUE(xmp.empty());
  EXPECT_EQ(original, nativeBytes(exif, bigEndian));
}

// Digest-based synchronization must update the native field from newer XMP.
TEST(DeviceSettingsConversion, SynchronizesBothDirectionsUsingDigests) {
#ifdef EXV_HAVE_XMP_TOOLKIT
  auto exif = decodedSettings(settingsBytes(bigEndian), bigEndian);
  XmpData xmp;
  syncExifWithXmp(exif, xmp);
  xmp[deviceXmp + "/exif:Columns"] = "7";
  syncExifWithXmp(exif, xmp);
  XmpData checked;
  copyExifToXmp(exif, checked);
  EXPECT_EQ("7", checked[deviceXmp + "/exif:Columns"].toString());

  const auto changed = decodedSettings(settingsBytes(bigEndian, 9, 3), bigEndian);
  exif[deviceKey.key()] = changed.findKey(deviceKey)->value();
  syncExifWithXmp(exif, xmp);
  EXPECT_EQ("9", xmp[deviceXmp + "/exif:Columns"].toString());
  EXPECT_EQ("3", xmp[deviceXmp + "/exif:Rows"].toString());
#else
  GTEST_SKIP() << "Native XMP digests require the toolkit";
#endif
}

// Decoding an ordered array must retain empty entries and their positions.
TEST(DeviceSettingsConversion, PreservesEmptyArrayEntriesAcrossXmpEncoding) {
#ifdef EXV_HAVE_XMP_TOOLKIT
  const auto source = structuredSettings({"", "Auto", ""});
  std::string packet;
  ASSERT_EQ(0, XmpParser::encode(packet, source));
  XmpData reopened;
  ASSERT_EQ(0, XmpParser::decode(reopened, packet, defaultDecodeParams()));
  const auto values = reopened.findKey(XmpKey(deviceXmp + "/exif:Values"));
  ASSERT_NE(reopened.end(), values);
  ASSERT_EQ(3U, values->count());
  EXPECT_EQ("", values->toString(0));
  EXPECT_EQ("Auto", values->toString(1));
  EXPECT_EQ("", values->toString(2));
#else
  GTEST_SKIP() << "XMP encoding requires the toolkit";
#endif
}

// Raw caller values without TIFF context must never acquire a guessed byte order.
TEST(DeviceSettingsConversion, RejectsAmbiguousRawByteOrderWithoutMutation) {
  const auto bytes = settingsBytes(littleEndian);
  DataValue raw(bytes.data(), bytes.size());
  ExifData exif;
  exif.add(deviceKey, &raw);
  auto xmp = structuredSettings();
  const auto before = snapshot(xmp);
  moveExifToXmp(exif, xmp);
  EXPECT_EQ(before, snapshot(xmp));
  EXPECT_NE(exif.end(), exif.findKey(deviceKey));
}

// Exact parsed array items retain empties without changing the public empty setter.
TEST(DeviceSettingsConversion, PreservesGeneralParsedArrayEmptiesAndSetterBehavior) {
#ifdef EXV_HAVE_XMP_TOOLKIT
  XmpArrayValue array(xmpBag);
  array.read("");
  EXPECT_EQ(0U, array.count());
  Internal::XmpArrayValueAccess::append(array, "");
  array.read("middle");
  Internal::XmpArrayValueAccess::append(array, "");
  XmpData source;
  source["Xmp.dc.subject"] = array;
  std::string packet;
  ASSERT_EQ(0, XmpParser::encode(packet, source));
  XmpData reopened;
  ASSERT_EQ(0, XmpParser::decode(reopened, packet, defaultDecodeParams()));
  const auto item = reopened.findKey(XmpKey("Xmp.dc.subject"));
  ASSERT_NE(reopened.end(), item);
  ASSERT_EQ(3U, item->count());
  EXPECT_EQ("", item->toString(0));
  EXPECT_EQ("middle", item->toString(1));
  EXPECT_EQ("", item->toString(2));
#else
  GTEST_SKIP() << "XMP encoding requires the toolkit";
#endif
}

// A packet's namespace spelling must not change the standard field interpretation.
TEST(DeviceSettingsConversion, AcceptsParsedNamespaceAliases) {
#ifdef EXV_HAVE_XMP_TOOLKIT
  const std::string packet = R"(<x:xmpmeta xmlns:x="adobe:ns:meta/">)"
                             R"(<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">)"
                             R"(<rdf:Description rdf:about="" xmlns:e="http://ns.adobe.com/exif/1.0/">)"
                             R"(<e:DeviceSettingDescription rdf:parseType="Resource">)"
                             R"(<e:Columns>2</e:Columns><e:Rows>1</e:Rows>)"
                             R"(</e:DeviceSettingDescription></rdf:Description></rdf:RDF></x:xmpmeta>)";
  XmpData source;
  ASSERT_EQ(0, XmpParser::decode(source, packet, defaultDecodeParams()));
  ExifData exif;
  copyXmpToExif(source, exif);
  ASSERT_NE(exif.end(), exif.findKey(deviceKey));
  XmpData result;
  copyExifToXmp(exif, result);
  EXPECT_EQ("2", result[deviceXmp + "/exif:Columns"].toString());
  EXPECT_EQ("1", result[deviceXmp + "/exif:Rows"].toString());
#else
  GTEST_SKIP() << "XMP parsing requires the toolkit";
#endif
}
