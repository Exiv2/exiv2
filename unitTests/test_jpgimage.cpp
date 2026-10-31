// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>
#include "config.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace {
using namespace Exiv2;
constexpr size_t maxExifFragment = 65527;
constexpr std::array<byte, 6> exifId{'E', 'x', 'i', 'f', 0, 0};

// Append an ordinary JPEG marker, its encoded length, and its payload.
void segment(Blob& image, byte marker, const Blob& payload) {
  ASSERT_LE(payload.size(), 65533U);
  const auto size = static_cast<uint16_t>(payload.size() + 2);
  image.insert(image.end(), {0xff, marker, static_cast<byte>(size >> 8), static_cast<byte>(size)});
  image.insert(image.end(), payload.begin(), payload.end());
}

// Construct a TIFF with out-of-line text crossing fragment boundaries.
Blob tiff(ByteOrder order, size_t size = 70000, char fill = 'q') {
  Blob bytes(size, static_cast<byte>(fill));
  std::fill_n(bytes.begin(), 64, 0);
  bytes[0] = bytes[1] = order == littleEndian ? 'I' : 'M';
  us2Data(bytes.data() + 2, 42, order);
  ul2Data(bytes.data() + 4, 8, order);
  us2Data(bytes.data() + 8, 1, order);
  us2Data(bytes.data() + 10, 0x010e, order);  // ImageDescription.
  us2Data(bytes.data() + 12, 2, order);
  ul2Data(bytes.data() + 14, static_cast<uint32_t>(size - 64), order);
  ul2Data(bytes.data() + 18, 64, order);
  bytes.back() = 0;
  return bytes;
}

// Wrap TIFF slices in adjacent Exif APP1 segments, preserving TIFF-relative offsets.
Blob fragmented(const Blob& bytes, bool jpeg, size_t chunk = maxExifFragment) {
  Blob image = jpeg ? Blob{0xff, 0xd8} : Blob{0xff, 0x01, 'E', 'x', 'i', 'v', '2'};
  for (size_t offset = 0; offset < bytes.size(); offset += std::min(chunk, bytes.size() - offset)) {
    const auto count = std::min(chunk, bytes.size() - offset);
    Blob payload(exifId.begin(), exifId.end());
    payload.insert(payload.end(), bytes.begin() + offset, bytes.begin() + offset + count);
    segment(image, 0xe1, payload);
  }
  image.insert(image.end(), {0xff, 0xd9});
  return image;
}

// Own fixture bytes in MemIo so returned images remain valid for repeated reads.
Image::UniquePtr read(const Blob& bytes) {
  auto io = std::make_unique<MemIo>();
  io->write(bytes.data(), bytes.size());
  auto image = ImageFactory::open(std::move(io));
  image->readMetadata();
  return image;
}

// Own a uniquely created temporary directory for file-backed read/write regressions.
class TemporaryExifFile {
 public:
  // Reserve a directory without overwriting any existing file or fixture.
  TemporaryExifFile() {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    directory_ = std::filesystem::temp_directory_path() / ("exiv2-exif-" + std::to_string(stamp));
    if (!std::filesystem::create_directory(directory_))
      throw std::runtime_error("Cannot create Exif test directory");
  }

  // Remove only this fixture's directory after its assertions finish.
  ~TemporaryExifFile() {
    std::error_code error;
    std::filesystem::remove_all(directory_, error);
  }

  // Return the private synthetic fixture path.
  std::filesystem::path path() const {
    return directory_ / "fragmented.exv";
  }

 private:
  std::filesystem::path directory_;
};

// Simulate a short payload read while retaining ordinary marker/header behavior.
class ShortExifReadIo : public MemIo {
 public:
  using MemIo::read;

  // Store an owned fixture; no external byte buffer must outlive this test double.
  explicit ShortExifReadIo(const Blob& bytes) {
    write(bytes.data(), bytes.size());
  }

  // Return one byte less than requested for a full-sized Exif segment.
  size_t read(byte* buffer, size_t count) override {
    return MemIo::read(buffer, count > 60000 ? count - 1 : count);
  }
};

// Require the entire out-of-line value, rather than just a surviving tag count.
void expectDescription(const Image& image, size_t size, char fill = 'q') {
  const auto it = image.exifData().findKey(ExifKey("Exif.Image.ImageDescription"));
  ASSERT_NE(it, image.exifData().end());
  EXPECT_TRUE(it->toString() == std::string(size - 65, fill));
}

// Cover both container signatures, TIFF byte orders, and boundary placements.
class JpegExifContinuation : public testing::TestWithParam<std::tuple<bool, ByteOrder, size_t>> {};

// Reassemble values and directory fields split across two or more APP1 segments.
TEST_P(JpegExifContinuation, readsCompleteValue) {
  const auto [jpeg, order, chunk] = GetParam();
  const auto bytes = fragmented(tiff(order), jpeg, chunk);
  const auto image = read(bytes);
  expectDescription(*image, 70000);
  EXPECT_EQ(image->byteOrder(), order);
}

INSTANTIATE_TEST_SUITE_P(Boundaries, JpegExifContinuation,
                         testing::Combine(testing::Bool(), testing::Values(littleEndian, bigEndian),
                                          testing::Values(size_t{19}, size_t{23334}, maxExifFragment - 1,
                                                          maxExifFragment)));

// A complete ordinary Exif record must retain its existing read/write behavior.
TEST(JpegExifSegments, singleSegmentStillWritable) {
  for (const auto jpeg : {false, true}) {
    const auto source = read(fragmented(tiff(littleEndian, 100), jpeg));
    expectDescription(*source, 100);
    auto image = ImageFactory::create(jpeg ? ImageType::jpeg : ImageType::exv);
    image->setExifData(source->exifData());
    image->exifData()["Exif.Image.Artist"] = "single segment";
    ASSERT_NO_THROW(image->writeMetadata());
    image->readMetadata();
    EXPECT_EQ(image->exifData()["Exif.Image.Artist"].toString(), "single segment");
  }
}

// A fresh TIFF header starts an independent record, retaining first-record precedence.
TEST(JpegExifSegments, independentRecordsAreNotConcatenated) {
  auto bytes = fragmented(tiff(littleEndian, 100, 'a'), false);
  const auto second = fragmented(tiff(bigEndian, 100, 'b'), false);
  bytes.erase(bytes.end() - 2, bytes.end());
  bytes.insert(bytes.end(), second.begin() + 7, second.end());
  auto image = read(bytes);
  expectDescription(*image, 100, 'a');
  ASSERT_NO_THROW(image->writeMetadata());
}

// A continuation can end exactly on the maximum payload boundary without an empty tail.
TEST(JpegExifSegments, exactMultipleAndCompleteMaximumSegment) {
  for (const auto size : {maxExifFragment, 2 * maxExifFragment, 3 * maxExifFragment}) {
    const auto image = read(fragmented(tiff(bigEndian, size), true));
    expectDescription(*image, size);
  }
}

// Preserve every following metadata category and SOF dimensions after chain completion.
TEST(JpegExifSegments, processesTerminatingMarkersOnce) {
  auto bytes = fragmented(tiff(littleEndian), true);
  bytes.resize(bytes.size() - 2);
  segment(bytes, 0xfe, Blob{'a', 'f', 't', 'e', 'r'});
  segment(bytes, 0xc0, Blob{8, 0, 2, 0, 3, 1, 1, 0x11, 0});
  Blob icc{'I', 'C', 'C', '_', 'P', 'R', 'O', 'F', 'I', 'L', 'E', 0, 1, 1, 0, 0, 0, 8, 0, 0, 0, 0};
  segment(bytes, 0xe2, icc);
  bytes.insert(bytes.end(), {0xff, 0xda, 0, 2, 0x17, 0xff, 0, 0xe1});
  const auto image = read(bytes);
  expectDescription(*image, 70000);
  EXPECT_EQ(image->comment(), "after");
  EXPECT_EQ(image->pixelWidth(), 3);
  EXPECT_EQ(image->pixelHeight(), 2);
  ASSERT_EQ(image->iccProfile().size(), 8U);
  EXPECT_EQ(image->iccProfile().read_uint32(0, bigEndian), 8U);
}

// An unrelated marker ends the chain; later fragments cannot repair that TIFF.
TEST(JpegExifSegments, doesNotMergeAcrossComment) {
  auto bytes = fragmented(tiff(littleEndian), false);
  Blob comment;
  segment(comment, 0xfe, Blob{'g', 'a', 'p'});
  bytes.insert(bytes.begin() + 7 + 2 + 65535, comment.begin(), comment.end());
  const auto image = read(bytes);
  EXPECT_EQ(image->comment(), "gap");
  const auto it = image->exifData().findKey(ExifKey("Exif.Image.ImageDescription"));
  ASSERT_NE(it, image->exifData().end());
  EXPECT_LT(it->count(), 70000 - 64);
}

// Reject recognized continuation writes, including clearing, without touching MemIo bytes.
TEST(JpegExifSegments, rejectsMemoryWritesWithoutChangingSource) {
  for (const auto jpeg : {false, true}) {
    for (const auto clear : {false, true}) {
      const auto bytes = fragmented(tiff(littleEndian), jpeg);
      auto image = read(bytes);
      if (clear)
        image->clearMetadata();
      else
        image->exifData()["Exif.Image.Artist"] = "changed";

      EXPECT_THROW(image->writeMetadata(), Error);
      auto& io = image->io();
      ASSERT_EQ(io.open(), 0);
      const auto actual = io.read(io.size());
      EXPECT_EQ(Blob(actual.begin(), actual.end()), bytes);
      io.close();
    }
  }
}

// Repeated reads replace caller edits while reassembling the same complete Exif stream.
TEST(JpegExifSegments, repeatedReadResetsMetadata) {
  auto image = read(fragmented(tiff(bigEndian), false));
  image->exifData()["Exif.Image.Artist"] = "pending";
  image->setComment("pending");
  image->readMetadata();
  expectDescription(*image, 70000);
  EXPECT_EQ(image->exifData().findKey(ExifKey("Exif.Image.Artist")), image->exifData().end());
  EXPECT_TRUE(image->comment().empty());
}

// FileIo rejection must preserve the complete physical source as well as metadata values.
TEST(JpegExifSegments, fileReadsAndRejectedWritesPreserveBytes) {
  const TemporaryExifFile temporary;
  const auto path = temporary.path().string();
  const auto bytes = fragmented(tiff(bigEndian), false);
  {
    std::ofstream output(path, std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
  }
  auto image = ImageFactory::open(path);
  image->readMetadata();
  expectDescription(*image, 70000);

  image->clearMetadata();
  EXPECT_THROW(image->writeMetadata(), Error);
  std::ifstream input(path, std::ios::binary);
  EXPECT_TRUE(Blob(std::istreambuf_iterator<char>(input), {}) == bytes);
}

// Empty continuations, truncated payloads, and missing chain terminators fail before decoding.
TEST(JpegExifSegments, rejectsMalformedChains) {
  const auto good = fragmented(tiff(littleEndian), false);
  std::vector<Blob> malformed;
  auto empty = good;
  empty.resize(empty.size() - 2);
  segment(empty, 0xe1, Blob(exifId.begin(), exifId.end()));
  empty.insert(empty.end(), {0xff, 0xd9});
  malformed.push_back(std::move(empty));

  auto truncated = good;
  truncated.resize(truncated.size() - 10);
  malformed.push_back(std::move(truncated));
  auto noTerminator = good;
  noTerminator.resize(noTerminator.size() - 2);
  malformed.push_back(std::move(noTerminator));

  for (const auto length : {0, 1, 7}) {
    auto invalid = good;
    invalid.resize(invalid.size() - 2);
    invalid.insert(invalid.end(), {0xff, 0xe1, 0, static_cast<byte>(length), 'E', 'x'});
    malformed.push_back(std::move(invalid));
  }
  for (const auto& bytes : malformed) {
    auto image = ImageFactory::open(bytes.data(), bytes.size());
    EXPECT_THROW(image->readMetadata(), Error);
    EXPECT_TRUE(image->exifData().empty());
  }
}

// Cap tiny-fragment work independently of the aggregate byte limit.
TEST(JpegExifSegments, enforcesFragmentLimit) {
  const auto bytes = fragmented(tiff(littleEndian), false, 17);
  auto image = ImageFactory::open(bytes.data(), bytes.size());
  EXPECT_THROW(image->readMetadata(), Error);
  EXPECT_TRUE(image->exifData().empty());
}

// Bound aggregate allocation even when each individual APP1 length is valid.
TEST(JpegExifSegments, enforcesAggregateLimit) {
  const auto bytes = fragmented(tiff(littleEndian, 64 * 1024 * 1024 + 1), false);
  auto image = ImageFactory::open(bytes.data(), bytes.size());
  EXPECT_THROW(image->readMetadata(), Error);
  EXPECT_TRUE(image->exifData().empty());
}

// A continuation's first bytes may resemble TIFF when the writer chooses a different split.
TEST(JpegExifSegments, shortensBoundaryToAvoidIndependentTiffSignature) {
  auto bytes = tiff(littleEndian);
  const std::array<byte, 4> signature{'M', 'M', 0, 42};
  std::copy(signature.begin(), signature.end(), bytes.begin() + maxExifFragment);
  const auto image = read(fragmented(bytes, false, maxExifFragment - 1));
  const auto& value = image->exifData()["Exif.Image.ImageDescription"];
  Blob actual(value.size());
  value.copy(actual.data(), littleEndian);
  EXPECT_TRUE(actual == Blob(bytes.begin() + 64, bytes.end()));
}

// Earlier independent records cannot hide a continuation chain from the write preflight.
TEST(JpegExifSegments, rejectsLaterIndependentRecordWithContinuations) {
  auto bytes = fragmented(tiff(littleEndian, 100), false);
  bytes.resize(bytes.size() - 2);
  const auto later = fragmented(tiff(bigEndian), false);
  bytes.insert(bytes.end(), later.begin() + 7, later.end());
  auto image = read(bytes);
  expectDescription(*image, 100);
  EXPECT_THROW(image->writeMetadata(), Error);
}

// Complete every other search category before a final Exif chain, including IPTC/XMP.
TEST(JpegExifSegments, finalCategoryStillCollectsContinuations) {
  Blob bytes{0xff, 0xd8};
  segment(bytes, 0xfe, Blob{'c'});
  segment(bytes, 0xc0, Blob{8, 0, 2, 0, 3, 1, 1, 0x11, 0});
  segment(bytes, 0xe2, Blob{'I', 'C', 'C', '_', 'P', 'R', 'O', 'F', 'I', 'L', 'E', 0, 1, 1, 0, 0, 0, 8, 0, 0, 0, 0});

  const std::string packet =
      "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\"><rdf:RDF "
      "xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">"
      "<rdf:Description rdf:about=\"\" xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\" "
      "xmp:Label=\"following\"/></rdf:RDF></x:xmpmeta>";
  const std::string xmpId = "http://ns.adobe.com/xap/1.0/";
  Blob xmp(xmpId.begin(), xmpId.end());
  xmp.push_back(0);
  xmp.insert(xmp.end(), packet.begin(), packet.end());
  segment(bytes, 0xe1, xmp);

  IptcData iptc;
  iptc["Iptc.Application2.ObjectName"] = "press";
  const auto irb = Photoshop::setIptcIrb(nullptr, 0, iptc);
  const byte photoshopId[] = "Photoshop 3.0";
  Blob photoshop(photoshopId, photoshopId + sizeof(photoshopId));
  photoshop.insert(photoshop.end(), irb.begin(), irb.end());
  segment(bytes, 0xed, photoshop);

  const auto chain = fragmented(tiff(bigEndian), true);
  bytes.insert(bytes.end(), chain.begin() + 2, chain.end());
  for (const auto exifFirst : {false, true}) {
    auto fixture = bytes;
    if (exifFirst) {
      fixture = chain;
      fixture.resize(fixture.size() - 2);
      fixture.insert(fixture.end(), bytes.begin() + 2, bytes.end() - (chain.size() - 2));
      fixture.insert(fixture.end(), {0xff, 0xd9});
    }
    const auto image = read(fixture);
    expectDescription(*image, 70000);
    // The toolkit regenerates packet formatting; a toolkit-free read retains raw bytes.
#ifdef EXV_HAVE_XMP_TOOLKIT
    EXPECT_EQ(image->xmpData()["Xmp.xmp.Label"].toString(), "following");
#else
    EXPECT_EQ(image->xmpPacket(), packet);
#endif
    EXPECT_EQ(image->iptcData()["Iptc.Application2.ObjectName"].toString(), "press");
    EXPECT_EQ(image->comment(), "c");
    EXPECT_EQ(image->pixelWidth(), 3);
    EXPECT_EQ(image->iccProfile().size(), 8U);
  }
}

// Preserve raw MakerNote and JPEG thumbnail bytes spanning distinct TIFF extents.
TEST(JpegExifSegments, preservesMakerNoteAndThumbnail) {
  Blob bytes(70000, 0);
  bytes[0] = bytes[1] = 'I';
  us2Data(bytes.data() + 2, 42, littleEndian);
  ul2Data(bytes.data() + 4, 8, littleEndian);
  us2Data(bytes.data() + 8, 1, littleEndian);

  // Encode a scalar IFD entry at its TIFF-relative location.
  const auto entry = [&](size_t offset, uint16_t tag, uint16_t type, uint32_t count, uint32_t value) {
    us2Data(bytes.data() + offset, tag, littleEndian);
    us2Data(bytes.data() + offset + 2, type, littleEndian);
    ul2Data(bytes.data() + offset + 4, count, littleEndian);
    ul2Data(bytes.data() + offset + 8, value, littleEndian);
  };
  entry(10, 0x8769, 4, 1, 26);                   // Photo IFD.
  ul2Data(bytes.data() + 22, 44, littleEndian);  // Thumbnail IFD.
  us2Data(bytes.data() + 26, 1, littleEndian);
  entry(28, 0x927c, 7, 68000, 128);
  std::fill_n(bytes.begin() + 128, 68000, 'm');
  us2Data(bytes.data() + 44, 3, littleEndian);
  entry(46, 0x0103, 3, 1, 6);
  entry(58, 0x0201, 4, 1, 69992);
  entry(70, 0x0202, 4, 1, 8);
  const Blob thumbnail{0xff, 0xd8, 't', 'e', 's', 't', 0xff, 0xd9};
  std::copy(thumbnail.begin(), thumbnail.end(), bytes.begin() + 69992);

  const auto image = read(fragmented(bytes, false, 23332));
  const auto& note = image->exifData()["Exif.Photo.MakerNote"];
  Blob actual(note.size());
  note.copy(actual.data(), littleEndian);
  EXPECT_TRUE(actual == Blob(68000, 'm'));
  const auto copied = ExifThumbC(image->exifData()).copy();
  EXPECT_TRUE(Blob(copied.begin(), copied.end()) == thumbnail);
}

// A short underlying read must not decode or expose an incomplete continuation chain.
TEST(JpegExifSegments, rejectsShortPayloadRead) {
  auto image = ImageFactory::open(std::make_unique<ShortExifReadIo>(fragmented(tiff(littleEndian), false)));
  EXPECT_THROW(image->readMetadata(), Error);
  EXPECT_TRUE(image->exifData().empty());
}

// Accept exactly the fragment budget rather than rejecting its final permitted segment.
TEST(JpegExifSegments, acceptsFragmentLimit) {
  constexpr size_t size = 4096 * 19;
  const auto image = read(fragmented(tiff(littleEndian, size), false, 19));
  expectDescription(*image, size);
}

// Even the TIFF header may cross short segment boundaries; offsets still share one origin.
TEST(JpegExifSegments, splitsTiffHeaderAcrossFragments) {
  for (const auto order : {littleEndian, bigEndian}) {
    for (const auto chunk : {1, 3, 7, 13}) {
      const auto image = read(fragmented(tiff(order, 100), false, chunk));
      expectDescription(*image, 100);
    }
  }
}

// Accept the exact byte budget without making the decoded value itself enormous.
TEST(JpegExifSegments, acceptsAggregateLimit) {
  auto bytes = tiff(littleEndian, 64 * 1024 * 1024);
  ul2Data(bytes.data() + 14, 36, littleEndian);
  bytes[99] = 0;
  const auto image = read(fragmented(bytes, false));
  expectDescription(*image, 100);
}
}  // namespace
