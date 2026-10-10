// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffbox_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include <gtest/gtest.h>
#include <exiv2/basicio.hpp>
#include <exiv2/error.hpp>

#include <algorithm>
#include <limits>
#include <utility>

using namespace Exiv2;
using namespace Exiv2::Internal;

namespace {
using Bytes = std::vector<byte>;
constexpr BmffReadLimits testLimits{100, 1024, 8};

// Encode fixture fields independently of the production reader.
void integer(Bytes& bytes, uint64_t value, unsigned width) {
  for (unsigned i = width; i != 0; --i)
    bytes.push_back(static_cast<byte>(value >> ((i - 1) * 8)));
}

// Append fixture ranges without interpreting their contents.
void append(Bytes& bytes, const Bytes& suffix) {
  bytes.insert(bytes.end(), suffix.begin(), suffix.end());
}

// Construct ordinary or extended boxes, optionally declaring a terminal size.
Bytes box(uint32_t type, const Bytes& payload = {}, bool extended = false, bool terminal = false) {
  Bytes bytes;
  integer(bytes, terminal ? 0 : extended ? 1 : payload.size() + 8, 4);
  integer(bytes, type, 4);
  if (extended)
    integer(bytes, payload.size() + 16, 8);
  append(bytes, payload);
  return bytes;
}

// Decode only the synthetic nest container's four-byte prefix and child range.
std::vector<BmffBox> children(BmffReader& reader, BmffCursor& input, unsigned depth = 0) {
  std::vector<BmffBox> result;
  reader.visit(input, depth, [&](BmffBox& current, BmffCursor& fields) {
    if (current.type == bmffType("nest")) {
      EXPECT_EQ(fields.number(4), 42u);
      current.children = children(reader, fields, depth + 1);
    }
    result.push_back(std::move(current));
  });
  return result;
}

// Wrap a child range in the test adapter's prefixed container.
Bytes nested(const Bytes& payload) {
  Bytes prefix;
  integer(prefix, 42, 4);
  append(prefix, payload);
  return box(bmffType("nest"), prefix);
}

// Distinguish malformed structure from failures of the underlying I/O.
template <typename Operation>
void expectError(ErrorCode code, Operation operation) {
  try {
    operation();
    FAIL() << "Expected an Exiv2::Error";
  } catch (const Error& error) {
    EXPECT_EQ(error.code(), code);
  }
}

// Borrow fixture bytes and inject failures without permitting source mutation.
class FailingIo : public MemIo {
 public:
  using MemIo::read;

  // Retain borrowed fixture storage for this test double's lifetime.
  explicit FailingIo(const Bytes& bytes) : MemIo(bytes.data(), bytes.size()) {
  }

  bool failSeek{};
  bool reportError{};

  //! @brief Fail selected seeks before any field read occurs.
  int seek(int64_t offset, Position origin) override {
    return failSeek ? 1 : MemIo::seek(offset, origin);
  }

  //! @brief Return a short field unless testing an error after a complete read.
  size_t read(byte* output, size_t size) override {
    return MemIo::read(output, reportError ? size : size - 1);
  }

  //! @brief Inject the stream's error flag independently of its returned byte count.
  int error() const override {
    return reportError ? 1 : MemIo::error();
  }
};

// Expose two stored headers separated by a virtual 8 GiB opaque payload.
class SparseIo : public MemIo {
 public:
  using MemIo::read;

  // Store a large extended box and a following terminal box, but no payload bytes.
  SparseIo() {
    integer(prefix, 1, 4);
    integer(prefix, bmffType("skip"), 4);
    integer(prefix, tailOffset, 8);
    tail = box(bmffType("tail"), {}, false, true);
  }

  static constexpr uint64_t tailOffset = uint64_t{1} << 33;
  Bytes prefix;
  Bytes tail;
  uint64_t position{};
  size_t bytesRead{};
  bool forbiddenRead{};

  //! @brief Report a virtual input length without allocating its contents.
  size_t size() const override {
    return static_cast<size_t>(tailOffset + tail.size());
  }

  //! @brief Accept absolute seeks within the virtual input.
  int seek(int64_t offset, Position origin) override {
    if (origin != beg || offset < 0 || static_cast<uint64_t>(offset) > size())
      return 1;
    position = static_cast<uint64_t>(offset);
    return 0;
  }

  //! @brief Serve only stored headers and detect accidental opaque-payload reads.
  size_t read(byte* output, size_t size) override {
    const auto& region = position >= tailOffset ? tail : prefix;
    const auto relative = position >= tailOffset ? position - tailOffset : position;
    if (relative > region.size() || size > region.size() - relative) {
      forbiddenRead = true;
      return 0;
    }
    std::copy_n(region.begin() + relative, size, output);
    position += size;
    bytesRead += size;
    return size;
  }
};
}  // namespace

// Generic traversal accepts opaque boxes without requiring image-format markers.
TEST(BmffBoxReader, traversesNonImageBoxesWithoutInterpretingOpaquePayloads) {
  auto bytes = box(bmffType("data"), box(bmffType("nest"), {0xff}));
  append(bytes, box(bmffType("last"), {1, 2, 3}));
  MemIo io(bytes.data(), bytes.size());
  BmffReader reader(io, {2, 16, 0});
  auto input = reader.cursor({0, reader.fileSize()});

  const auto result = children(reader, input);

  ASSERT_EQ(result.size(), 2u);
  EXPECT_EQ(result.front().type, bmffType("data"));
  EXPECT_TRUE(result.front().children.empty());
  EXPECT_EQ(result.back().span.offset, result.front().span.size);
  EXPECT_EQ(result.back().payload().size, 3u);
  EXPECT_EQ(input.position(), bytes.size());
  EXPECT_NO_THROW(input.finish());
}

// Empty sibling ranges require neither header reads nor a minimum box count.
TEST(BmffBoxReader, acceptsAnEmptyRangeWithZeroBudgets) {
  MemIo io;
  BmffReader reader(io, {0, 0, 0});
  auto input = reader.cursor({0, 0});
  EXPECT_TRUE(children(reader, input).empty());
  EXPECT_EQ(input.number(0), 0u);
  EXPECT_NO_THROW(input.read(nullptr, 0));
}

// Reject input states that cannot support the reader's absolute seek contract.
TEST(BmffBoxReader, rejectsClosedOrUnseekablyLargeInputs) {
  // Simulate a closed stream independently of MemIo's always-open storage.
  class ClosedIo : public MemIo {
   public:
    //! @brief Report that the underlying stream is unavailable.
    bool isopen() const override {
      return false;
    }
  } closed;
  EXPECT_THROW(BmffReader(closed, testLimits), Error);

  if (sizeof(size_t) < 8)
    GTEST_SKIP() << "BasicIo size() cannot exceed signed 64-bit offsets on this host";

  // Report a length that cannot be converted safely to BasicIo's signed offsets.
  class OversizedIo : public MemIo {
   public:
    //! @brief Expose an unrepresentable seek range without allocating storage.
    size_t size() const override {
      return std::numeric_limits<size_t>::max();
    }
  } oversized;
  EXPECT_THROW(BmffReader(oversized, testLimits), Error);
}

// Preserve encoded header fields while excluding UUID user types from payloads.
TEST(BmffBoxReader, retainsNormalExtendedAndTerminalUuidHeaders) {
  for (unsigned mode = 0; mode < 3; ++mode) {
    Bytes uuid(16);
    for (unsigned i = 0; i < uuid.size(); ++i)
      uuid[i] = static_cast<byte>(i);
    auto payload = uuid;
    append(payload, {0x12, 0x34});
    const auto bytes = box(bmffType("uuid"), payload, mode == 1, mode == 2);
    MemIo io(bytes.data(), bytes.size());
    BmffReader reader(io, testLimits);
    auto input = reader.cursor({0, reader.fileSize()});

    const auto header = reader.readBox(input);
    auto fields = reader.cursor(header.payload());

    EXPECT_EQ(header.size32, mode == 1 ? 1u : mode == 2 ? 0u : bytes.size());
    EXPECT_EQ(header.headerSize, mode == 1 ? 32u : 24u);
    EXPECT_EQ(header.extendsToEnd, mode == 2);
    EXPECT_EQ(header.span, (BmffSpan{0, bytes.size()}));
    EXPECT_TRUE(std::equal(uuid.begin(), uuid.end(), header.userType.begin()));
    EXPECT_EQ(fields.number(2), 0x1234u);
    EXPECT_NO_THROW(fields.finish());
    EXPECT_NO_THROW(input.finish());
  }
}

// Truncating any ordinary, extended, or UUID header must fail before payload use.
TEST(BmffBoxReader, rejectsEveryTruncatedHeader) {
  for (const auto type : {bmffType("data"), bmffType("uuid")}) {
    for (const bool extended : {false, true}) {
      const auto bytes = box(type, type == bmffType("uuid") ? Bytes(16) : Bytes{}, extended);
      for (size_t size = 0; size < bytes.size(); ++size) {
        SCOPED_TRACE(size);
        MemIo io(bytes.data(), size);
        BmffReader reader(io, testLimits);
        auto input = reader.cursor({0, reader.fileSize()});
        expectError(ErrorCode::kerCorruptedMetadata, [&] { reader.readBox(input); });
      }
    }
  }
}

// Check declared lengths against both physical headers and immediate parent ranges.
TEST(BmffBoxReader, rejectsInvalidSizesWithoutOverflowOrSiblingEscape) {
  for (const bool extended : {false, true}) {
    for (const uint64_t size : {2ULL, 7ULL, 15ULL, 17ULL, std::numeric_limits<unsigned long long>::max()}) {
      Bytes bytes;
      integer(bytes, extended ? 1 : size, 4);
      integer(bytes, bmffType("data"), 4);
      if (extended)
        integer(bytes, size, 8);
      append(bytes, Bytes(32));
      MemIo io(bytes.data(), bytes.size());
      BmffReader reader(io, testLimits);
      auto input = reader.cursor({0, extended ? 16u : 8u});
      expectError(ErrorCode::kerCorruptedMetadata, [&] { reader.readBox(input); });
    }
  }
}

// Size-zero boxes are valid only in ranges that actually reach the end of the file.
TEST(BmffBoxReader, boundsTerminalBoxesByFileEnd) {
  auto bytes = box(bmffType("data"), {}, false, true);
  append(bytes, box(bmffType("tail"), {}, false, true));
  MemIo io(bytes.data(), bytes.size());
  BmffReader reader(io, testLimits);
  auto truncatedParent = reader.cursor({0, 8});
  expectError(ErrorCode::kerCorruptedMetadata, [&] { reader.readBox(truncatedParent); });

  auto finalChild = reader.cursor({8, 8});
  EXPECT_EQ(reader.readBox(finalChild).span, (BmffSpan{8, 8}));
  auto complete = reader.cursor({0, reader.fileSize()});
  EXPECT_EQ(reader.readBox(complete).span.size, bytes.size());
}

// Cursor creation and field operations reject arithmetic overflow and range escape.
TEST(BmffBoxReader, checksCursorRangesAndFieldWidths) {
  const Bytes bytes{0, 1, 2, 3, 4, 5, 6, 7};
  MemIo io(bytes.data(), bytes.size());
  BmffReader reader(io, testLimits);
  EXPECT_THROW(static_cast<void>(reader.cursor({9, 0})), Error);
  EXPECT_THROW(static_cast<void>(reader.cursor({1, std::numeric_limits<uint64_t>::max()})), Error);
  EXPECT_THROW(static_cast<void>(reader.cursor({std::numeric_limits<uint64_t>::max(), 2})), Error);

  auto input = reader.cursor({0, bytes.size()});
  EXPECT_THROW(input.number(9), Error);
  EXPECT_THROW(input.advance(std::numeric_limits<uint64_t>::max()), Error);
  EXPECT_THROW(input.finish(), Error);
  EXPECT_EQ(input.position(), 0u);
  EXPECT_EQ(input.number(8), 0x0001020304050607ULL);
  EXPECT_THROW(input.number(1), Error);
  EXPECT_NO_THROW(input.finish());
}

// Only the adapter knows the container prefix and which boxes contain children.
TEST(BmffBoxReader, traversesAdapterSelectedChildrenAfterTheirPrefix) {
  auto bytes = nested(nested(box(bmffType("leaf"), {1})));
  append(bytes, box(bmffType("last")));
  MemIo io(bytes.data(), bytes.size());
  BmffReader reader(io, testLimits);
  auto input = reader.cursor({0, reader.fileSize()});

  const auto result = children(reader, input);

  ASSERT_EQ(result.size(), 2u);
  ASSERT_EQ(result[0].children.size(), 1u);
  ASSERT_EQ(result[0].children[0].children.size(), 1u);
  EXPECT_EQ(result[0].children[0].children[0].span.offset, 24u);
  EXPECT_EQ(result[0].children[0].children[0].type, bmffType("leaf"));
  EXPECT_EQ(result[1].type, bmffType("last"));
}

// Byte and box budgets are cumulative across nested and newly obtained cursors.
TEST(BmffBoxReader, enforcesCallerBudgetsAcrossAllVisits) {
  const auto bytes = nested(box(bmffType("leaf")));
  for (const auto limits : {BmffReadLimits{1, 20, 1}, BmffReadLimits{2, 19, 1}, BmffReadLimits{2, 20, 0}}) {
    MemIo io(bytes.data(), bytes.size());
    BmffReader reader(io, limits);
    auto input = reader.cursor({0, reader.fileSize()});
    EXPECT_THROW(children(reader, input), Error);
  }

  MemIo io(bytes.data(), bytes.size());
  BmffReader reader(io, {2, 20, 1});
  auto input = reader.cursor({0, reader.fileSize()});
  EXPECT_NO_THROW(children(reader, input));
  auto revisit = reader.cursor({0, reader.fileSize()});
  EXPECT_THROW(reader.readBox(revisit), Error);
}

// A cursor cannot charge another reader's budget or read that reader's input.
TEST(BmffBoxReader, rejectsCursorsFromAnotherReader) {
  const auto bytes = box(bmffType("data"));
  MemIo io(bytes.data(), bytes.size());
  BmffReader first(io, testLimits), second(io, testLimits);
  auto input = first.cursor({0, first.fileSize()});
  EXPECT_THROW(second.readBox(input), Error);
  EXPECT_THROW(children(second, input), Error);
  EXPECT_EQ(input.position(), 0u);
}

// Failed seeks, short reads, and stream errors remain I/O errors and never write.
TEST(BmffBoxReader, propagatesIoFailuresWithoutMutatingSource) {
  const auto bytes = box(bmffType("data"));
  for (unsigned mode = 0; mode < 3; ++mode) {
    FailingIo io(bytes);
    io.failSeek = mode == 1;
    io.reportError = mode == 2;
    BmffReader reader(io, testLimits);
    auto input = reader.cursor({0, reader.fileSize()});
    expectError(ErrorCode::kerInputDataReadFailed, [&] { reader.readBox(input); });
    EXPECT_EQ(input.position(), 0u);
    EXPECT_TRUE(std::equal(bytes.begin(), bytes.end(), io.mmap()));
  }
}

// Skip an extended opaque payload beyond 32-bit offsets using only header reads.
TEST(BmffBoxReader, traversesSparse64BitRangesWithoutPayloadReads) {
  if (sizeof(size_t) < 8)
    GTEST_SKIP() << "BasicIo size() cannot represent this input on a 32-bit host";
  SparseIo io;
  BmffReader reader(io, {2, 24, 0});
  auto input = reader.cursor({0, reader.fileSize()});

  const auto result = children(reader, input);

  ASSERT_EQ(result.size(), 2u);
  EXPECT_EQ(result[0].size32, 1u);
  EXPECT_EQ(result[0].span.size, SparseIo::tailOffset);
  EXPECT_EQ(result[1].span.offset, SparseIo::tailOffset);
  EXPECT_EQ(result[1].size32, 0u);
  EXPECT_EQ(io.bytesRead, 24u);
  EXPECT_FALSE(io.forbiddenRead);
}

#endif  // EXV_ENABLE_BMFF
