// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/basicio.hpp>
#include <exiv2/error.hpp>
#include <exiv2/image.hpp>
#include <exiv2/iptc.hpp>
#include <exiv2/tags.hpp>
#include <exiv2/tiffimage.hpp>
#include <exiv2/value.hpp>
#include "jxrimage_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "tiffvisitor_int.hpp"
#include "unittest_utils.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

namespace {
using namespace Exiv2;
using namespace Exiv2::Internal;
using Bytes = std::vector<byte>;

// Construct a small valid XMP packet with a distinct per-image removal canary.
std::string packet(size_t image) {
  return "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\"><rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">"
         "<rdf:Description rdf:about=\"\" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" dc:format=\"XMP-CANARY-" +
         std::to_string(image) + "\"/></rdf:RDF></x:xmpmeta>";
}

// Encode an unsigned TIFF scalar without depending on host byte order.
Bytes number(uint32_t value) {
  Bytes bytes(4);
  ul2Data(bytes.data(), value, littleEndian);
  return bytes;
}

// Include the required terminator in a TIFF UTF-8/ASCII value.
Bytes text(const std::string& value) {
  Bytes bytes(value.begin(), value.end());
  bytes.push_back(0);
  return bytes;
}

// Describe a synthetic directory field independently of the production TIFF parser.
struct Field {
  uint16_t tag;
  uint16_t type;
  uint32_t count;
  Bytes bytes;
};

// Build ordered JXR images and metadata subdirectories with distinct opaque streams.
// These payloads test container mechanics; actual decoder validation uses real fixtures.
Bytes fixture(size_t images = 1, bool previewFirst = false, bool alpha = false, bool eof = false) {
  std::vector<std::vector<Field>> directories;
  for (size_t i = 0; i < images; ++i) {
    const auto make = text("MAKE-CANARY-" + std::to_string(i));
    const auto xml = packet(i);
    IptcData iptc;
    iptc["Iptc.Application2.Caption"] = "IPTC-CANARY-" + std::to_string(i);
    const auto encodedIptc = IptcParser::encode(iptc);
    Bytes profile(128, 0);
    ul2Data(profile.data(), profile.size(), bigEndian);
    const std::string marker = "ICC-CANARY-" + std::to_string(i);
    std::copy(marker.begin(), marker.end(), profile.begin() + 32);
    std::vector<Field> fields{
        {0x010f, 2, static_cast<uint32_t>(make.size()), make},
        {0x02bc, 1, static_cast<uint32_t>(xml.size()), Bytes(xml.begin(), xml.end())},
        {0x83bb, 7, static_cast<uint32_t>(encodedIptc.size()), Bytes(encodedIptc.begin(), encodedIptc.end())},
        {0x8769, 4, 1, number(0)},
        {0x8773, 7, static_cast<uint32_t>(profile.size()), profile},
        {0xbc01, 1, 16, Bytes(16, 42)},
        {0xbc04, 4, 1, number(previewFirst && i == 0 ? 1 : 2)},
        {0xbc80, 4, 1, number(static_cast<uint32_t>(16 + i))},
        {0xbc81, 4, 1, number(16)},
        {0xbcc0, 4, 1, number(0)},
        {0xbcc1, 4, 1, number(eof ? 0 : 7)},
    };
    if (alpha) {
      fields.push_back({0xbcc2, 4, 1, number(0)});
      fields.push_back({0xbcc3, 4, 1, number(5)});
    }
    directories.push_back(std::move(fields));
  }
  for (size_t i = 0; i < images; ++i) {
    auto date = text("2026:10:18 12:34:56");
    directories.push_back({{0x9003, 2, static_cast<uint32_t>(date.size()), std::move(date)}});
  }

  // Reserve all tables first so nested pointers and next-image links are absolute.
  std::vector<size_t> starts;
  size_t length = 8;
  for (const auto& fields : directories) {
    starts.push_back(length);
    length += 6 + 12 * fields.size();
  }
  Bytes bytes(length, 0);
  bytes[0] = bytes[1] = 'I';
  bytes[2] = 0xbc;
  bytes[3] = 1;
  ul2Data(bytes.data() + 4, 8, littleEndian);
  std::vector<std::pair<size_t, size_t>> streamEntries;
  for (size_t d = 0; d < directories.size(); ++d) {
    us2Data(bytes.data() + starts[d], directories[d].size(), littleEndian);
    for (size_t f = 0; f < directories[d].size(); ++f) {
      auto field = directories[d][f];
      const size_t at = starts[d] + 2 + f * 12;
      us2Data(bytes.data() + at, field.tag, littleEndian);
      us2Data(bytes.data() + at + 2, field.type, littleEndian);
      ul2Data(bytes.data() + at + 4, field.count, littleEndian);
      if (field.tag == 0x8769)
        field.bytes = number(starts[images + d]);
      if (field.tag == 0xbcc0 || field.tag == 0xbcc2)
        streamEntries.emplace_back(at + 8, field.tag == 0xbcc0 ? 7 : 5);
      if (field.bytes.size() <= 4) {
        std::copy(field.bytes.begin(), field.bytes.end(), bytes.begin() + at + 8);
      } else {
        if (bytes.size() & 1)
          bytes.push_back(0);
        ul2Data(bytes.data() + at + 8, bytes.size(), littleEndian);
        bytes.insert(bytes.end(), field.bytes.begin(), field.bytes.end());
      }
    }
    if (d + 1 < images)
      ul2Data(bytes.data() + starts[d] + 2 + 12 * directories[d].size(), starts[d + 1], littleEndian);
  }
  for (size_t i = 0; i < streamEntries.size(); ++i) {
    if (bytes.size() & 1)
      bytes.push_back(0);
    ul2Data(bytes.data() + streamEntries[i].first, bytes.size(), littleEndian);
    bytes.insert(bytes.end(), streamEntries[i].second, static_cast<byte>(128 + i));
  }
  return bytes;
}

// Open a private factory adapter using an owned copy of the input bytes.
Image::UniquePtr open(const Bytes& bytes) {
  auto io = std::make_unique<MemIo>();
  io->write(bytes.data(), bytes.size());
  return ImageFactory::open(std::move(io));
}

// Snapshot the source after an operation, including rejected writes.
Bytes contents(Image& image) {
  auto& io = image.io();
  io.open();
  const byte* data = io.mmap();
  return {data, data + io.size()};
}

// Locate a top-level field for focused malformed-input mutations.
size_t fieldAt(const Bytes& bytes, uint16_t tag, size_t image = 0) {
  size_t offset = getULong(bytes.data() + 4, littleEndian);
  for (size_t i = 0; i < image; ++i)
    offset = getULong(bytes.data() + offset + 2 + 12 * getUShort(bytes.data() + offset, littleEndian), littleEndian);
  const size_t count = getUShort(bytes.data() + offset, littleEndian);
  for (size_t i = 0; i < count; ++i) {
    const size_t at = offset + 2 + 12 * i;
    if (getUShort(bytes.data() + at, littleEndian) == tag)
      return at;
  }
  throw std::logic_error("fixture field missing");
}

// Inspect compressed streams independently of the production image-data copier.
std::vector<Bytes> streams(const Bytes& bytes) {
  std::vector<Bytes> result;
  size_t offset = getULong(bytes.data() + 4, littleEndian);
  size_t image = 0;
  while (offset) {
    for (uint16_t tag : {0xbcc0, 0xbcc2}) {
      const size_t count = getUShort(bytes.data() + offset, littleEndian);
      for (size_t i = 0; i < count; ++i) {
        const size_t at = offset + 2 + i * 12;
        if (getUShort(bytes.data() + at, littleEndian) != tag)
          continue;
        const size_t start = getULong(bytes.data() + at + 8, littleEndian);
        size_t length = getULong(bytes.data() + fieldAt(bytes, tag + 1, image) + 8, littleEndian);
        if (length == 0)
          length = bytes.size() - start;
        result.emplace_back(bytes.begin() + start, bytes.begin() + start + length);
      }
    }
    offset = getULong(bytes.data() + offset + 2 + 12 * getUShort(bytes.data() + offset, littleEndian), littleEndian);
    ++image;
  }
  return result;
}

// Decode one local tree to inspect secondary metadata without a new public page API.
ExifData imageExif(const Bytes& bytes, size_t image) {
  size_t offset = getULong(bytes.data() + 4, littleEndian);
  for (size_t i = 0; i < image; ++i)
    offset = getULong(bytes.data() + offset + 2 + 12 * getUShort(bytes.data() + offset, littleEndian), littleEndian);
  TiffReadContext context;
  auto tree = TiffParserWorker::parseDirectory(bytes.data(), bytes.size(), offset, context);
  ExifData exif;
  IptcData iptc;
  XmpData xmp;
  TiffDecoder decoder(exif, iptc, xmp, tree.get(), TiffMapping::findDecoder, defaultDecodeParams());
  tree->accept(decoder);
  return exif;
}

// Check that a removed metadata canary no longer survives in serialized bytes.
bool contains(const Bytes& bytes, const std::string& value) {
  return std::search(bytes.begin(), bytes.end(), value.begin(), value.end()) != bytes.end();
}

// Count final transfers so rejection can be distinguished from failed restoration.
class CountingIo : public MemIo {
 public:
  // Keep the synthetic source in writable I/O owned by the factory adapter.
  explicit CountingIo(const Bytes& bytes) : MemIo(bytes.data(), bytes.size()) {
  }
  // Count source replacement, optionally simulating a final-transfer error.
  void transfer(BasicIo& source) override {
    ++transfers;
    if (failTransfer)
      throw Error(ErrorCode::kerImageWriteFailed);
    MemIo::transfer(source);
  }
  size_t transfers{};
  bool failTransfer{};
};

// Verify factory identity, stored geometry, nested metadata and category capabilities.
TEST(JxrImage, FactoryUsesDedicatedAdapter) {
  auto image = open(fixture(6, true, true));
  ASSERT_EQ(ImageType::jxr, image->imageType());
  image->readMetadata();
  EXPECT_EQ("image/jxr", image->mimeType());
  EXPECT_EQ(17U, image->pixelWidth());
  EXPECT_EQ(16U, image->pixelHeight());
  EXPECT_EQ("MAKE-CANARY-1", image->exifData()["Exif.Image.Make"].toString());
  EXPECT_EQ("2026:10:18 12:34:56", image->exifData()["Exif.Photo.DateTimeOriginal"].toString());
  EXPECT_EQ(amReadWrite, image->checkMode(mdIccProfile));
  EXPECT_FALSE(image->iccProfile().empty());
  EXPECT_EQ(packet(1), static_cast<const Image&>(*image).xmpPacket());
}

// Binary XMP/ICC extraction selects the exposed image even when previews precede it.
TEST(JxrImage, BinaryExtractionUsesPrimaryOnly) {
  const auto before = fixture(6, true);
  auto image = open(before);
  std::ostringstream xml;
  image->printStructure(xml, kpsXMP);
  EXPECT_EQ(packet(1), xml.str());
  std::ostringstream icc;
  image->printStructure(icc, kpsIccProfile);
  EXPECT_NE(std::string::npos, icc.str().find("ICC-CANARY-1"));
  EXPECT_EQ(std::string::npos, icc.str().find("ICC-CANARY-0"));
  EXPECT_EQ(before, contents(*image));
}

// Growing primary metadata preserves six directories, secondary values and both stream kinds.
TEST(JxrImage, PrimaryEditsPreserveOtherImagesAndStreams) {
  const auto before = fixture(6, true, true);
  auto image = open(before);
  image->readMetadata();
  image->exifData()["Exif.Image.Make"] = std::string(2048, 'N');
  image->exifData()["Exif.GPSInfo.GPSLatitudeRef"] = "N";
  ASSERT_NO_THROW(image->writeMetadata());
  const auto after = contents(*image);
  EXPECT_EQ(streams(before), streams(after));
  for (size_t i = 0; i < 6; ++i) {
    auto exif = imageExif(after, i);
    EXPECT_EQ(i == 1 ? std::string(2048, 'N') : "MAKE-CANARY-" + std::to_string(i), exif["Exif.Image.Make"].toString());
  }
  image->readMetadata();
  EXPECT_EQ("N", image->exifData()["Exif.GPSInfo.GPSLatitudeRef"].toString());
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_EQ(streams(before), streams(contents(*image)));
}

// Explicit category APIs broadcast removal while preserving all protected native fields.
TEST(JxrImage, ExplicitCategoryClearsRemoveObsoleteBytesThroughout) {
  for (const std::string category : {"exif", "iptc", "xmp", "icc", "all"}) {
    SCOPED_TRACE(category);
    const auto before = fixture(6, true, true);
    auto image = open(before);
    image->readMetadata();
    if (category == "exif")
      image->clearExifData();
    if (category == "iptc")
      image->clearIptcData();
    if (category == "xmp")
      image->clearXmpData();
    if (category == "icc")
      image->clearIccProfile();
    if (category == "all")
      image->clearMetadata();
    ASSERT_NO_THROW(image->writeMetadata());
    const auto after = contents(*image);
    EXPECT_EQ(streams(before), streams(after));
    for (size_t i = 0; i < 6; ++i) {
      EXPECT_EQ(category != "exif" && category != "all", contains(after, "MAKE-CANARY-" + std::to_string(i)));
      EXPECT_EQ(category != "iptc" && category != "all", contains(after, "IPTC-CANARY-" + std::to_string(i)));
      EXPECT_EQ(category != "xmp" && category != "all", contains(after, "XMP-CANARY-" + std::to_string(i)));
      EXPECT_EQ(category != "icc" && category != "all", contains(after, "ICC-CANARY-" + std::to_string(i)));
    }
    image->readMetadata();
    EXPECT_EQ(17U, image->pixelWidth());
  }
}

// Empty category setters have the same global removal semantics as explicit clears.
TEST(JxrImage, EmptySettersClearAllImages) {
  auto image = open(fixture(6, true, true));
  image->readMetadata();
  image->setExifData({});
  image->setIptcData({});
  image->setXmpPacket({});
  image->setIccProfile({}, false);
  ASSERT_NO_THROW(image->writeMetadata());
  const auto after = contents(*image);
  EXPECT_FALSE(contains(after, "CANARY"));
  EXPECT_EQ(12U, streams(after).size());
}

// Mutable Exif/IPTC containers represent only the exposed primary image.
TEST(JxrImage, MutableContainerClearsDoNotBroadcast) {
  auto image = open(fixture(3, true));
  image->readMetadata();
  image->exifData().clear();
  image->iptcData().clear();
  ASSERT_NO_THROW(image->writeMetadata());
  const auto after = contents(*image);
  EXPECT_TRUE(contains(after, "MAKE-CANARY-0"));
  EXPECT_FALSE(contains(after, "MAKE-CANARY-1"));
  EXPECT_TRUE(contains(after, "MAKE-CANARY-2"));
  EXPECT_TRUE(contains(after, "IPTC-CANARY-0"));
  EXPECT_FALSE(contains(after, "IPTC-CANARY-1"));
}

// Reads discard removal intent; successful writes reset it before subsequent edits.
TEST(JxrImage, RepeatedReadsAndWritesResetClearIntent) {
  auto image = open(fixture(3));
  image->readMetadata();
  image->clearMetadata();
  image->readMetadata();
  image->exifData()["Exif.Image.Make"] = "first edit";
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_TRUE(contains(contents(*image), "MAKE-CANARY-2"));
  image->clearExifData();
  image->exifData()["Exif.Image.Make"] = "replacement after clear";
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_FALSE(contains(contents(*image), "MAKE-CANARY-2"));
  EXPECT_EQ("replacement after clear", image->exifData()["Exif.Image.Make"].toString());
}

// The zero image count means EOF and is normalized to an explicit output length.
TEST(JxrImage, EofSizedStreamSurvivesGrowth) {
  const auto before = fixture(1, false, false, true);
  auto image = open(before);
  image->readMetadata();
  image->exifData()["Exif.Image.Model"] = std::string(4096, 'G');
  ASSERT_NO_THROW(image->writeMetadata());
  const auto after = contents(*image);
  EXPECT_EQ(streams(before), streams(after));
  EXPECT_EQ(7U, getULong(after.data() + fieldAt(after, 0xbcc1) + 8, littleEndian));
}

// Raw XMP copying and self-copy preserve exact packets in toolkit-enabled and disabled builds.
TEST(JxrImage, MetadataCopyAndSelfCopyPreserveRawXmp) {
  auto source = open(fixture());
  source->readMetadata();
  source->setXmpPacket(packet(99));
  auto target = open(fixture(3));
  target->readMetadata();
  target->setMetadata(*source);
  ASSERT_NO_THROW(target->writeMetadata());
  EXPECT_EQ(packet(99), static_cast<const Image&>(*target).xmpPacket());
  const auto copied = contents(*target);
  EXPECT_EQ(1U, getUShort(copied.data() + fieldAt(copied, 0x02bc) + 2, littleEndian));
  EXPECT_EQ(7U, getUShort(copied.data() + fieldAt(copied, 0x8773) + 2, littleEndian));
  target->setMetadata(*target);
  ASSERT_NO_THROW(target->writeMetadata());
  EXPECT_EQ(packet(99), static_cast<const Image&>(*target).xmpPacket());
  source->clearXmpPacket();
  target->setMetadata(*source);
  ASSERT_NO_THROW(target->writeMetadata());
  EXPECT_FALSE(contains(contents(*target), "XMP-CANARY"));
}

// Each explicit XMP clear removes raw carriers even when no serializer is available.
TEST(JxrImage, AllXmpClearApisRemoveOldPackets) {
  for (int method = 0; method < 4; ++method) {
    auto image = open(fixture(3));
    image->readMetadata();
    if (method == 0)
      image->clearXmpData();
    if (method == 1)
      image->clearXmpPacket();
    if (method == 2)
      image->setXmpData({});
    if (method == 3)
      image->setXmpPacket({});
    ASSERT_NO_THROW(image->writeMetadata());
    EXPECT_FALSE(contains(contents(*image), "XMP-CANARY"));
  }
}

// Native fields cannot be changed even when a write would otherwise fit in place.
TEST(JxrImage, NativeEditsRejectBeforeSourceTransfer) {
  const auto before = fixture();
  auto storage = std::make_unique<CountingIo>(before);
  auto* io = storage.get();
  auto image = ImageFactory::open(std::move(storage));
  image->readMetadata();
  image->exifData()["Exif.Image.0xbc80"] = uint32_t{123};
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(0U, io->transfers);
  EXPECT_EQ(before, contents(*image));
}

// Partial removal or duplication of the exposed native view is a protected-field edit.
TEST(JxrImage, PartialNativeRemovalAndDuplicatesRejectUnchanged) {
  for (bool duplicate : {false, true}) {
    const auto before = fixture();
    auto image = open(before);
    image->readMetadata();
    const auto field = image->exifData().findKey(ExifKey("Exif.Image.0xbc80"));
    ASSERT_NE(image->exifData().end(), field);
    if (duplicate) {
      const auto value = *field;
      image->exifData().add(value);
    } else {
      image->exifData().erase(field);
    }
    EXPECT_THROW(image->writeMetadata(), Error);
    EXPECT_EQ(before, contents(*image));
  }
}

// A final transfer error retains pending metadata and removal intent for a retry.
TEST(JxrImage, TransferFailureRetainsPendingEdits) {
  const auto before = fixture(3);
  auto storage = std::make_unique<CountingIo>(before);
  auto* io = storage.get();
  auto image = ImageFactory::open(std::move(storage));
  image->readMetadata();
  image->clearXmpData();
  io->failTransfer = true;
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(before, contents(*image));
  io->failTransfer = false;
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_FALSE(contains(contents(*image), "XMP-CANARY"));
}

// Malformed tables, cycles, pointer layouts and shared payload ownership reject unchanged.
TEST(JxrImage, MalformedLayoutsRejectWithoutSourceChanges) {
  const std::vector<std::function<void(Bytes&)>> mutations{
      [](auto& b) { ul2Data(b.data() + 4, 0xfffffff0, littleEndian); },
      [](auto& b) { b.resize(15); },
      [](auto& b) { ul2Data(b.data() + 8 + 2 + 12 * getUShort(b.data() + 8, littleEndian), 8, littleEndian); },
      [](auto& b) { ul2Data(b.data() + fieldAt(b, 0xbcc1) + 8, 0xffffffff, littleEndian); },
      [](auto& b) { ul2Data(b.data() + fieldAt(b, 0xbcc0) + 8, 8, littleEndian); },
      [](auto& b) { us2Data(b.data() + fieldAt(b, 0xbcc0) + 2, 13, littleEndian); },
      [](auto& b) { ul2Data(b.data() + fieldAt(b, 0x010f) + 8, 4, littleEndian); },
      [](auto& b) { ul2Data(b.data() + fieldAt(b, 0x010f) + 4, 0xffffffff, littleEndian); },
      [](auto& b) { ul2Data(b.data() + fieldAt(b, 0x8769) + 8, 8, littleEndian); },
      [](auto& b) { us2Data(b.data() + fieldAt(b, 0x8769), 0x014a, littleEndian); },
  };
  for (size_t i = 0; i < mutations.size(); ++i) {
    SCOPED_TRACE(i);
    auto malformed = fixture();
    mutations[i](malformed);
    auto storage = std::make_unique<CountingIo>(malformed);
    auto* io = storage.get();
    auto image = newJxrInstance(std::move(storage), defaultImageCtorParams(false));
    ASSERT_TRUE(image);
    EXPECT_THROW(image->writeMetadata(), Error);
    EXPECT_EQ(0U, io->transfers);
    EXPECT_EQ(malformed, contents(*image));
  }
}

// TIFF table/free-space offsets have no JPEG XR relocation policy, even as scalar tags.
TEST(JxrImage, UnsupportedTiffPointersRejectWithoutTransfer) {
  for (uint16_t tag : {0x0120, 0x0121, 0x0207, 0x0208, 0x0209}) {
    auto before = fixture();
    const auto at = fieldAt(before, 0x010f);
    us2Data(before.data() + at, tag, littleEndian);
    us2Data(before.data() + at + 2, 4, littleEndian);
    ul2Data(before.data() + at + 4, 1, littleEndian);
    ul2Data(before.data() + at + 8, 123, littleEndian);
    auto storage = std::make_unique<CountingIo>(before);
    auto* io = storage.get();
    auto image = ImageFactory::open(std::move(storage));
    ASSERT_NO_THROW(image->readMetadata());
    image->exifData()["Exif.Image.Model"] = "unrelated edit";
    EXPECT_THROW(image->writeMetadata(), Error);
    EXPECT_EQ(0U, io->transfers);
    EXPECT_EQ(before, contents(*image));
  }
}

// Separate alpha must have both fields and disjoint, bounded, nonempty payloads.
TEST(JxrImage, InvalidAlphaPairsRejectBeforeTransfer) {
  for (int mutation = 0; mutation < 4; ++mutation) {
    auto malformed = fixture(1, false, true);
    const auto alpha = fieldAt(malformed, 0xbcc2);
    const auto count = fieldAt(malformed, 0xbcc3);
    if (mutation == 0)
      us2Data(malformed.data() + count, 0xbcc4, littleEndian);
    if (mutation == 1)
      ul2Data(malformed.data() + count + 8, 0, littleEndian);
    if (mutation == 2)
      ul2Data(malformed.data() + alpha + 8, getULong(malformed.data() + fieldAt(malformed, 0xbcc0) + 8, littleEndian),
              littleEndian);
    if (mutation == 3)
      ul2Data(malformed.data() + alpha + 8, 0xfffffff0, littleEndian);
    auto image = open(malformed);
    EXPECT_THROW(image->writeMetadata(), Error);
    EXPECT_EQ(malformed, contents(*image));
  }
}

// Resolving an EOF count never updates the borrowed input, even before rejection.
TEST(JxrImage, ReadAndRejectedWriteDoNotChangeEofCount) {
  const auto before = fixture(1, false, false, true);
  auto image = open(before);
  image->readMetadata();
  EXPECT_EQ(before, contents(*image));
  image->exifData()["Exif.Image.0xbc80"] = uint32_t{123};
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(before, contents(*image));
}

// The public TIFF parser cannot bypass the JPEG XR safety policy.
TEST(JxrImage, DirectTiffWritesRejectJxr) {
  auto before = fixture();
  MemIo io(before.data(), before.size());
  ExifData exif;
  exif["Exif.Image.Make"] = "Nikon";
  EXPECT_THROW(TiffParser::encode(io, before.data(), before.size(), littleEndian, exif, {}, {}), Error);
  EXPECT_EQ(before, Bytes(io.mmap(), io.mmap() + io.size()));
  EXPECT_THROW(TiffParser::encode(io, nullptr, 0, littleEndian, exif, {}, {}), Error);
  EXPECT_EQ(before, Bytes(io.mmap(), io.mmap() + io.size()));
  TiffImage direct(std::make_unique<MemIo>(before.data(), before.size()), defaultImageCtorParams(false));
  direct.readMetadata();
  direct.exifData()["Exif.Image.Make"] = "Nikon";
  EXPECT_THROW(direct.writeMetadata(), Error);
  EXPECT_EQ(before, contents(direct));
}

// Create a minimal TIFF raster, including a tag that is structural only in JXR.
Bytes tiffFixture() {
  MemIo io;
  ExifData exif;
  exif["Exif.Image.ImageWidth"] = uint32_t{2};
  exif["Exif.Image.ImageLength"] = uint32_t{2};
  exif["Exif.Image.StripOffsets"] = uint32_t{0};
  exif["Exif.Image.StripByteCounts"] = uint32_t{4};
  exif["Exif.Image.0xbcc0"] = uint32_t{123};
  const byte pixels[] = {11, 22, 33, 44};
  exif["Exif.Image.StripOffsets"].setDataArea(pixels, 4);
  TiffParser::encode(io, nullptr, 0, littleEndian, exif, {}, {});
  return {io.mmap(), io.mmap() + io.size()};
}

// Ordinary TIFF rebuilding still preserves its own strip payloads and factory identity.
TEST(JxrImage, OrdinaryTiffStripWritingRemainsUnchanged) {
  auto image = open(tiffFixture());
  ASSERT_EQ(ImageType::tiff, image->imageType());
  image->readMetadata();
  image->exifData()["Exif.Image.Make"] = std::string(2048, 'T');
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_TRUE(contains(contents(*image), std::string("\x0b\x16\x21\x2c", 4)));
  EXPECT_EQ(123U, image->exifData()["Exif.Image.0xbcc0"].toUint32());
  EXPECT_EQ("image/tiff", image->mimeType());
}

// TIFF-derived RAW adapters keep their own headers, strips and ordinary unknown tags.
TEST(JxrImage, TiffDerivedRawWritingRemainsUnchanged) {
  auto bytes = tiffFixture();
  us2Data(bytes.data() + 2, 0x4f52, littleEndian);
  auto image = open(bytes);
  ASSERT_EQ(ImageType::orf, image->imageType());
  image->readMetadata();
  image->exifData()["Exif.Image.Make"] = std::string(2048, 'R');
  ASSERT_NO_THROW(image->writeMetadata());
  const auto after = contents(*image);
  EXPECT_EQ(0x4f52, getUShort(after.data() + 2, littleEndian));
  EXPECT_TRUE(contains(after, std::string("\x0b\x16\x21\x2c", 4)));
  EXPECT_EQ(123U, image->exifData()["Exif.Image.0xbcc0"].toUint32());
  auto reopened = open(after);
  EXPECT_EQ(ImageType::orf, reopened->imageType());
  ASSERT_NO_THROW(reopened->readMetadata());
  EXPECT_EQ(std::string(2048, 'R'), reopened->exifData()["Exif.Image.Make"].toString());
}

// Stream offset/count widths are promoted safely before relocation.
TEST(JxrImage, ShortAndByteStreamFieldsArePromoted) {
  auto before = fixture();
  for (uint16_t tag : {0xbcc0, 0xbcc1})
    us2Data(before.data() + fieldAt(before, tag) + 2, tag == 0xbcc0 ? 3 : 1, littleEndian);
  auto image = open(before);
  image->readMetadata();
  image->exifData()["Exif.Image.Make"] = std::string(4096, 'P');
  ASSERT_NO_THROW(image->writeMetadata());
  const auto after = contents(*image);
  EXPECT_EQ(streams(before), streams(after));
  EXPECT_EQ(4U, getUShort(after.data() + fieldAt(after, 0xbcc0) + 2, littleEndian));
}

// Malformed chain and value budgets are bounded independently of pixel dimensions.
TEST(JxrImage, DirectoryBudgetAndNoPrimaryRejectUnchanged) {
  for (const auto& before : {fixture(1025), fixture(1, true)}) {
    auto image = open(before);
    EXPECT_THROW(image->writeMetadata(), Error);
    EXPECT_EQ(before, contents(*image));
  }
}

// Probe restoration and version checking do not depend on a filename extension.
TEST(JxrImage, ProbeRestoresPositionAndRejectsOtherVersions) {
  const auto before = fixture();
  MemIo io(before.data(), before.size());
  EXPECT_TRUE(isJxrType(io, false));
  EXPECT_EQ(0U, io.tell());
  EXPECT_TRUE(isJxrType(io, true));
  EXPECT_EQ(4U, io.tell());
  const byte shortHeader[] = {'I', 'I', 0xbc};
  MemIo shortIo(shortHeader, sizeof(shortHeader));
  EXPECT_FALSE(isJxrType(shortIo, true));
  EXPECT_EQ(0U, shortIo.tell());
  auto future = before;
  future[3] = 2;
  MemIo futureIo(future.data(), future.size());
  EXPECT_FALSE(isJxrType(futureIo, true));
  EXPECT_EQ(0U, futureIo.tell());
  EXPECT_THROW(ImageFactory::create(ImageType::jxr), Error);
}

// Repeated raw-packet replacement grows metadata without retaining obsolete packet bytes.
TEST(JxrImage, RawPacketReplacementAndMutableRemovalArePrimaryOnly) {
  auto image = open(fixture(3));
  image->readMetadata();
  image->setXmpPacket(packet(99));
  ASSERT_NO_THROW(image->writeMetadata());
  auto after = contents(*image);
  EXPECT_FALSE(contains(after, "XMP-CANARY-0"));
  EXPECT_TRUE(contains(after, "XMP-CANARY-99"));
  EXPECT_TRUE(contains(after, "XMP-CANARY-2"));
  image->xmpPacket().clear();
  ASSERT_NO_THROW(image->writeMetadata());
  after = contents(*image);
  EXPECT_FALSE(contains(after, "XMP-CANARY-99"));
  EXPECT_TRUE(contains(after, "XMP-CANARY-2"));
}

#ifdef EXV_HAVE_XMP_TOOLKIT
// Explicit raw-packet selection remains authoritative over a pending structured view.
TEST(JxrImage, ExplicitXmpPacketSelectionIsHonored) {
  auto image = open(fixture());
  image->readMetadata();
  const auto original = static_cast<const Image&>(*image).xmpPacket();
  image->xmpData()["Xmp.dc.format"] = "pending-structured-value";
  image->writeXmpFromPacket(true);
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_EQ(original, static_cast<const Image&>(*image).xmpPacket());
  EXPECT_EQ("XMP-CANARY-0", image->xmpData()["Xmp.dc.format"].toString());
  auto structured = image->xmpData();
  structured["Xmp.dc.format"] = "explicit-structured-value";
  image->setXmpData(structured);
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_EQ("explicit-structured-value", image->xmpData()["Xmp.dc.format"].toString());
}

// Copying and self-copy also preserve structured edits that have not been written yet.
TEST(JxrImage, MetadataCopyPreservesPendingStructuredXmp) {
  auto source = open(fixture());
  source->readMetadata();
  source->xmpData()["Xmp.dc.format"] = "pending-structured-copy";
  auto target = open(fixture(3));
  target->readMetadata();
  target->setMetadata(*source);
  ASSERT_NO_THROW(target->writeMetadata());
  EXPECT_EQ("pending-structured-copy", target->xmpData()["Xmp.dc.format"].toString());
  source->setMetadata(*source);
  ASSERT_NO_THROW(source->writeMetadata());
  EXPECT_EQ("pending-structured-copy", source->xmpData()["Xmp.dc.format"].toString());
}

// Mutable structured XMP edits and clears affect the primary view without broadcasting.
TEST(JxrImage, MutableStructuredXmpEditsArePrimaryOnly) {
  auto image = open(fixture(3));
  image->readMetadata();
  image->xmpData()["Xmp.dc.format"] = "structured-replacement";
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_EQ("structured-replacement", image->xmpData()["Xmp.dc.format"].toString());
  EXPECT_TRUE(contains(contents(*image), "XMP-CANARY-2"));
  image->xmpData().clear();
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_FALSE(contains(contents(*image), "structured-replacement"));
  EXPECT_TRUE(contains(contents(*image), "XMP-CANARY-2"));
}
#endif

#ifndef EXV_HAVE_XMP_TOOLKIT
// A mutable structured-XMP edit must also reject when toolkit serialization is absent.
TEST(JxrImage, StructuredXmpWithoutToolkitRejectsUnchanged) {
  const auto before = fixture();
  auto image = open(before);
  image->readMetadata();
  XmpData structured;
  structured["Xmp.dc.format"] = "unavailable";
  EXPECT_THROW(image->setXmpData(structured), Error);
  image->xmpData() = structured;
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(before, contents(*image));
}
#endif

}  // namespace
