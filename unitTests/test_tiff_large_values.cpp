// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>
#include <tiffcomposite_int.hpp>
#include <tiffimage_int.hpp>
#include <tiffvisitor_int.hpp>

#include <array>
#include <limits>

using namespace Exiv2;
using namespace Exiv2::Internal;

namespace {
template <typename F>
void expectError(F&& operation, ErrorCode code) {
  try {
    operation();
    FAIL() << "Expected an Exiv2 error";
  } catch (const Error& error) {
    EXPECT_EQ(code, error.code());
  }
}

class TiffValueRange : public testing::TestWithParam<ByteOrder> {
 protected:
  std::array<byte, 32> buffer{};
  TiffEntry entry{0x935c, IfdId::ifd0Id};

  void read(uint16_t type, uint32_t count, uint32_t offset, size_t size = 24, size_t base = 0) {
    us2Data(buffer.data(), 0x935c, GetParam());
    us2Data(buffer.data() + 2, type, GetParam());
    ul2Data(buffer.data() + 4, count, GetParam());
    ul2Data(buffer.data() + 8, offset, GetParam());
    entry.setStart(buffer.data());
    TiffReader reader(buffer.data(), size, &entry, TiffRwState{GetParam(), base});
    reader.readTiffEntry(&entry);
  }
};

// Exercise field-width checks without constructing a multi-gigabyte Value.
class CountValue : public DataValue {
 public:
  explicit CountValue(size_t count) : count_(count) {
    value_ = {42};
  }
  size_t count() const override {
    return count_;
  }

 private:
  CountValue* clone_() const override {
    return new CountValue(*this);
  }
  size_t count_;
};

// Report an oversized layout without allocating its payload.
class SizeEntry : public TiffEntry {
 public:
  SizeEntry(size_t valueSize, size_t imageSize = 0) :
      TiffEntry(0x935c, IfdId::ifd0Id), valueSize_(valueSize), imageSize_(imageSize) {
  }

 private:
  size_t doSize() const override {
    return valueSize_;
  }
  size_t doSizeImage() const override {
    return imageSize_;
  }
  size_t valueSize_;
  size_t imageSize_;
};

// A large declared header makes the output unrepresentable without allocating it.
class LargeHeader : public TiffHeaderBase {
 public:
  LargeHeader() : TiffHeaderBase(42, std::numeric_limits<uint32_t>::max(), littleEndian, 8) {
  }
  bool read(const byte*, size_t) override {
    return true;
  }
};
}  // namespace

TEST_P(TiffValueRange, readsInlineBytes) {
  read(7, 4, 0x01020304);
  ASSERT_NE(nullptr, entry.pValue());
  EXPECT_EQ(4u, entry.pValue()->count());
  EXPECT_EQ(buffer.data() + 8, entry.pData());
}

TEST_P(TiffValueRange, readsEmptyValueWithoutFollowingItsOffset) {
  read(7, 0, std::numeric_limits<uint32_t>::max());
  ASSERT_NE(nullptr, entry.pValue());
  EXPECT_EQ(0u, entry.pValue()->count());
}

TEST_P(TiffValueRange, readsPayloadEndingAtBufferBoundary) {
  buffer[23] = 42;
  read(7, 8, 16);
  ASSERT_NE(nullptr, entry.pValue());
  EXPECT_EQ(8u, entry.pValue()->count());
  EXPECT_EQ(42, entry.pValue()->toInt64(7));
}

TEST_P(TiffValueRange, truncatesPayloadExtendingPastBufferBoundary) {
  read(7, 9, 16);
  ASSERT_NE(nullptr, entry.pValue());
  EXPECT_EQ(0u, entry.pValue()->count());
}

TEST_P(TiffValueRange, truncatesInvalidOffsetBeforeFollowingIt) {
  read(7, 8, std::numeric_limits<uint32_t>::max());
  ASSERT_NE(nullptr, entry.pValue());
  EXPECT_EQ(0u, entry.pValue()->count());
}

TEST_P(TiffValueRange, checksMakerNoteBaseBeforeFollowingOffset) {
  expectError([&] { read(7, 8, 16, 24, std::numeric_limits<size_t>::max() - 8); }, ErrorCode::kerCorruptedMetadata);
}

TEST_P(TiffValueRange, resolvesMakerNoteBaseWithinTheSuppliedBuffer) {
  buffer[23] = 42;
  read(7, 8, 8, 24, 8);
  ASSERT_NE(nullptr, entry.pValue());
  EXPECT_EQ(8u, entry.pValue()->count());
  EXPECT_EQ(42, entry.pValue()->toInt64(7));
}

TEST_P(TiffValueRange, retainsSonyPreviewReferenceOutsideMetadata) {
  us2Data(buffer.data() + 2, 1, GetParam());
  ul2Data(buffer.data() + 4, 1000, GetParam());
  ul2Data(buffer.data() + 8, 100, GetParam());
  TiffEntry preview(0x2001, IfdId::sony1Id);
  preview.setStart(buffer.data());
  TiffReader reader(buffer.data(), buffer.size(), &preview, TiffRwState{GetParam(), 0});
  reader.readTiffEntry(&preview);
  ASSERT_NE(nullptr, preview.pValue());
  EXPECT_EQ(undefined, preview.pValue()->typeId());
  EXPECT_EQ(0u, preview.pValue()->count());
  EXPECT_EQ(100u, preview.offset());
}

TEST_P(TiffValueRange, readsMultibyteValuesInTiffByteOrder) {
  us2Data(buffer.data() + 16, 0x1234, GetParam());
  us2Data(buffer.data() + 18, 0x5678, GetParam());
  us2Data(buffer.data() + 20, 0xabcd, GetParam());
  read(3, 3, 16, 22);
  ASSERT_NE(nullptr, entry.pValue());
  ASSERT_EQ(3u, entry.pValue()->count());
  EXPECT_EQ(0x1234, entry.pValue()->toInt64(0));
  EXPECT_EQ(0x5678, entry.pValue()->toInt64(1));
  EXPECT_EQ(0xabcd, entry.pValue()->toInt64(2));
}

TEST_P(TiffValueRange, retainsUnknownTypeWithByteSizedPayload) {
  buffer[23] = 42;
  read(0xffff, 8, 16);
  ASSERT_NE(nullptr, entry.pValue());
  EXPECT_EQ(8u, entry.pValue()->size());
}

TEST_P(TiffValueRange, rejectsHugeProductBeforeAllocating) {
  // Division against the actual buffer size rejects the count even on 32-bit hosts.
  read(12, std::numeric_limits<uint32_t>::max(), 16);
  EXPECT_EQ(nullptr, entry.pValue());
}

TEST_P(TiffValueRange, checksProductForExternalSonyReference) {
  us2Data(buffer.data() + 2, 12, GetParam());
  ul2Data(buffer.data() + 4, std::numeric_limits<uint32_t>::max(), GetParam());
  ul2Data(buffer.data() + 8, 100, GetParam());
  TiffEntry preview(0x2001, IfdId::sony1Id);
  preview.setStart(buffer.data());
  TiffReader reader(buffer.data(), buffer.size(), &preview, TiffRwState{GetParam(), 0});
  if constexpr (sizeof(size_t) == 4) {
    expectError([&] { reader.readTiffEntry(&preview); }, ErrorCode::kerArithmeticOverflow);
  } else {
    reader.readTiffEntry(&preview);
    ASSERT_NE(nullptr, preview.pValue());
    EXPECT_EQ(0u, preview.pValue()->count());
  }
}

TEST_P(TiffValueRange, skipsTruncatedDirectoryEntry) {
  read(7, 8, 16, 11);
  EXPECT_EQ(nullptr, entry.pValue());
}

INSTANTIATE_TEST_SUITE_P(ByteOrders, TiffValueRange, testing::Values(littleEndian, bigEndian));

TEST(TiffLargeLayout, rejectsUnrepresentableValueOffsetBeforeWriting) {
  TiffDirectory directory(0, IfdId::ifd0Id);
  directory.addChild(std::make_unique<SizeEntry>(8));
  MemIo output;
  IoWrapper wrapper(output, nullptr, 0, nullptr);
  size_t image = std::string::npos;
  expectError(
      [&] {
        directory.write(wrapper, littleEndian, std::numeric_limits<uint32_t>::max() - 8, std::string::npos,
                        std::string::npos, image);
      },
      ErrorCode::kerOffsetOutOfRange);
  EXPECT_EQ(0u, output.size());
}

TEST(TiffLargeLayout, rejectsAlignmentOverflowBeforeWriting) {
  TiffDirectory directory(0, IfdId::ifd0Id);
  directory.addChild(std::make_unique<SizeEntry>(std::numeric_limits<size_t>::max()));
  MemIo output;
  IoWrapper wrapper(output, nullptr, 0, nullptr);
  size_t image = std::string::npos;
  expectError([&] { directory.write(wrapper, littleEndian, 8, std::string::npos, std::string::npos, image); },
              ErrorCode::kerArithmeticOverflow);
  EXPECT_EQ(0u, output.size());
}

TEST(TiffLargeLayout, includesImageDataInLayoutPreflight) {
  TiffDirectory directory(0, IfdId::ifd0Id);
  directory.addChild(std::make_unique<SizeEntry>(0, std::numeric_limits<size_t>::max()));
  MemIo output;
  IoWrapper wrapper(output, nullptr, 0, nullptr);
  size_t image = std::string::npos;
  expectError([&] { directory.write(wrapper, littleEndian, 8, std::string::npos, std::string::npos, image); },
              ErrorCode::kerArithmeticOverflow);
  EXPECT_EQ(0u, output.size());
}

TEST(TiffLargeLayout, rejectsUnrepresentableCountWithoutNarrowing) {
  if constexpr (sizeof(size_t) <= 4) {
    GTEST_SKIP() << "size_t cannot represent a count larger than a TIFF field";
  } else {
    TiffDirectory directory(0, IfdId::ifd0Id);
    auto entry = std::make_unique<TiffEntry>(0x935c, IfdId::ifd0Id);
    entry->setValue(std::make_unique<CountValue>(static_cast<size_t>(uint64_t{1} << 32)));
    directory.addChild(std::move(entry));
    MemIo output;
    IoWrapper wrapper(output, nullptr, 0, nullptr);
    size_t image = std::string::npos;
    expectError([&] { directory.write(wrapper, littleEndian, 8, std::string::npos, std::string::npos, image); },
                ErrorCode::kerImageWriteFailed);
  }
}

TEST(TiffLargeLayout, rejectsInvalidCountBeforeUpdatingMappedSource) {
  if constexpr (sizeof(size_t) <= 4) {
    GTEST_SKIP() << "size_t cannot represent a count larger than a TIFF field";
  } else {
    // One inline Artist value; no large source or allocation is needed.
    std::array<byte, 26> data{'I', 'I', 42, 0, 8, 0, 0, 0, 1, 0, 0x3b, 1, 2, 0, 2, 0, 0, 0, 'A'};
    const auto original = data;
    ExifData exif;
    exif["Exif.Image.Artist"] = "B";
    CountValue invalid(static_cast<size_t>(uint64_t{1} << 32));
    exif.add(ExifKey("Exif.Image.0x935c"), &invalid);
    IptcData iptc;
    XmpData xmp;
    MemIo io;
    expectError([&] { TiffParser::encode(io, data.data(), data.size(), littleEndian, exif, iptc, xmp); },
                ErrorCode::kerImageWriteFailed);
    EXPECT_EQ(original, data);
    EXPECT_EQ(0u, io.size());
  }
}

TEST(TiffLargeLayout, rejectsInvalidLayoutBeforeUpdatingMappedSource) {
  std::array<byte, 26> data{'I', 'I', 42, 0, 8, 0, 0, 0, 1, 0, 0x3b, 1, 2, 0, 2, 0, 0, 0, 'A'};
  const auto original = data;
  ExifData exif;
  exif["Exif.Image.Artist"] = "B";
  IptcData iptc;
  XmpData xmp;
  MemIo io;
  LargeHeader header;
  expectError(
      [&] {
        TiffParserWorker::encode(io, data.data(), data.size(), exif, iptc, xmp, Tag::root, TiffMapping::findEncoder,
                                 &header, nullptr);
      },
      ErrorCode::kerOffsetOutOfRange);
  EXPECT_EQ(original, data);
  EXPECT_EQ(0u, io.size());
}
