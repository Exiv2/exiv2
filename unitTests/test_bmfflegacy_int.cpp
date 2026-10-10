// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/bmffimage.hpp>

#ifdef EXV_ENABLE_BMFF

#include <gtest/gtest.h>
#include <exiv2/basicio.hpp>
#include <exiv2/error.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <sstream>
#include <string_view>
#include <vector>

using namespace Exiv2;

namespace {
using Bytes = std::vector<byte>;
const Bytes canonUuid{0x85, 0xc0, 0xb6, 0x87, 0x82, 0x0f, 0x11, 0xe0, 0x81, 0x11, 0xf4, 0xce, 0x46, 0x2b, 0x6a, 0x48};
const Bytes previewUuid{0xea, 0xf4, 0x2b, 0x5e, 0x1c, 0x98, 0x4b, 0x88, 0xb9, 0xfb, 0xb7, 0xdc, 0x40, 0x6e, 0x4d, 0x16};

// Encode fixture fields independently of the shared parser.
void integer(Bytes& bytes, uint64_t value, unsigned width) {
  for (unsigned i = width; i != 0; --i)
    bytes.push_back(static_cast<byte>(value >> ((i - 1) * 8)));
}

// Concatenate source bytes without interpreting child boxes or packet contents.
void append(Bytes& bytes, const Bytes& suffix) {
  bytes.insert(bytes.end(), suffix.begin(), suffix.end());
}

// Construct ordinary, extended or EOF-sized boxes for compatibility fixtures.
Bytes box(std::string_view type, const Bytes& payload = {}, bool extended = false, bool terminal = false) {
  Bytes bytes;
  integer(bytes, terminal ? 0 : extended ? 1 : payload.size() + 8, 4);
  bytes.insert(bytes.end(), type.begin(), type.end());
  if (extended)
    integer(bytes, payload.size() + 16, 8);
  append(bytes, payload);
  return bytes;
}

// Give the legacy probe a brand without requiring HEIF item-model admission.
Bytes file(std::string_view brand = "avif") {
  Bytes payload(brand.begin(), brand.end());
  append(payload, Bytes(4));
  payload.insert(payload.end(), brand.begin(), brand.end());
  return box("ftyp", payload);
}

// Open a read-only legacy view that borrows fixture storage until destruction.
std::unique_ptr<BmffImage> legacy(const Bytes& bytes, size_t depth = 100) {
  return std::make_unique<BmffImage>(std::make_unique<MemIo>(bytes.data(), bytes.size()),
                                     ImageCtorParams(false, depth));
}

// Assert the precise error so hardening does not hide unrelated parsing failures.
template <typename Operation>
void expectError(ErrorCode expected, Operation operation) {
  try {
    operation();
    FAIL() << "Expected an Exiv2::Error";
  } catch (const Error& error) {
    EXPECT_EQ(error.code(), expected);
  }
}

// Build a minimal little-endian TIFF with one out-of-line Artist value.
Bytes artistTiff(std::string_view artist) {
  Bytes bytes{'I', 'I', 42, 0, 8, 0, 0, 0, 1, 0, 0x3b, 1, 2, 0};
  const auto size = static_cast<uint32_t>(artist.size() + 1);
  for (unsigned i = 0; i < 4; ++i)
    bytes.push_back(static_cast<byte>(size >> (i * 8)));
  append(bytes, {26, 0, 0, 0, 0, 0, 0, 0});
  bytes.insert(bytes.end(), artist.begin(), artist.end());
  bytes.push_back(0);
  return bytes;
}

// Encode disjoint and overlapping properties to check the legacy XML replacement order.
Bytes packet(std::string_view label, bool extra) {
  const auto xml = std::string(
                       "<x:xmpmeta xmlns:x='adobe:ns:meta/'><rdf:RDF "
                       "xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>"
                       "<rdf:Description rdf:about='' xmlns:xmp='http://ns.adobe.com/xap/1.0/' xmp:Label='") +
                   std::string(label) + "'" + (extra ? " xmp:Nickname='first-only'" : "") + "/></rdf:RDF></x:xmpmeta>";
  return Bytes(xml.begin(), xml.end());
}

// Inject one short underlying read without changing declared file boundaries.
class ShortReadIo : public MemIo {
 public:
  using MemIo::read;

  // Borrow source bytes and select the absolute read position to fail.
  ShortReadIo(const Bytes& bytes, size_t offset) : MemIo(bytes.data(), bytes.size()), offset_(offset) {
  }

  //! @brief Return one byte fewer only at the selected header or payload read.
  size_t read(byte* output, size_t count) override {
    if (tell() == offset_ && count != 0) {
      injected = true;
      return MemIo::read(output, count - 1);
    }
    return MemIo::read(output, count);
  }

  bool injected{};

 private:
  size_t offset_;
};

// Signal seek or stream errors after otherwise successful positioning or reading.
class ErrorIo : public MemIo {
 public:
  using MemIo::read;

  // Borrow the fixture and select which underlying failure to expose.
  ErrorIo(const Bytes& bytes, bool seekFailure) : MemIo(bytes.data(), bytes.size()), seekFailure_(seekFailure) {
  }

  //! @brief Let the seek reach its target while reporting a failure the reader must honor.
  int seek(int64_t offset, Position origin) override {
    const auto result = MemIo::seek(offset, origin);
    if (seekFailure_ && offset == 28 && origin == beg) {
      injected = true;
      return 1;
    }
    return result;
  }

  //! @brief Signal a stream error after returning a complete opaque payload.
  size_t read(byte* output, size_t count) override {
    if (!seekFailure_ && tell() == 28)
      injected = true;
    return MemIo::read(output, count);
  }

  //! @brief Preserve the injected stream error until the read is rejected.
  int error() const override {
    return !seekFailure_ && injected ? 1 : MemIo::error();
  }

  bool injected{};

 private:
  bool seekFailure_;
};

// Represent a large mdat without allocating or serving its opaque image bytes.
class SparseLegacyIo : public MemIo {
 public:
  using MemIo::read;
  static constexpr uint64_t tailOffset = (uint64_t{1} << 33) + 20;

  // Store only the file type, extended mdat header, and final free box.
  SparseLegacyIo() : prefix_(file()), tail_(box("free")) {
    integer(prefix_, 1, 4);
    append(prefix_, {'m', 'd', 'a', 't'});
    integer(prefix_, tailOffset - 20, 8);
  }

  //! @brief Rewind the virtual input when BmffImage opens it.
  int open() override {
    position_ = 0;
    return 0;
  }

  //! @brief Expose the virtual file length without materializing the media payload.
  size_t size() const override {
    return static_cast<size_t>(tailOffset + tail_.size());
  }

  //! @brief Report the absolute position used by legacy preview and traversal code.
  size_t tell() const override {
    return static_cast<size_t>(position_);
  }

  //! @brief Permit only absolute seeks within the synthetic file.
  int seek(int64_t offset, Position origin) override {
    if (origin != beg || offset < 0 || static_cast<uint64_t>(offset) > size())
      return 1;
    position_ = static_cast<uint64_t>(offset);
    return 0;
  }

  //! @brief Serve stored headers and detect any accidental media-payload access.
  size_t read(byte* output, size_t count) override {
    const auto& region = position_ >= tailOffset ? tail_ : prefix_;
    const auto relative = position_ >= tailOffset ? position_ - tailOffset : position_;
    if (relative > region.size() || count > region.size() - relative) {
      forbiddenRead = true;
      return 0;
    }
    std::copy_n(region.begin() + relative, count, output);
    position_ += count;
    return count;
  }

  bool forbiddenRead{};

 private:
  Bytes prefix_;
  Bytes tail_;
  uint64_t position_{};
};
}  // namespace

// Unknown payloads stay opaque and structure output retains original encoded size fields.
TEST(BmffLegacy, retainsExtendedTerminalAndOpaqueStructure) {
  auto bytes = file();
  append(bytes, box("skip", {0xff, 0xff, 0xff}, true));
  append(bytes, box("moov", box("free", {}, false, true)));
  auto image = legacy(bytes);
  ASSERT_NO_THROW(image->readMetadata());
  EXPECT_EQ(image->mimeType(), "image/avif");

  for (const auto mode : {kpsBasic, kpsRecursive}) {
    std::ostringstream output;
    image->printStructure(output, mode, 0);
    EXPECT_EQ(output.str(),
              "Exiv2::BmffImage::boxHandler: ftyp        0->20 brand: avif\n"
              "Exiv2::BmffImage::boxHandler: skip       20->1 \n"
              "Exiv2::BmffImage::boxHandler: moov       39->16 \n"
              "  Exiv2::BmffImage::boxHandler: free       47->0 \n");
  }
}

// Counted iinf traversal keeps the legacy 16-bit count and ignores unclaimed trailing bytes.
TEST(BmffLegacy, retainsCountedInfoCompatibility) {
  for (const byte version : {0, 1}) {
    Bytes info{version, 0, 0, 0, 0, 1};
    append(info, box("free"));
    append(info, {0xff});
    auto bytes = file();
    append(bytes, box("iinf", info));
    auto image = legacy(bytes);
    EXPECT_NO_THROW(image->readMetadata());
  }
}

// A count that claims a missing entry remains corruption, rather than an empty table.
TEST(BmffLegacy, rejectsMissingCountedEntry) {
  auto bytes = file();
  append(bytes, box("iinf", {0, 0, 0, 0, 0, 1}));
  auto image = legacy(bytes);
  expectError(ErrorCode::kerCorruptedMetadata, [&] { image->readMetadata(); });
}

// Canon containers retain TIFF interpretation, preview origins, and repeated-read behavior.
TEST(BmffLegacy, readsCanonMetadataAndPrefixedPreviews) {
  for (const bool extended : {false, true}) {
    auto bytes = file("crx ");
    auto metadata = canonUuid;
    append(metadata, box("CMT1", artistTiff("Canon artist")));
    append(bytes, box("uuid", metadata, extended));

    auto previews = previewUuid;
    append(previews, Bytes(8));
    const Bytes preview{0, 0, 0, 0, 0, 0, 0, 30, 0, 20, 0, 0, 0, 0, 0, 4, 0xff, 0xd8, 0xff, 0xd9};
    append(previews, box("PRVW", preview));
    const auto previewPosition = bytes.size() + (extended ? 32 : 24) + 8 + 8 + 16;
    append(bytes, box("uuid", previews, extended));
    auto image = legacy(bytes);

    for (unsigned read = 0; read < 2; ++read) {
      ASSERT_NO_THROW(image->readMetadata());
      EXPECT_EQ(image->mimeType(), "image/x-canon-cr3");
      EXPECT_EQ(image->exifData()["Exif.Image.Artist"].toString(), "Canon artist");
      ASSERT_EQ(image->nativePreviews().size(), read + 1);
      const auto& actual = image->nativePreviews().back();
      EXPECT_EQ(actual.position_, previewPosition);
      EXPECT_EQ(actual.size_, 4u);
      EXPECT_EQ(actual.width_, 30u);
      EXPECT_EQ(actual.height_, 20u);
      EXPECT_EQ(actual.mimeType_, "image/jpeg");
    }
  }
}

// Last XML and ICC boxes retain legacy precedence without accumulating structured XMP.
TEST(BmffLegacy, retainsDuplicateMetadataAndProfilePrecedence) {
  auto bytes = box("JXL ", {13, 10, 135, 10});
  append(bytes, file("jxl "));
  append(bytes, box("xml ", packet("first", true)));
  append(bytes, box("xml ", packet("last", false)));
  Bytes expected;
  for (const byte marker : {1, 2}) {
    Bytes profile(128);
    profile[3] = 128;
    profile.back() = marker;
    Bytes colour{'p', 'r', 'o', 'f'};
    append(colour, profile);
    append(bytes, box("colr", colour));
    expected = profile;
  }
  auto image = legacy(bytes);

  for (unsigned read = 0; read < 2; ++read) {
    ASSERT_NO_THROW(image->readMetadata());
    EXPECT_EQ(image->mimeType(), "image/jxl");
    ASSERT_EQ(image->iccProfile().size(), expected.size());
    EXPECT_EQ(std::memcmp(image->iccProfile().c_data(), expected.data(), expected.size()), 0);
#ifdef EXV_HAVE_XMP_TOOLKIT
    EXPECT_EQ(image->xmpData()["Xmp.xmp.Label"].toString(), "last");
    EXPECT_EQ(image->xmpData().findKey(XmpKey("Xmp.xmp.Nickname")), image->xmpData().end());
#else
    EXPECT_TRUE(image->xmpData().empty());
#endif
  }
}

// Compressed JPEG XL metadata follows the same feature gates and box order as before migration.
TEST(BmffLegacy, preservesOptionalBrotliMetadataReading) {
  // Fixed Brotli streams contain an Artist TIFF and an XMP Label packet; no encoder is needed at runtime.
  const Bytes exif{0x45, 0x78, 0x69, 0x66, 0x8b, 0x15, 0x80, 0x00, 0x00, 0x00, 0x00, 0x49, 0x49,
                   0x2a, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x00, 0x3b, 0x01, 0x02, 0x00, 0x0e,
                   0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x72,
                   0x6f, 0x74, 0x6c, 0x69, 0x20, 0x61, 0x72, 0x74, 0x69, 0x73, 0x74, 0x00, 0x03};
  const Bytes xmp{0x78, 0x6d, 0x6c, 0x20, 0x1b, 0xd9, 0x00, 0x00, 0x64, 0x74, 0x5b, 0xaa, 0xe0, 0xe9, 0x1a, 0x35,
                  0x31, 0x12, 0x89, 0xd0, 0xae, 0xc9, 0x10, 0xb5, 0xd9, 0xa1, 0x16, 0x00, 0x67, 0x81, 0x04, 0xb7,
                  0xd5, 0x16, 0x68, 0x4b, 0xa2, 0xb6, 0x27, 0x7a, 0x83, 0xf4, 0x86, 0xcf, 0xc3, 0x9a, 0x4d, 0x3e,
                  0x29, 0x45, 0x7c, 0xec, 0x4b, 0xdb, 0x85, 0xc6, 0xfe, 0x15, 0x15, 0x9f, 0x1c, 0xe7, 0x35, 0x59,
                  0x3e, 0x59, 0xea, 0x42, 0xa3, 0x12, 0x87, 0x0b, 0x6c, 0x9d, 0x66, 0xd4, 0x77, 0xb8, 0x20, 0xc7,
                  0xe0, 0xb5, 0x48, 0x60, 0x98, 0x22, 0x4a, 0xc1, 0x05, 0xce, 0x3f, 0x5f, 0xfa, 0x03, 0xf9, 0x6c,
                  0x49, 0xb6, 0x41, 0xad, 0x62, 0xa2, 0xd0, 0xe5, 0x38, 0xd6, 0xb6, 0xb4, 0x31, 0x23, 0x9f, 0x50,
                  0x87, 0x74, 0x76, 0x7c, 0x7a, 0x43, 0x04, 0x62, 0x04, 0xa6, 0x1b, 0x5b, 0xb4, 0xf1, 0x11, 0xb6,
                  0xa4, 0x7e, 0x74, 0xef, 0x46, 0xa4, 0x04, 0x12, 0x19, 0x4a, 0x20, 0x4b, 0xaa, 0x00};
  auto bytes = box("JXL ", {13, 10, 135, 10});
  append(bytes, file("jxl "));
  append(bytes, box("brob", exif));
  append(bytes, box("brob", xmp));
  auto image = legacy(bytes);

  ASSERT_NO_THROW(image->readMetadata());
  EXPECT_EQ(image->mimeType(), "image/jxl");
#ifdef EXV_HAVE_BROTLI
  EXPECT_EQ(image->exifData()["Exif.Image.Artist"].toString(), "Brotli artist");
#ifdef EXV_HAVE_XMP_TOOLKIT
  EXPECT_EQ(image->xmpData()["Xmp.xmp.Label"].toString(), "compressed");
#else
  EXPECT_TRUE(image->xmpData().empty());
#endif
#else
  EXPECT_TRUE(image->exifData().empty());
  EXPECT_TRUE(image->xmpData().empty());
#endif
}

// Legacy depth limits belong to the adapter; the shared reader must not import HEIF budgets.
TEST(BmffLegacy, preservesDepthLimitsAndSkipsLargeMediaRanges) {
  auto bytes = file();
  append(bytes, box("moov", box("free")));
  auto shallow = legacy(bytes, 1);
  expectError(ErrorCode::kerCorruptedMetadata, [&] { shallow->readMetadata(); });
  auto deep = legacy(bytes, 2);
  EXPECT_NO_THROW(deep->readMetadata());

  if (sizeof(size_t) < 8)
    GTEST_SKIP() << "BasicIo cannot represent the sparse 8 GiB input on this host";
  auto io = std::make_unique<SparseLegacyIo>();
  const auto* probe = io.get();
  BmffImage large(std::move(io), ImageCtorParams(false, 100));
  EXPECT_NO_THROW(large.readMetadata());
  EXPECT_FALSE(probe->forbiddenRead);
  EXPECT_EQ(large.mimeType(), "image/avif");
}

// A size-zero nested box cannot terminate at a parent boundary before the actual EOF.
TEST(BmffLegacyHardening, rejectsNestedTerminalBoxBeforeEof) {
  auto bytes = file();
  append(bytes, box("moov", box("free", {}, false, true)));
  append(bytes, box("free", Bytes(24)));
  auto image = legacy(bytes);
  expectError(ErrorCode::kerCorruptedMetadata, [&] { image->readMetadata(); });
}

// UUID headers must own their complete user type instead of borrowing bytes from a sibling.
TEST(BmffLegacyHardening, rejectsUuidSmallerThanItsUserType) {
  for (const bool extended : {false, true}) {
    auto bytes = file();
    append(bytes, box("uuid", {}, extended));
    append(bytes, box("free", Bytes(32)));
    auto image = legacy(bytes);
    expectError(ErrorCode::kerCorruptedMetadata, [&] { image->readMetadata(); });
  }
}

// Canon's preview prefix must fit inside its UUID box before child traversal can begin.
TEST(BmffLegacyHardening, rejectsTruncatedCanonPreviewPrefix) {
  auto bytes = file("crx ");
  append(bytes, box("uuid", previewUuid));
  append(bytes, box("free", Bytes(24)));
  auto image = legacy(bytes);
  expectError(ErrorCode::kerCorruptedMetadata, [&] { image->readMetadata(); });
}

// Underlying short reads must not silently discard a subtree or use zero-filled payload bytes.
TEST(BmffLegacyHardening, rejectsShortHeaderAndPayloadReads) {
  for (const size_t offset : {20u, 28u}) {
    auto bytes = file();
    append(bytes, box("free", Bytes(32)));
    auto io = std::make_unique<ShortReadIo>(bytes, offset);
    const auto* probe = io.get();
    BmffImage image(std::move(io), ImageCtorParams(false, 100));

    expectError(ErrorCode::kerInputDataReadFailed, [&] { image.readMetadata(); });
    EXPECT_TRUE(probe->injected);
  }
}

// Failed seeks and stream errors cannot be ignored merely because bytes remain readable.
TEST(BmffLegacyHardening, rejectsFailedSeekAndStreamError) {
  for (const bool seekFailure : {false, true}) {
    auto bytes = file();
    append(bytes, box("free", Bytes(32)));
    auto io = std::make_unique<ErrorIo>(bytes, seekFailure);
    const auto* probe = io.get();
    BmffImage image(std::move(io), ImageCtorParams(false, 100));

    expectError(ErrorCode::kerInputDataReadFailed, [&] { image.readMetadata(); });
    EXPECT_TRUE(probe->injected);
  }
}

#endif  // EXV_ENABLE_BMFF
