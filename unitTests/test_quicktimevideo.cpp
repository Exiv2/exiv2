// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>

#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {
using namespace Exiv2;

// Own stderr capture for one synchronous reader operation, including exceptions.
class NikonErrors {
 public:
  // Redirect library diagnostics into this fixture's owned stream.
  NikonErrors() : previous_(std::cerr.rdbuf(output_.rdbuf())) {
  }

  // Restore stderr before destroying its replacement stream.
  ~NikonErrors() {
    std::cerr.rdbuf(previous_);
  }

  // Return diagnostics emitted by the operation under test.
  std::string text() const {
    return output_.str();
  }

 private:
  std::ostringstream output_;
  std::streambuf* previous_;
};

// Append an explicitly big-endian field to a small synthetic Nikon record.
void number(Blob& bytes, uint32_t value, unsigned width) {
  for (unsigned i = width; i > 0; --i)
    bytes.push_back(static_cast<byte>(value >> ((i - 1) * 8)));
}

// Frame one atom without borrowing any camera-file contents.
Blob atom(const std::string& type, const Blob& contents) {
  Blob result;
  number(result, static_cast<uint32_t>(contents.size() + 8), 4);
  result.insert(result.end(), type.begin(), type.end());
  result.insert(result.end(), contents.begin(), contents.end());
  return result;
}

// Join complete records or atoms while retaining their encoded boundaries.
void append(Blob& bytes, const Blob& addition) {
  bytes.insert(bytes.end(), addition.begin(), addition.end());
}

// Encode the proprietary eight-byte NCTG tag/type/count header.
Blob record(uint32_t tag, uint16_t type, uint16_t count, const Blob& data) {
  Blob result;
  number(result, tag, 4);
  number(result, type, 2);
  number(result, count, 2);
  append(result, data);
  return result;
}

// Supply one recognized text tag for checking progress and neighboring-record ownership.
Blob modelRecord(const std::string& model) {
  Blob text(model.begin(), model.end());
  text.push_back(0);
  return record(2, 2, static_cast<uint16_t>(text.size()), text);
}

// Pad only the baseline-focused fixtures so its old 100-record loop can finish.
Blob padded(Blob records, size_t count) {
  for (; count < 100; ++count)
    append(records, record(0x90000000, 7, 1, Blob{0}));
  return records;
}

// Construct a metadata-only MP4 with an optional sibling after its NCTG atom.
Blob movie(const Blob& records, const Blob& sibling = {}) {
  Blob brand{'m', 'p', '4', '2', 0, 0, 0, 1, 'm', 'p', '4', '2'};
  auto result = atom("ftyp", brand);
  // Earlier factory probes inspect up to 80 bytes before the video probe.
  append(result, atom("free", Blob(128, 0)));
  auto children = atom("NCTG", records);
  append(children, sibling);
  append(result, atom("moov", atom("udta", atom("NCDT", children))));
  return result;
}

// Read through the public factory; tests need no access to private decoder methods.
Image::UniquePtr read(const Blob& bytes) {
  auto image = ImageFactory::open(bytes.data(), bytes.size());
  image->readMetadata();
  return image;
}

// Read a field without inserting a missing property into the actual result.
std::string field(const Image& image, const std::string& name) {
  const auto found = image.xmpData().findKey(XmpKey("Xmp.video." + name));
  return found == image.xmpData().end() ? "<absent>" : found->toString();
}

// Set one fixed-width camera text field with explicit padding and no trailing dependency.
void text(Blob& data, size_t start, const std::string& value) {
  std::fill_n(data.begin() + start, 20, 0);
  std::copy_n(value.begin(), std::min<size_t>(value.size(), 20), data.begin() + start);
}

// Build distinct legacy/modern Picture Control layouts with a recognizable prefix canary.
Blob picture(const std::string& version, const std::string& name = "N-LOG", const std::string& base = "BASE") {
  const bool modern = version.starts_with("03");
  Blob data(modern ? 108 : 76, 0x80);
  std::copy(version.begin(), version.end(), data.begin());
  if (modern)
    std::copy_n("PAD!", 4, data.begin() + 4);
  text(data, modern ? 8 : 4, name);
  text(data, modern ? 28 : 24, base);
  data[modern ? 54 : 48] = 2;
  if (version.starts_with("01")) {
    const std::array<byte, 9> controls{0, 0, 0, 0, 0, 0, 0, 0, 4};
    std::copy(controls.begin(), controls.end(), data.begin() + 49);
  }
  return data;
}

// Give a Picture Control packet enough unrelated records for comparison with the baseline.
Blob pictureMovie(const Blob& data) {
  return movie(padded(record(0x02000023, 7, static_cast<uint16_t>(data.size()), data), 1));
}
}  // namespace

// Signed SHORT scalars require one two-byte component, with proper negative values.
TEST(QuickTimeNikon, SignedShortScalars) {
  for (const int value : {-32768, -1, 0, 1, 32767}) {
    Blob data;
    number(data, static_cast<uint16_t>(value), 2);
    NikonErrors errors;
    const auto image = read(movie(padded(record(0x0200000b, 8, 1, data), 1)));
    EXPECT_EQ(std::to_string(value), field(*image, "WhiteBalanceFineTune"));
    EXPECT_TRUE(errors.text().empty()) << errors.text();
  }
}

// Multiple signed components remain a numeric list instead of becoming a decimal fraction.
TEST(QuickTimeNikon, SignedShortArray) {
  Blob data;
  for (const int value : {-32768, -1, 0, 32767})
    number(data, static_cast<uint16_t>(value), 2);
  NikonErrors errors;
  const auto image = read(movie(padded(record(0x0200000b, 8, 4, data), 1)));
  EXPECT_EQ("-32768 -1 0 32767", field(*image, "WhiteBalanceFineTune"));
  EXPECT_TRUE(errors.text().empty()) << errors.text();
}

// Unmapped signed records must not consume bytes belonging to the following recognized tag.
TEST(QuickTimeNikon, IgnoredSignedShortPreservesFollowingRecord) {
  auto records = record(0x1001, 8, 1, Blob{0, 0});
  append(records, modelRecord("after-signed"));
  NikonErrors errors;
  const auto image = read(movie(padded(records, 2)));
  EXPECT_EQ("after-signed", field(*image, "Model"));
  EXPECT_TRUE(errors.text().empty()) << errors.text();
}

// Record iteration ends at the payload boundary rather than an arbitrary count of 100.
TEST(QuickTimeNikon, PayloadBoundedRecordCounts) {
  for (const size_t count : {0, 1, 99, 100, 101, 129}) {
    Blob records;
    for (size_t i = 0; i < count; ++i)
      append(records, modelRecord(std::to_string(i)));
    NikonErrors errors;
    const auto image = read(movie(records));
    EXPECT_EQ(count == 0 ? "<absent>" : std::to_string(count - 1), field(*image, "Model"));
    EXPECT_TRUE(errors.text().empty()) << errors.text();
  }
}

// Zero components carry no value and must leave the next header untouched.
TEST(QuickTimeNikon, ZeroCountHasNoValue) {
  auto records = record(0x22, 3, 0, {});
  append(records, modelRecord("after-empty"));
  const auto image = read(movie(records));
  EXPECT_EQ("<absent>", field(*image, "FrameWidth"));
  EXPECT_EQ("after-empty", field(*image, "Model"));
}

// Known-width unmapped BYTE and signed-rational fields are skipped as complete records.
TEST(QuickTimeNikon, SkipsUnmappedFieldTypes) {
  auto records = record(0x90000001, 1, 1, Blob{0x80});
  append(records, record(0x90000002, 10, 1, Blob{0xff, 0xff, 0xff, 0xff, 0, 0, 0, 2}));
  append(records, modelRecord("after-unknown"));
  const auto image = read(movie(records));
  EXPECT_EQ("after-unknown", field(*image, "Model"));
}

// Count-times-width arithmetic must not wrap at 16 bits for a valid ignored value.
TEST(QuickTimeNikon, SkipsLargeRecordWithoutNarrowing) {
  auto records = record(0x90000001, 4, 65535, Blob(65535 * 4, 0));
  append(records, modelRecord("after-large"));
  const auto image = read(movie(records));
  EXPECT_EQ("after-large", field(*image, "Model"));
}

// Incomplete headers, unknown widths, and overstated counts reject inside the NCTG boundary.
TEST(QuickTimeNikon, RejectsMalformedRecords) {
  for (const Blob& malformed : {Blob(1, 0), Blob(4, 0), Blob(7, 0), record(2, 99, 1, Blob{0}),
                                record(2, 2, 100, Blob{0}), record(0x0200000b, 8, 2, Blob{0, 1})}) {
    EXPECT_THROW(read(movie(malformed, atom("NCTH", Blob(1024, 0)))), Error);
  }
}

// A sibling atom cannot be mistaken for further NCTG records or overwrite a good result.
TEST(QuickTimeNikon, DoesNotReadSiblingAtom) {
  const auto sibling = atom("NCTH", padded(modelRecord("foreign-canary"), 1));
  const auto image = read(movie(modelRecord("owned-value"), sibling));
  EXPECT_EQ("owned-value", field(*image, "Model"));
}

// A legacy numeric LensInfo fallback must not replace a descriptive LensModel.
TEST(QuickTimeNikon, PreservesDescriptiveLensName) {
  const Blob lensInfo{0, 0, 0, 50, 0, 0, 0, 1};
  for (const bool nameFirst : {false, true}) {
    const auto name = record(0x0110a434, 2, 6, Blob{'N', 'I', 'K', 'O', 'N', 0});
    auto records = nameFirst ? name : record(0x02000084, 5, 1, lensInfo);
    append(records, nameFirst ? record(0x02000084, 5, 1, lensInfo) : name);
    EXPECT_EQ("NIKON", field(*read(movie(records)), "LensModel"));
  }
  EXPECT_EQ("50", field(*read(movie(record(0x02000084, 5, 1, lensInfo))), "LensModel"));
}

// The original supported Picture Control family retains its text and adjustment meanings.
TEST(QuickTimeNikon, LegacyPictureControl) {
  const auto image = read(pictureMovie(picture("0100", "STANDARD", "NEUTRAL")));
  EXPECT_EQ("0100", field(*image, "PictureControlVersion"));
  EXPECT_EQ("STANDARD", field(*image, "PictureControlName"));
  EXPECT_EQ("NEUTRAL", field(*image, "PictureControlBase"));
  EXPECT_EQ("Full Control", field(*image, "PictureControlAdjust"));
  EXPECT_EQ("Normal", field(*image, "Sharpness"));
  EXPECT_EQ("4", field(*image, "ToningSaturation"));
}
