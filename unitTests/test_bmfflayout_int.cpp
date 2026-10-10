// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffitemwrite_int.hpp"
#include "bmfflayout_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include <gtest/gtest.h>
#include <exiv2/basicio.hpp>
#include <exiv2/error.hpp>

#include <algorithm>
#include <limits>

using namespace Exiv2;
using namespace Exiv2::Internal;

namespace {
using Bytes = std::vector<byte>;
constexpr uint64_t max32 = std::numeric_limits<uint32_t>::max();
constexpr uint64_t maxPosition = std::numeric_limits<int64_t>::max();

// Construct a source-backed output node without format recognition or item tables.
BmffOutputBox sourceBox(uint32_t type, BmffSpan source) {
  BmffOutputBox box;
  box.type = type;
  box.segments.push_back({source});
  return box;
}

// Snapshot the complete contents of a small memory stream for byte comparisons.
Bytes contents(MemIo& io) {
  return Bytes(io.mmap(), io.mmap() + io.size());
}

// Distinguish layout corruption from a stream's read/write failures.
template <typename Operation>
void expectError(ErrorCode code, Operation operation) {
  try {
    operation();
    FAIL() << "Expected an Exiv2::Error";
  } catch (const Error& error) {
    EXPECT_EQ(error.code(), code);
  }
}

// Count requests and inject stream failures while retaining ordinary memory storage.
class TestIo : public MemIo {
 public:
  using MemIo::read;
  using MemIo::write;

  // Start with an empty output stream.
  TestIo() = default;

  // Borrow stable fixture storage as the input stream.
  explicit TestIo(const Bytes& bytes) : MemIo(bytes.data(), bytes.size()) {
  }

  bool shortRead{};
  bool shortWrite{};
  bool failSeek{};
  bool reportError{};
  size_t largestRead{};
  size_t largestWrite{};
  size_t writes{};

  //! @brief Observe the largest read and optionally return one byte fewer.
  size_t read(byte* output, size_t count) override {
    largestRead = std::max(largestRead, count);
    return MemIo::read(output, shortRead && count ? count - 1 : count);
  }

  //! @brief Observe writes and optionally stop before the requested buffer is complete.
  size_t write(const byte* data, size_t count) override {
    largestWrite = std::max(largestWrite, count);
    ++writes;
    return MemIo::write(data, shortWrite && count ? count - 1 : count);
  }

  //! @brief Inject a seek failure independently of returned read counts.
  int seek(int64_t offset, Position origin) override {
    return failSeek ? 1 : MemIo::seek(offset, origin);
  }

  //! @brief Inject an error flag even after a full-sized transfer.
  int error() const override {
    return reportError ? 1 : MemIo::error();
  }
};
}  // namespace

// Preserve opaque bytes and caller-defined prefixes in a non-image container.
TEST(BmffLayout, writesPrefixedContainersAndOpaqueSegmentsWithoutImageMarkers) {
  const Bytes source{0, 0, 0, 1, 'b', 'a', 'd', '!', 9, 10};
  const Bytes replacement{11, 12};
  MemIo input(source.data(), source.size()), output;
  BmffOutputBox parent;
  parent.type = bmffType("nest");
  parent.prefix = {42};
  parent.children.push_back(sourceBox(bmffType("data"), {0, 8}));
  parent.segments = {{{8, 2}}, {{}, &replacement}};
  std::vector<BmffOutputBox> boxes{parent};

  EXPECT_EQ(layoutBmff(boxes), 29u);
  EXPECT_EQ(boxes[0].children[0].position, 9u);
  EXPECT_EQ(boxes[0].segments[0].position, 25u);
  writeBmffLayout(input, output, boxes);
  EXPECT_NO_THROW(verifyBmffLayout(input, output, boxes));

  const Bytes expected{0,   0,   0, 29, 'n', 'e', 's', 't', 42,  0,   0, 0,  16, 'd', 'a',
                       't', 'a', 0, 0,  0,   1,   'b', 'a', 'd', '!', 9, 10, 11, 12};
  EXPECT_EQ(contents(output), expected);
  EXPECT_EQ(contents(input), source);
}

// Encode both UUID header forms and retain an explicitly extended small box.
TEST(BmffLayout, writesOrdinaryAndExtendedUuidHeaders) {
  MemIo input, output;
  BmffOutputBox box;
  box.type = bmffType("uuid");
  for (size_t i = 0; i < box.uuid.size(); ++i)
    box.uuid[i] = static_cast<byte>(i);
  std::vector<BmffOutputBox> boxes{box, box};
  boxes[1].extended = true;

  EXPECT_EQ(layoutBmff(boxes), 56u);
  writeBmffLayout(input, output, boxes);
  EXPECT_NO_THROW(verifyBmffLayout(input, output, boxes));

  const Bytes expected{0,  0,  0,  24, 'u', 'u', 'i', 'd', 0, 1,   2,   3,   4,   5,  6,  7,  8,  9, 10,
                       11, 12, 13, 14, 15,  0,   0,   0,   1, 'u', 'u', 'i', 'd', 0,  0,  0,  0,  0, 0,
                       0,  32, 0,  1,  2,   3,   4,   5,   6, 7,   8,   9,   10,  11, 12, 13, 14, 15};
  EXPECT_EQ(contents(output), expected);
}

// Exercise exact size boundaries and nested promotion without allocating large payloads.
TEST(BmffLayout, promotesHeadersAndPlacesSparseRangesBeyondFourGiB) {
  for (const auto type : {bmffType("data"), bmffType("uuid")}) {
    const uint64_t shortHeader = type == bmffType("uuid") ? 24 : 8;
    std::vector<BmffOutputBox> boxes{sourceBox(type, {0, max32 - shortHeader})};
    EXPECT_EQ(layoutBmff(boxes), max32);
    EXPECT_FALSE(boxes[0].extended);

    ++boxes[0].segments[0].source.size;
    EXPECT_EQ(layoutBmff(boxes), max32 + 9);
    EXPECT_TRUE(boxes[0].extended);
    EXPECT_EQ(boxes[0].header, shortHeader + 8);

    // Promotion stays in effect if a later pass shrinks the payload.
    boxes[0].segments[0].source.size = 1;
    EXPECT_EQ(layoutBmff(boxes), shortHeader + 9);
    EXPECT_TRUE(boxes[0].extended);
  }

  BmffOutputBox parent;
  parent.type = bmffType("nest");
  parent.prefix = {1, 2, 3};
  parent.children.push_back(sourceBox(bmffType("data"), {uint64_t{1} << 34, max32}));
  std::vector<BmffOutputBox> boxes{parent, sourceBox(bmffType("tail"), {0, 1})};
  EXPECT_EQ(layoutBmff(boxes), max32 + 44);
  EXPECT_EQ(boxes[0].header, 16u);
  EXPECT_EQ(boxes[0].children[0].segments[0].position, 35u);
  EXPECT_EQ(boxes[1].position, max32 + 35);
}

// Reject impossible output sizes and source intervals before attempting any I/O.
TEST(BmffLayout, rejectsSignedPositionOverflow) {
  std::vector<BmffOutputBox> boxes{sourceBox(bmffType("data"), {maxPosition, 1})};
  EXPECT_THROW((void)layoutBmff(boxes), Error);
  boxes[0].segments[0].source = {0, maxPosition};
  EXPECT_THROW((void)layoutBmff(boxes), Error);
  boxes[0].segments[0].source = {0, maxPosition - 16};
  EXPECT_EQ(layoutBmff(boxes), maxPosition);
  boxes.push_back(sourceBox(bmffType("tail"), {0, 0}));
  EXPECT_THROW((void)layoutBmff(boxes), Error);
}

// Translate approved subranges, rejecting gaps and ambiguous or overflowing maps.
TEST(BmffLayout, relocatesOnlyExplicitlyApprovedSourceRanges) {
  const BmffRelocations mappings({{{100, 20}, max32 + 10}, {{20, 10}, 5}});
  EXPECT_EQ(mappings.position({24, 4}), 9u);
  EXPECT_EQ(mappings.position({105, 15}), max32 + 15);
  EXPECT_EQ(mappings.position({120, 0}), max32 + 30);
  EXPECT_THROW((void)mappings.position({19, 1}), Error);
  EXPECT_THROW((void)mappings.position({29, 2}), Error);
  EXPECT_THROW((void)mappings.position({90, 40}), Error);
  EXPECT_THROW((void)mappings.position({maxPosition, 1}), Error);
  EXPECT_THROW((void)BmffRelocations({{{20, 10}, 0}, {{25, 10}, 30}}), Error);
  EXPECT_THROW((void)BmffRelocations({{{0, 2}, maxPosition}}), Error);
  EXPECT_THROW((void)BmffRelocations({{{maxPosition, 1}, 0}}), Error);
}

// Bound requests for source payloads, replacement buffers and owned structural prefixes.
TEST(BmffLayout, boundsAllCopyingAndVerificationRequests) {
  const Bytes source(2 * 64 * 1024 + 13, 0x53);
  const Bytes replacement(64 * 1024 + 19, 0xa7);
  TestIo input(source), output;
  auto box = sourceBox(bmffType("data"), {0, source.size()});
  box.prefix = Bytes(64 * 1024 + 5, 0x29);
  box.segments.push_back({{}, &replacement});
  std::vector<BmffOutputBox> boxes{box};
  (void)layoutBmff(boxes);

  writeBmffLayout(input, output, boxes);
  verifyBmffLayout(input, output, boxes);

  EXPECT_EQ(input.largestRead, 64u * 1024);
  EXPECT_EQ(output.largestRead, 64u * 1024);
  EXPECT_EQ(output.largestWrite, 64u * 1024);
  EXPECT_EQ(input.writes, 0u);
  EXPECT_EQ(contents(input), source);
}

// Propagate failures from both streams without modifying the borrowed input.
TEST(BmffLayout, propagatesShortIoSeekFailuresAndStreamErrors) {
  const Bytes source{1, 2, 3, 4};
  std::vector<BmffOutputBox> boxes{sourceBox(bmffType("data"), {0, source.size()})};
  (void)layoutBmff(boxes);

  for (unsigned failure = 0; failure != 6; ++failure) {
    TestIo input(source), output;
    input.shortRead = failure == 0;
    input.failSeek = failure == 1;
    input.reportError = failure == 2;
    output.shortWrite = failure == 3;
    output.failSeek = failure == 4;
    output.reportError = failure == 5;
    expectError(failure < 3 ? ErrorCode::kerInputDataReadFailed : ErrorCode::kerImageWriteFailed,
                [&] { writeBmffLayout(input, output, boxes); });
    EXPECT_EQ(input.writes, 0u);
    EXPECT_EQ(contents(input), source);
  }

  TestIo input(source), output;
  writeBmffLayout(input, output, boxes);
  output.shortRead = true;
  expectError(ErrorCode::kerInputDataReadFailed, [&] { verifyBmffLayout(input, output, boxes); });
}

// Verify headers, owned prefixes, source bytes and replacement bytes, not only file length.
TEST(BmffLayout, detectsCorruptionInEveryOutputRegion) {
  const Bytes source{1, 2, 3};
  const Bytes replacement{4, 5};
  auto box = sourceBox(bmffType("data"), {0, source.size()});
  box.prefix = {42};
  box.segments.push_back({{}, &replacement});
  std::vector<BmffOutputBox> boxes{box};
  (void)layoutBmff(boxes);

  for (const auto offset : {0u, 8u, 9u, 12u}) {
    MemIo input(source.data(), source.size()), output;
    writeBmffLayout(input, output, boxes);
    output.mmap()[offset] ^= 1;
    expectError(ErrorCode::kerCorruptedMetadata, [&] { verifyBmffLayout(input, output, boxes); });
    EXPECT_EQ(contents(input), source);
  }
}

// Empty layouts are valid, while aliasing and nonempty output streams are rejected.
TEST(BmffLayout, supportsEmptyLayoutsAndRejectsInvalidSinks) {
  MemIo input, output;
  std::vector<BmffOutputBox> boxes;
  EXPECT_EQ(layoutBmff(boxes), 0u);
  EXPECT_NO_THROW(writeBmffLayout(input, output, boxes));
  EXPECT_NO_THROW(verifyBmffLayout(input, output, boxes));
  EXPECT_THROW(writeBmffLayout(input, input, boxes), Error);

  const byte canary = 42;
  ASSERT_EQ(output.write(&canary, 1), 1u);
  EXPECT_THROW(writeBmffLayout(input, output, boxes), Error);
  EXPECT_EQ(contents(output), Bytes{canary});
}

// Normalize item coordinates and promote iloc offsets independently of any image format.
TEST(BmffItemWrite, preservesIdatOriginsAndPromotesFileOffsets) {
  BmffItemModel model;
  auto& file = model.items[70000].location;
  file.baseOffset = 19;
  file.extents.push_back({0, max32 + 1, 3, {100, 3}});
  auto& relative = model.items[2].location;
  relative.baseOffset = 20;
  relative.constructionMethod = 1;
  relative.extents.push_back({0, 0, 5, {200, 5}});
  model.items[3].location.extents.push_back({0, 0, 2, {}});
  model.locationOrder = {70000, 2, 3};

  normalizeBmffLocations(model);
  EXPECT_EQ(model.locationFormat.version, 2u);
  EXPECT_EQ(model.locationFormat.offsetSize, 4u);
  EXPECT_EQ(file.baseOffset, 0u);
  const auto initial = encodeBmffLocations(model, 1024);
  const BmffRelocations retained({{{100, 3}, max32 + 100}, {{200, 5}, max32 + 210}});
  EXPECT_TRUE(relocateBmffItems(model, retained, {{3, max32 + 300}}, max32 + 200));
  EXPECT_EQ(file.extents[0].offset, max32 + 100);
  EXPECT_EQ(relative.extents[0].offset, 10u);
  EXPECT_EQ(model.items.at(3).location.extents[0].offset, max32 + 300);
  EXPECT_EQ(model.locationFormat.offsetSize, 8u);
  EXPECT_EQ(encodeBmffLocations(model, 1024).size(), initial.size() + 12);
  EXPECT_FALSE(relocateBmffItems(model, retained, {{3, max32 + 300}}, max32 + 200));
  EXPECT_THROW(relocateBmffItems(model, retained, {{3, max32 + 300}}, max32 + 211), Error);
}

// Retain extent indices and widen lengths without tying resource limits to HEIF.
TEST(BmffItemWrite, retainsIndicesAndChecksEncodingBudgets) {
  BmffItemModel model;
  model.locationFormat.version = 1;
  model.locationFormat.indexSize = 2;
  model.items[1].location.extents.push_back({9, 0, max32 + 1, {0, max32 + 1}});
  model.locationOrder = {1};
  normalizeBmffLocations(model);
  EXPECT_EQ(model.locationFormat.lengthSize, 8u);
  EXPECT_EQ(model.locationFormat.indexSize, 2u);
  const auto bytes = encodeBmffLocations(model, 100);
  EXPECT_EQ(bytes.size(), 30u);
  EXPECT_EQ(bytes[4], 0x48);
  EXPECT_EQ(bytes[5], 2);
  EXPECT_EQ(bytes[17], 9);
  EXPECT_EQ(encodeBmffLocations(model, bytes.size()), bytes);
  EXPECT_THROW((void)encodeBmffLocations(model, bytes.size() - 1), Error);
  EXPECT_THROW((void)encodeBmffLocations(model, 0), Error);
}

// Keep field-width convergence in the item layer while the adapter chooses payload ownership.
TEST(BmffItemWrite, convergesAndRegeneratesLocationsBeforeAnyIo) {
  // Borrow a neutral item model and provide no image or metadata interpretation.
  class Builder : public BmffItemLayoutBuilder {
   public:
    // Keep the model alive while the item layer updates its widths and offsets.
    explicit Builder(const BmffItemModel& model) : model_(model) {
    }

    mutable unsigned builds{};
    bool unstable{};
    const Bytes replacement{42};

    //! @brief Encode iloc and place a replacement after the retained source interval.
    std::vector<BmffOutputBox> build() const override {
      ++builds;
      BmffOutputBox locations;
      locations.type = bmffType("iloc");
      locations.prefix = encodeBmffLocations(model_, 1024);
      if (unstable)
        locations.prefix.resize(locations.prefix.size() + builds);
      auto data = sourceBox(bmffType("data"), model_.items.at(1).location.extents[0].source);
      BmffOutputBox tail;
      tail.type = bmffType("tail");
      tail.segments.push_back({{}, &replacement});
      return {locations, data, tail};
    }

    //! @brief Approve the source interval and identify the new item's absolute destination.
    BmffItemPositions positions(const std::vector<BmffOutputBox>& boxes) const override {
      const auto& data = boxes.at(1).segments[0];
      return {{{data.source, data.position}}, {{2, boxes.at(2).segments[0].position}}, 0};
    }

   private:
    const BmffItemModel& model_;
  };

  for (const auto length : {uint64_t{3}, max32 + 1}) {
    BmffItemModel model;
    model.items[1].location.extents.push_back({0, 0, length, {0, length}});
    model.items[2].location.extents.push_back({0, 0, 1, {}});
    model.locationOrder = {1, 2};
    Builder builder(model);

    const auto boxes = layoutBmffItems(model, builder);

    EXPECT_EQ(builder.builds, length > max32 ? 3u : 2u);
    EXPECT_EQ(model.locationFormat.offsetSize, length > max32 ? 8u : 4u);
    EXPECT_EQ(model.items.at(1).location.extents[0].offset, boxes[1].segments[0].position);
    EXPECT_EQ(model.items.at(2).location.extents[0].offset, boxes[2].segments[0].position);
    EXPECT_EQ(boxes[0].prefix, encodeBmffLocations(model, 1024));

    // A changing structural size cannot be emitted as a converged result.
    builder.unstable = true;
    EXPECT_THROW((void)layoutBmffItems(model, builder), Error);
  }
}

#endif
