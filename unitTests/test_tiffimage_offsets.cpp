// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace Exiv2;

namespace {

// Describe one valid uncompressed grayscale image and its distinct expected chunks.
struct Page {
  std::string group;
  bool tiled;
  uint32_t width;
  uint32_t height;
  std::vector<Blob> chunks;
};

// Retain independently constructed input bytes and image identities through rewrites.
struct Fixture {
  Blob bytes;
  std::vector<Page> pages;
};

// Represent the small numeric or ASCII fields used only by the fixture builder.
struct Field {
  uint16_t tag;
  uint16_t type;
  std::vector<uint32_t> values;
};

// Construct TIFF directories and payloads without invoking the writer under test.
Fixture makeFixture(size_t children, ByteOrder order, int mode) {
  Fixture fixture{
      {order == littleEndian ? byte{'I'} : byte{'M'}, order == littleEndian ? byte{'I'} : byte{'M'}, 0, 0, 0, 0, 0, 0},
      {}};
  us2Data(fixture.bytes.data() + 2, 42, order);
  ul2Data(fixture.bytes.data() + 4, 8, order);

  // Include root and next-IFD previews around the independently addressable children.
  for (size_t i = 0; i < children + 2; ++i) {
    const bool child = i > 0 && i <= children;
    const bool tiled = child && (mode == 1 || (mode == 2 && i % 2));
    const auto count = child ? uint32_t{1} + static_cast<uint32_t>(i % 2) : uint32_t{1};
    const auto width = tiled ? 16 * count : static_cast<uint32_t>(3 + i);
    Page page{i == 0  ? "Image"
              : child ? "SubImage" + std::to_string(i)
                      : "Thumbnail",
              tiled,
              width,
              tiled ? 16U : count,
              {}};
    for (uint32_t chunk = 0; chunk < count; ++chunk)
      page.chunks.emplace_back(tiled ? 256 : width, static_cast<byte>(11 + 13 * i + chunk));
    fixture.pages.push_back(std::move(page));
  }

  std::vector<size_t> directories;
  std::vector<std::vector<size_t>> imageOffsets;
  size_t subIfds = 0;
  size_t nextIfd = 0;
  for (size_t i = 0; i < fixture.pages.size(); ++i) {
    const auto& page = fixture.pages[i];
    const uint16_t offsets = page.tiled ? 0x0144 : 0x0111;
    const uint16_t counts = page.tiled ? 0x0145 : 0x0117;
    std::vector<uint32_t> lengths;
    for (const auto& chunk : page.chunks)
      lengths.push_back(static_cast<uint32_t>(chunk.size()));
    std::vector<Field> fields{{0x00fe, 4, {i == 0 ? 0U : 1U}},
                              {0x0100, 4, {page.width}},
                              {0x0101, 4, {page.height}},
                              {0x0102, 3, {8}},
                              {0x0103, 3, {1}},
                              {0x0106, 3, {1}},
                              {0x0115, 3, {1}},
                              {offsets, 4, std::vector<uint32_t>(lengths.size())},
                              {counts, 4, lengths}};
    if (page.tiled) {
      fields.push_back({0x0142, 4, {16}});
      fields.push_back({0x0143, 4, {16}});
    } else {
      fields.push_back({0x0116, 4, {1}});
    }
    if (i == 0) {
      fields.push_back({0x013b, 2, {'s', 'e', 'e', 'd', 0}});
      if (children)
        fields.push_back({0x014a, 4, std::vector<uint32_t>(children)});
    }
    std::sort(fields.begin(), fields.end(), [](const auto& a, const auto& b) { return a.tag < b.tag; });

    // Reserve each directory before appending out-of-line values; retain indices, not pointers.
    auto& bytes = fixture.bytes;
    const auto start = bytes.size();
    directories.push_back(start);
    bytes.resize(start + 2 + 12 * fields.size() + 4);
    us2Data(bytes.data() + start, static_cast<uint16_t>(fields.size()), order);
    if (i == 0)
      nextIfd = start + 2 + 12 * fields.size();
    std::vector<size_t> positions;
    for (size_t f = 0; f < fields.size(); ++f) {
      const auto& field = fields[f];
      const auto entry = start + 2 + 12 * f;
      us2Data(bytes.data() + entry, field.tag, order);
      us2Data(bytes.data() + entry + 2, field.type, order);
      ul2Data(bytes.data() + entry + 4, static_cast<uint32_t>(field.values.size()), order);
      const size_t width = field.type == 4 ? 4 : field.type == 3 ? 2 : 1;
      auto value = entry + 8;
      if (width * field.values.size() > 4) {
        value = bytes.size();
        ul2Data(bytes.data() + entry + 8, static_cast<uint32_t>(value), order);
        bytes.resize(value + width * field.values.size());
        bytes.resize(bytes.size() + bytes.size() % 2);
      }
      for (size_t v = 0; v < field.values.size(); ++v) {
        if (width == 4)
          ul2Data(bytes.data() + value + width * v, field.values[v], order);
        else if (width == 2)
          us2Data(bytes.data() + value + width * v, static_cast<uint16_t>(field.values[v]), order);
        else
          bytes[value + v] = static_cast<byte>(field.values[v]);
        if (field.tag == offsets)
          positions.push_back(value + width * v);
      }
      if (field.tag == 0x014a)
        subIfds = value;
    }
    imageOffsets.push_back(std::move(positions));
  }

  // Link directories, then place every source chunk in explicit directory/index order.
  auto& bytes = fixture.bytes;
  ul2Data(bytes.data() + nextIfd, static_cast<uint32_t>(directories.back()), order);
  for (size_t i = 0; i < children; ++i)
    ul2Data(bytes.data() + subIfds + 4 * i, static_cast<uint32_t>(directories[i + 1]), order);
  for (size_t i = 0; i < fixture.pages.size(); ++i) {
    for (size_t c = 0; c < fixture.pages[i].chunks.size(); ++c) {
      ul2Data(bytes.data() + imageOffsets[i][c], static_cast<uint32_t>(bytes.size()), order);
      const auto& chunk = fixture.pages[i].chunks[c];
      bytes.insert(bytes.end(), chunk.begin(), chunk.end());
      bytes.resize(bytes.size() + bytes.size() % 2);
    }
  }
  return fixture;
}

// Read the complete in-memory file without retaining a mapped pointer across edits.
Blob imageBytes(Image& image) {
  auto& io = image.io();
  if (io.open() != 0)
    throw Error(ErrorCode::kerFailedToReadImageData);
  IoCloser closer(io);
  const auto bytes = io.read(io.size());
  return {bytes.c_data(), bytes.c_data() + bytes.size()};
}

// Resolve every reopened offset/count pair and compare actual pixels and image identity.
void expectImages(Image& image, const Fixture& fixture) {
  image.readMetadata();
  const auto bytes = imageBytes(image);
  EXPECT_EQ(fixture.pages[0].width, image.pixelWidth());
  EXPECT_EQ(fixture.pages[0].height, image.pixelHeight());
  for (const auto& page : fixture.pages) {
    SCOPED_TRACE(page.group);
    const auto prefix = "Exif." + page.group + ".";
    auto& data = image.exifData();
    const auto offsets = data.findKey(ExifKey(prefix + (page.tiled ? "TileOffsets" : "StripOffsets")));
    const auto counts = data.findKey(ExifKey(prefix + (page.tiled ? "TileByteCounts" : "StripByteCounts")));
    ASSERT_NE(data.end(), offsets);
    ASSERT_NE(data.end(), counts);
    ASSERT_EQ(page.chunks.size(), offsets->count());
    ASSERT_EQ(page.chunks.size(), counts->count());
    EXPECT_EQ(page.width, data[prefix + "ImageWidth"].toUint32());
    EXPECT_EQ(page.height, data[prefix + "ImageLength"].toUint32());
    EXPECT_EQ(page.group == "Image" ? 0U : 1U, data[prefix + "NewSubfileType"].toUint32());
    for (size_t c = 0; c < page.chunks.size(); ++c) {
      const size_t start = offsets->toUint32(c);
      const size_t length = counts->toUint32(c);
      EXPECT_EQ(0U, start % 2);
      ASSERT_LE(start, bytes.size());
      ASSERT_LE(length, bytes.size() - start);
      EXPECT_EQ(page.chunks[c], Blob(bytes.begin() + start, bytes.begin() + start + length));
    }
  }
}

// Cross the four-child reduction boundary in both byte orders and payload representations.
class TiffImageOffsets : public ::testing::TestWithParam<std::tuple<size_t, ByteOrder, int>> {};

// Preserve root, SubIFD and next-IFD image bytes through intrusive and repeated edits.
TEST_P(TiffImageOffsets, PreservesImagesDuringEdits) {
  const auto [children, order, mode] = GetParam();
  const auto fixture = makeFixture(children, order, mode);
  auto image = ImageFactory::open(fixture.bytes.data(), fixture.bytes.size());
  expectImages(*image, fixture);

  image->exifData()["Exif.Image.Artist"] = std::string(600, 'a');
  ASSERT_NO_THROW(image->writeMetadata());
  expectImages(*image, fixture);
  EXPECT_EQ(std::string(600, 'a'), image->exifData()["Exif.Image.Artist"].toString());

  image->exifData()["Exif.Image.Artist"] = std::string(600, 'b');
  ASSERT_NO_THROW(image->writeMetadata());
  expectImages(*image, fixture);
  EXPECT_EQ(std::string(600, 'b'), image->exifData()["Exif.Image.Artist"].toString());

  image->exifData().erase(image->exifData().findKey(ExifKey("Exif.Image.Artist")));
  ASSERT_NO_THROW(image->writeMetadata());
  expectImages(*image, fixture);
  EXPECT_EQ(image->exifData().end(), image->exifData().findKey(ExifKey("Exif.Image.Artist")));
  ASSERT_NO_THROW(image->writeMetadata());
  expectImages(*image, fixture);
}

// Fresh serialization also needs ordered child emission when streams are supplied as data areas.
TEST(TiffImageOffsets, PreservesFreshImageDataAreas) {
  for (const auto order : {littleEndian, bigEndian}) {
    const auto fixture = makeFixture(5, order, 1);
    auto source = ImageFactory::open(fixture.bytes.data(), fixture.bytes.size());
    source->readMetadata();
    auto data = source->exifData();
    for (const auto& page : fixture.pages) {
      Blob pixels;
      for (const auto& chunk : page.chunks)
        pixels.insert(pixels.end(), chunk.begin(), chunk.end());
      data["Exif." + page.group + (page.tiled ? ".TileOffsets" : ".StripOffsets")].setDataArea(pixels.data(),
                                                                                               pixels.size());
    }
    MemIo output;
    TiffParser::encode(output, nullptr, 0, order, data, {}, {});
    output.seek(0, BasicIo::beg);
    const auto bytes = output.read(output.size());
    auto image = ImageFactory::open(bytes.c_data(), bytes.size());
    expectImages(*image, fixture);
  }
}

INSTANTIATE_TEST_SUITE_P(ChildrenOrdersAndPayloads, TiffImageOffsets,
                         ::testing::Combine(::testing::Values(size_t{0}, size_t{1}, size_t{2}, size_t{3}, size_t{4},
                                                              size_t{5}, size_t{8}, size_t{9}),
                                            ::testing::Values(littleEndian, bigEndian), ::testing::Values(0, 1, 2)));

}  // namespace
