// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffsample_int.hpp"
#include "cr3preview_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include <gtest/gtest.h>
#include <exiv2/basicio.hpp>
#include <exiv2/bmffimage.hpp>
#include <exiv2/error.hpp>
#include <exiv2/preview.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string_view>

using namespace Exiv2;
using namespace Exiv2::Internal;

namespace {
using Bytes = std::vector<byte>;

// Encode synthetic fields independently of the production cursors and table decoders.
void integer(Bytes& bytes, uint64_t value, unsigned width = 4) {
  for (unsigned i = width; i != 0; --i)
    bytes.push_back(static_cast<byte>(value >> ((i - 1) * 8)));
}

// Concatenate opaque fixture bytes without interpreting their structure.
void append(Bytes& bytes, const Bytes& suffix) {
  bytes.insert(bytes.end(), suffix.begin(), suffix.end());
}

// Frame ordinary and extended boxes independently of the production box reader.
Bytes box(std::string_view type, const Bytes& payload = {}, bool extended = false) {
  Bytes bytes;
  integer(bytes, extended ? 1 : payload.size() + 8);
  bytes.insert(bytes.end(), type.begin(), type.end());
  if (extended)
    integer(bytes, payload.size() + 16, 8);
  append(bytes, payload);
  return bytes;
}

// Assemble fixed-width table words without sharing production serialization code.
Bytes words(std::initializer_list<uint32_t> values) {
  Bytes bytes;
  for (auto value : values)
    integer(bytes, value);
  return bytes;
}

// Select physical encodings and Canon policy independently for generated fixtures.
struct Options {
  bool wide{true};
  bool variable{};
  bool extended{};
  bool mediaFirst{};
  bool external{};
  bool extraTrack{};
  bool extraMedia{};
  uint32_t descriptionIndex{1};
  uint16_t referenceIndex{1};
  uint16_t width{150};
  uint16_t height{91};
  std::string_view codec{"JPEG"};
  std::vector<std::string_view> extraCodecs;
};

// Construct the three standard singleton tables without requiring a brand or item model.
Bytes tables(uint64_t offset, uint32_t length, const Options& options = {}) {
  auto bytes = box("stsc", words({0, 1, 1, 1, options.descriptionIndex}), options.extended);
  auto sizes = words({0, options.variable ? 0 : length, 1});
  if (options.variable)
    integer(sizes, length);
  append(bytes, box("stsz", sizes, options.extended));
  auto chunks = words({0, 1});
  integer(chunks, offset, options.wide ? 8 : 4);
  append(bytes, box(options.wide ? "co64" : "stco", chunks, options.extended));
  return bytes;
}

// Read an existing small valid JPEG; extraction tests compare every original byte.
Bytes jpeg() {
  std::ifstream input(std::string(TESTDATA_PATH) + "/exiv2-empty.jpg", std::ios::binary);
  return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

// Build a Canon visual track, optionally preceding its referenced description with an opaque one.
Bytes track(uint64_t offset, uint32_t length, const Options& options = {}, bool metadata = false) {
  Bytes prefix(82);
  prefix[6] = static_cast<byte>(options.referenceIndex >> 8);
  prefix[7] = static_cast<byte>(options.referenceIndex);
  prefix[24] = static_cast<byte>(options.width >> 8);
  prefix[25] = static_cast<byte>(options.width);
  prefix[26] = static_cast<byte>(options.height >> 8);
  prefix[27] = static_cast<byte>(options.height);
  append(prefix, box(options.codec, words({0}), options.extended));
  for (const auto codec : options.extraCodecs)
    append(prefix, box(codec, words({0}), options.extended));
  auto descriptions = words({0, options.descriptionIndex});
  for (uint32_t i = 1; i < options.descriptionIndex; ++i)
    append(descriptions, box("junk"));
  append(descriptions, box("CRAW", prefix, options.extended));

  auto samples = box("stsd", descriptions, options.extended);
  append(samples, tables(offset, length, options));
  auto dref = words({0, options.referenceIndex});
  for (uint16_t i = 1; i < options.referenceIndex; ++i)
    append(dref, box("url ", words({0}), options.extended));
  append(dref, box("url ", words({options.external ? 0u : 1u}), options.extended));
  auto info = box("dinf", box("dref", dref, options.extended), options.extended);
  append(info, box("stbl", samples, options.extended));
  auto handler = words({0, 0});
  const std::string_view kind = metadata ? "meta" : "vide";
  handler.insert(handler.end(), kind.begin(), kind.end());
  auto media = box("hdlr", handler, options.extended);
  append(media, box("minf", info, options.extended));
  return box("trak", box("mdia", media, options.extended), options.extended);
}

// Place a single image inside mdat padding, with independent track and media ordering.
Bytes fixture(const Options& options = {}) {
  auto bytes = box("ftyp", {'c', 'r', 'x', ' ', 0, 0, 0, 0, 'c', 'r', 'x', ' '});
  if (options.extraMedia)
    append(bytes, box("mdat", Bytes(17, 42), options.extended));
  const auto image = jpeg();
  Bytes media(7, 42);
  append(media, image);
  append(media, Bytes(11, 43));
  const auto mdat = box("mdat", media, options.extended);
  const auto header = options.extended ? 16 : 8;
  auto tracks = track(0, static_cast<uint32_t>(image.size()), options);
  if (options.extraTrack)
    append(tracks, track(0, static_cast<uint32_t>(image.size()), options, true));
  const auto moovSize = box("moov", tracks, options.extended).size();
  const auto offset = bytes.size() + (options.mediaFirst ? 0 : moovSize) + header + 7;
  tracks = options.extraTrack ? track(offset, static_cast<uint32_t>(image.size()), options, true) : Bytes{};
  append(tracks, track(offset, static_cast<uint32_t>(image.size()), options));
  const auto moov = box("moov", tracks, options.extended);
  append(bytes, options.mediaFirst ? mdat : moov);
  append(bytes, options.mediaFirst ? moov : mdat);
  return bytes;
}

// Locate a generated field by its unique type, returning the payload's first byte.
size_t payload(const Bytes& bytes, std::string_view type) {
  const auto found = std::search(bytes.begin(), bytes.end(), type.begin(), type.end());
  EXPECT_NE(found, bytes.end());
  const auto position = static_cast<size_t>(found - bytes.begin());
  return position + 4;
}

// Mutate a fixed-width fixture field to exercise malformed-input handling.
void replace(Bytes& bytes, size_t position, uint64_t value, unsigned width = 4) {
  Bytes field;
  integer(field, value, width);
  ASSERT_LE(position + width, bytes.size());
  std::copy(field.begin(), field.end(), bytes.begin() + position);
}

// Preserve precise errors instead of allowing optional discovery to swallow actual I/O failures.
template <typename Operation>
void expectError(ErrorCode code, Operation operation) {
  try {
    operation();
    FAIL() << "Expected an Exiv2::Error";
  } catch (const Error& error) {
    EXPECT_EQ(error.code(), code);
  }
}

// Discover a preview in borrowed fixture bytes without entering the legacy metadata decoder.
std::optional<NativePreview> discover(const Bytes& bytes, size_t depth = 100) {
  MemIo io(bytes.data(), bytes.size());
  return findCr3JpegPreview(io, depth);
}

// Decode a standalone stbl, demonstrating that singleton mechanics need no image-format boxes.
std::optional<BmffSingleSample> sample(const Bytes& bytes) {
  MemIo io(bytes.data(), bytes.size());
  BmffReader reader(io, {100, 4096, 8});
  auto input = reader.cursor({0, bytes.size()});
  const auto table = reader.readBox(input);
  return readBmffSingleSample(reader, table, 1);
}

// Append a legacy Canon PRVW, leaving existing absolute track/sample coordinates untouched.
void addLegacyPreview(Bytes& bytes) {
  Bytes uuid{0xea, 0xf4, 0x2b, 0x5e, 0x1c, 0x98, 0x4b, 0x88, 0xb9, 0xfb, 0xb7, 0xdc, 0x40, 0x6e, 0x4d, 0x16};
  append(uuid, Bytes(8));
  append(uuid, box("PRVW", {0, 0, 0, 0, 0, 0, 0, 30, 0, 20, 0, 0, 0, 0, 0, 4, 0xff, 0xd8, 0xff, 0xd9}));
  append(bytes, box("uuid", uuid));
}

// Serve a multi-gigabyte virtual file while storing only headers and the JPEG signature.
class SparseIo : public MemIo {
 public:
  using MemIo::read;
  static constexpr uint64_t imageOffset = (uint64_t{1} << 33) + 123;

  // Retain only the small prefix; the mdat payload is represented by source coordinates.
  SparseIo() : prefix_(box("ftyp", {'c', 'r', 'x', ' ', 0, 0, 0, 0, 'c', 'r', 'x', ' '})) {
    append(prefix_, box("moov", track(imageOffset, 100)));
    const auto headerOffset = prefix_.size();
    integer(prefix_, 1);
    append(prefix_, {'m', 'd', 'a', 't'});
    integer(prefix_, imageOffset + 100 - headerOffset, 8);
  }

  //! @brief Expose the virtual file length without allocating compressed media.
  size_t size() const override {
    return static_cast<size_t>(imageOffset + 100);
  }

  //! @brief Report the virtual absolute input position.
  size_t tell() const override {
    return static_cast<size_t>(position_);
  }

  //! @brief Accept only in-range absolute seeks used by the shared reader.
  int seek(int64_t offset, Position origin) override {
    if (origin != beg || offset < 0 || static_cast<uint64_t>(offset) > size())
      return 1;
    position_ = static_cast<uint64_t>(offset);
    return 0;
  }

  //! @brief Serve headers and SOI only, recording any attempted image-buffer read.
  size_t read(byte* output, size_t count) override {
    largestRead = std::max(largestRead, count);
    if (position_ == imageOffset && count == 2) {
      output[0] = 0xff;
      output[1] = 0xd8;
    } else if (position_ <= prefix_.size() && count <= prefix_.size() - position_) {
      std::copy_n(prefix_.data() + position_, count, output);
    } else {
      unexpectedRead = true;
      return 0;
    }
    position_ += count;
    return count;
  }

  size_t largestRead{};
  bool unexpectedRead{};

 private:
  Bytes prefix_;
  uint64_t position_{};
};

// Inject a genuine failure specifically in the new optional track discovery path.
class FailingIo : public MemIo {
 public:
  using MemIo::read;

  // Borrow unchanged fixture storage and choose whether reads or seeks should fail.
  FailingIo(const Bytes& bytes, bool seekFailure) : MemIo(bytes.data(), bytes.size()), seekFailure_(seekFailure) {
  }

  //! @brief Fail all seeks when selected; malformed metadata must not mask this error.
  int seek(int64_t offset, Position origin) override {
    return seekFailure_ ? 1 : MemIo::seek(offset, origin);
  }

  //! @brief Return a short structural read in the read-failure variant.
  size_t read(byte* output, size_t count) override {
    return MemIo::read(output, count ? count - 1 : 0);
  }

 private:
  bool seekFailure_;
};
}  // namespace

// Standard singleton resolution works without ftyp, meta, Canon tags or JPEG bytes.
TEST(BmffSample, resolvesBothOffsetAndSizeEncodings) {
  for (const bool wide : {false, true}) {
    for (const bool variable : {false, true}) {
      Options options;
      options.wide = wide;
      options.variable = variable;
      options.descriptionIndex = 3;
      auto bytes = box("stbl", tables(8, 3, options));
      const auto actual = sample(bytes);
      ASSERT_TRUE(actual);
      EXPECT_EQ(actual->data, (BmffSpan{8, 3}));
      EXPECT_EQ(actual->descriptionIndex, 3u);
    }
  }
}

// Supported tables reject ambiguity, truncation, nonzero versions and invalid input ranges.
TEST(BmffSample, rejectsMalformedTables) {
  const auto original = box("stbl", tables(8, 3));
  for (const auto type : {"stsc", "stsz", "co64"}) {
    auto bytes = original;
    replace(bytes, payload(bytes, type), 0x01000000);
    expectError(ErrorCode::kerCorruptedMetadata, [&] { sample(bytes); });
    bytes = original;
    if (std::string_view(type) == "stsz")
      replace(bytes, payload(bytes, type) + 4, 0);
    replace(bytes, payload(bytes, type) + (std::string_view(type) == "stsz" ? 8 : 4), 0xffffffff);
    expectError(ErrorCode::kerCorruptedMetadata, [&] { sample(bytes); });
  }
  auto bytes = original;
  replace(bytes, payload(bytes, "co64") + 8, std::numeric_limits<uint64_t>::max(), 8);
  expectError(ErrorCode::kerCorruptedMetadata, [&] { sample(bytes); });
  bytes = tables(8, 3);
  append(bytes, box("stco", words({0, 1, 8})));
  expectError(ErrorCode::kerCorruptedMetadata, [&] { sample(box("stbl", bytes)); });
}

// Missing tables, zero-sized samples, multiple samples/chunks and compact sizes are unsupported.
TEST(BmffSample, omitsUnsupportedLayouts) {
  EXPECT_FALSE(sample(box("stbl")));
  Options variable;
  variable.variable = true;
  EXPECT_FALSE(sample(box("stbl", tables(8, 0, variable))));
  auto bytes = box("stbl", tables(8, 3));
  replace(bytes, payload(bytes, "stsz") + 8, 2);
  EXPECT_FALSE(sample(bytes));
  bytes = box("stbl", tables(8, 3));
  replace(bytes, payload(bytes, "stsc") + 12, 2);
  EXPECT_FALSE(sample(bytes));
  bytes = tables(8, 3);
  append(bytes, box("stz2", words({0, 0, 1})));
  EXPECT_FALSE(sample(box("stbl", bytes)));
}

// Ordinary and extended source headers, table encodings and physical orders yield exact bytes.
TEST(Cr3Preview, discoversAndExtractsThroughBothImageInterfaces) {
  for (const bool extended : {false, true}) {
    for (const bool wide : {false, true}) {
      Options options;
      options.extended = extended;
      options.wide = wide;
      options.variable = !wide;
      options.mediaFirst = wide;
      options.extraTrack = true;
      options.extraMedia = true;
      options.descriptionIndex = 2;
      options.referenceIndex = 2;
      const auto bytes = fixture(options);

      for (const bool factory : {false, true}) {
        Image::UniquePtr image;
        if (factory)
          image = ImageFactory::open(bytes.data(), bytes.size());
        else
          image = std::make_unique<BmffImage>(std::make_unique<MemIo>(bytes.data(), bytes.size()),
                                              ImageCtorParams(false, 100));
        ASSERT_NE(image, nullptr);
        ASSERT_NO_THROW(image->readMetadata());
        ASSERT_EQ(image->nativePreviews().size(), 1u);
        EXPECT_EQ(image->mimeType(), "image/x-canon-cr3");
        PreviewManager manager(*image);
        const auto properties = manager.getPreviewProperties();
        ASSERT_EQ(properties.size(), 1u);
        EXPECT_EQ(properties[0].width_, 150u);
        EXPECT_EQ(properties[0].height_, 91u);
        const auto extracted = manager.getPreviewImage(properties[0]);
        const auto expected = jpeg();
        ASSERT_EQ(extracted.size(), expected.size());
        EXPECT_TRUE(std::equal(expected.begin(), expected.end(), extracted.pData()));
      }
    }
  }
}

// Repeated reads and printing retain one current view, including replaced backing content.
TEST(Cr3Preview, replacesReadStateAndDoesNotAccumulateDuringPrinting) {
  auto bytes = fixture();
  addLegacyPreview(bytes);
  auto image = ImageFactory::open(bytes.data(), bytes.size());
  for (unsigned read = 0; read < 3; ++read) {
    image->readMetadata();
    ASSERT_EQ(image->nativePreviews().size(), 2u);
    EXPECT_EQ(image->nativePreviews()[0].width_, 30u);
    EXPECT_EQ(image->nativePreviews()[1].width_, 150u);
    std::ostringstream output;
    image->printStructure(output, kpsRecursive);
    ASSERT_EQ(image->nativePreviews().size(), 2u);
  }

  // The previous full-size range must disappear when a new CR3 has no JPEG track.
  Options options;
  options.codec = "CMP1";
  const auto replacement = fixture(options);
  MemIo source(replacement.data(), replacement.size());
  image->io().transfer(source);
  image->readMetadata();
  EXPECT_TRUE(image->nativePreviews().empty());
}

// Optional corrupt/unsupported tracks never become previews or break ordinary metadata reads.
TEST(Cr3Preview, omitsInvalidAndUnsupportedCandidates) {
  for (const auto codec : {"CMP1", "hvcC", "sinf", "free"}) {
    Options options;
    options.codec = codec;
    EXPECT_FALSE(discover(fixture(options)));
  }
  for (const auto codec : {"CMP1", "CDI1", "hvcC", "sinf", "JPEG"}) {
    Options options;
    options.extraCodecs = {codec};
    EXPECT_FALSE(discover(fixture(options)));
  }
  Options external;
  external.external = true;
  EXPECT_FALSE(discover(fixture(external)));
  Options empty;
  empty.width = 0;
  EXPECT_FALSE(discover(fixture(empty)));

  // Each mutation targets a different validation boundary of an otherwise valid container.
  const auto original = fixture();
  for (unsigned mutation = 0; mutation < 12; ++mutation) {
    auto bytes = original;
    switch (mutation) {
      case 0:
        replace(bytes, 8, bmffType("avif"));
        break;
      case 1:
        replace(bytes, payload(bytes, "stsz") + 8, 2);
        break;
      case 2:
        replace(bytes, payload(bytes, "stsc") + 16, 2);
        break;
      case 3:
        replace(bytes, payload(bytes, "CRAW") + 6, 2, 2);
        break;
      case 4:
        replace(bytes, payload(bytes, "co64") + 8, 8, 8);
        break;
      case 5:
        replace(bytes, payload(bytes, "co64") + 8, bytes.size() - 1, 8);
        break;
      case 6:
        replace(bytes, payload(bytes, "JPEG") - 8, 7);
        break;
      case 7:
        replace(bytes, payload(bytes, "stsc") + 4, 0xffffffff);
        break;
      case 8:
        replace(bytes, payload(bytes, "CRAW") - 8, 16);
        break;
      case 9:
        replace(bytes, payload(bytes, "stsd") + 4, 0xffffffff);
        break;
      case 10:
        replace(bytes, payload(bytes, "hdlr") + 8, bmffType("meta"));
        break;
      case 11:
        replace(bytes, payload(bytes, "mdat") + 7, 0, 2);
        break;
    }
    EXPECT_FALSE(discover(bytes)) << mutation;
    auto image = ImageFactory::open(bytes.data(), bytes.size());
    ASSERT_NO_THROW(image->readMetadata()) << mutation;
    EXPECT_TRUE(image->nativePreviews().empty()) << mutation;
  }
}

// Existing native loader slots are never displaced by the optional full-size candidate.
TEST(Cr3Preview, preservesExistingSlotsAndFallbackPreviews) {
  auto bytes = fixture();
  for (unsigned i = 0; i < 4; ++i)
    addLegacyPreview(bytes);
  auto image = ImageFactory::open(bytes.data(), bytes.size());
  image->readMetadata();
  ASSERT_EQ(image->nativePreviews().size(), 4u);
  for (const auto& preview : image->nativePreviews())
    EXPECT_EQ(preview.width_, 30u);

  Options options;
  options.external = true;
  bytes = fixture(options);
  addLegacyPreview(bytes);
  image = ImageFactory::open(bytes.data(), bytes.size());
  image->readMetadata();
  ASSERT_EQ(image->nativePreviews().size(), 1u);
  EXPECT_EQ(image->nativePreviews()[0].width_, 30u);
}

// Largest-area selection is independent of track numbering, with stable ties in file order.
TEST(Cr3Preview, selectsLargestTrackAndRetainsFileOrderTies) {
  for (const bool tie : {false, true}) {
    Options options;
    options.mediaFirst = true;
    auto bytes = fixture(options);
    const auto first = discover(bytes);
    ASSERT_TRUE(first);
    options.width = tie ? 91 : 300;
    options.height = tie ? 150 : 200;
    const auto next = track(first->position_, static_cast<uint32_t>(first->size_), options);
    const auto moovPayload = payload(bytes, "moov");
    const auto moovSize = bytes.size() - (moovPayload - 8);
    append(bytes, next);
    replace(bytes, moovPayload - 8, moovSize + next.size());

    const auto selected = discover(bytes);
    ASSERT_TRUE(selected);
    EXPECT_EQ(selected->width_, tie ? 150u : 300u);
    EXPECT_EQ(selected->height_, tie ? 91u : 200u);
  }
}

// Optional work respects depth/header budgets rather than buffering arbitrarily large media.
TEST(Cr3Preview, boundsDiscoveryWork) {
  EXPECT_FALSE(discover(fixture(), 6));
  auto bytes = fixture();
  for (unsigned i = 0; i < 4096; ++i)
    append(bytes, box("free"));
  EXPECT_FALSE(discover(bytes));
}

// Native source ranges above 4 GiB retain all offset bits and read only the JPEG signature.
TEST(Cr3Preview, resolvesSparseLargeOffsetsWithBoundedReads) {
  if (std::numeric_limits<size_t>::max() <= std::numeric_limits<uint32_t>::max())
    GTEST_SKIP() << "requires 64-bit source coordinates";
  SparseIo io;
  const auto preview = findCr3JpegPreview(io, 100);
  ASSERT_TRUE(preview);
  EXPECT_EQ(preview->position_, SparseIo::imageOffset);
  EXPECT_EQ(preview->size_, 100u);
  EXPECT_LE(io.largestRead, 8u);
  EXPECT_FALSE(io.unexpectedRead);
}

// Genuine seek/short-read failures propagate, unlike malformed optional track structure.
TEST(Cr3Preview, propagatesUnderlyingIoFailures) {
  const auto bytes = fixture();
  for (const bool seekFailure : {false, true}) {
    FailingIo io(bytes, seekFailure);
    expectError(ErrorCode::kerInputDataReadFailed, [&] { findCr3JpegPreview(io, 100); });
  }
}

#endif  // EXV_ENABLE_BMFF
