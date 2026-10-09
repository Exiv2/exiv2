// SPDX-License-Identifier: GPL-2.0-or-later

#include "bmffimage_int.hpp"
#include "bmffwrite_int.hpp"

#ifdef EXV_ENABLE_BMFF

#include <gtest/gtest.h>
#include <exiv2/basicio.hpp>
#include <exiv2/bmffimage.hpp>
#include <exiv2/error.hpp>
#include <exiv2/exiv2.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "unittest_utils.hpp"

using namespace Exiv2;
using namespace Exiv2::Internal;

namespace {
using Bytes = std::vector<byte>;

// Append a big-endian field without using the production serializer.
void integer(Bytes& output, uint64_t value, unsigned width) {
  for (unsigned i = width; i != 0; --i)
    output.push_back(static_cast<byte>(value >> ((i - 1) * 8)));
}

// Append independently assembled fixture bytes.
void append(Bytes& output, const Bytes& value) {
  output.insert(output.end(), value.begin(), value.end());
}

// Append a null-terminated format string.
void string(Bytes& output, std::string_view value) {
  output.insert(output.end(), value.begin(), value.end());
  output.push_back(0);
}

// Construct a box with a normal, extended, or end-of-file size header.
Bytes box(uint32_t type, const Bytes& payload, bool extended = false, bool toEnd = false) {
  Bytes result;
  integer(result, toEnd ? 0 : extended ? 1 : payload.size() + 8, 4);
  integer(result, type, 4);

  if (extended)
    integer(result, payload.size() + 16, 8);

  append(result, payload);
  return result;
}

// Construct the version and flags prefix of a FullBox.
Bytes full(uint8_t version = 0, uint32_t flags = 0) {
  Bytes result;
  integer(result, (uint32_t{version} << 24) | flags, 4);
  return result;
}

// Locate the first fixture box by its four-character type or fail the test setup.
size_t position(const Bytes& bytes, std::string_view type) {
  const auto found = std::search(bytes.begin(), bytes.end(), type.begin(), type.end());
  if (type.size() != 4 || found == bytes.end() || found - bytes.begin() < 4)
    throw std::runtime_error("missing fixture box");
  return static_cast<size_t>(found - bytes.begin()) - 4;
}

// Overwrite a bounded fixture field to construct a specific input mutation.
void patch(Bytes& bytes, size_t offset, uint64_t value, unsigned width) {
  Bytes encoded;
  integer(encoded, value, width);
  if (offset > bytes.size() || width > bytes.size() - offset)
    throw std::runtime_error("invalid fixture patch");

  std::copy(encoded.begin(), encoded.end(), bytes.begin() + offset);
}

// Select independently encoded fixture layouts, payloads, and malformed variants.
struct Options {
  std::optional<Bytes> exifPayload, xmpPayload;
  bool secondExif{}, metadataIdat{};
  std::string mimeType{"application/rdf+xml"};
  std::string auxiliaryType;
  uint32_t brand{bmffType("heic")};
  uint32_t idBase{};
  uint8_t locationVersion{};
  uint8_t infoVersion{};
  uint8_t entryVersion{2};
  uint8_t primaryVersion{};
  uint8_t referenceVersion{};
  uint8_t associationVersion{};
  uint8_t offsetWidth{4};
  uint8_t lengthWidth{4};
  uint8_t baseWidth{};
  uint8_t indexWidth{};
  bool wideProperties{};
  bool extended{};
  bool terminal{};
  bool idat{};
  bool noMetadata{};
  bool noReferences{};
  bool sharedMetadata{};
  bool locationFirst{};
  uint16_t propertyCount{2};
  uint32_t propertyType{bmffType("ispe")};
  std::string encoding;
  Bytes extraMeta;
  Bytes extraRoot;
};

//! Synthetic bytes are independently assembled from the format's field layout.
//! Image and metadata payloads deliberately need no decoder for structural tests.
Bytes fixture(const Options& opt = {}) {
  Bytes fileType;
  integer(fileType, opt.brand, 4);
  integer(fileType, 0, 4);
  integer(fileType, bmffType("mif1"), 4);
  integer(fileType, opt.brand, 4);
  auto ftyp = box(bmffType("ftyp"), fileType, opt.extended);

  // Interleave metadata with two primary-image extents to exercise compaction.
  const auto exifPayload = opt.exifPayload.value_or(Bytes{'e', 'x', 'i', 'f'});
  const auto xmpPayload = opt.xmpPayload.value_or(Bytes{'x', 'm', 'p'});
  Bytes payload{'A', 'B'};
  append(payload, exifPayload);
  append(payload, Bytes{'C', 'D'});
  append(payload, xmpPayload);
  const unsigned count = opt.noMetadata ? 1 : 3;
  const unsigned headerSize = opt.extended ? 16 : 8;

  // Build meta twice so file-relative media offsets include its final encoded size.
  auto meta = [&](uint64_t mediaStart) {
    auto result = full();
    auto handler = full();
    integer(handler, 0, 4);
    integer(handler, bmffType("pict"), 4);
    for (int i = 0; i < 3; ++i)
      integer(handler, 0, 4);
    string(handler, "");
    append(result, box(bmffType("hdlr"), handler, opt.extended));

    // Select the primary item using the version-dependent ID width.
    auto primary = full(opt.primaryVersion);
    integer(primary, opt.idBase + 1, opt.primaryVersion == 0 ? 2 : 4);
    append(result, box(bmffType("pitm"), primary, opt.extended));

    // Describe image, Exif, and XMP items independently of their location table.
    auto info = full(opt.infoVersion);
    integer(info, count, opt.infoVersion == 0 ? 2 : 4);
    for (unsigned i = 1; i <= count; ++i) {
      auto entry = full(opt.entryVersion, i == 1 ? 0 : 1);
      integer(entry, opt.idBase + i, opt.entryVersion == 2 ? 2 : 4);
      integer(entry, 0, 2);
      integer(entry, i == 1 ? bmffType("hvc1") : i == 2 || opt.secondExif ? bmffType("Exif") : bmffType("mime"), 4);
      string(entry, i == 1 ? "image" : i == 2 ? "exif" : "xmp");
      if (i == 3 && !opt.secondExif) {
        string(entry, opt.mimeType);
        string(entry, opt.encoding);
      }
      append(info, box(bmffType("infe"), entry, opt.extended));
    }

    // Encode iloc using the requested widths and coordinate origins.
    auto locations = full(opt.locationVersion);
    integer(locations, (opt.offsetWidth << 4) | opt.lengthWidth, 1);
    integer(locations, (opt.baseWidth << 4) | opt.indexWidth, 1);
    integer(locations, count, opt.locationVersion < 2 ? 2 : 4);
    for (unsigned i = 1; i <= count; ++i) {
      const bool inIdat = opt.idat && (i == 1 || opt.metadataIdat);
      const uint64_t source = inIdat ? 0 : mediaStart;
      const uint64_t itemOffset = i == 1 ? 0 : i == 2 ? 2 : 4 + exifPayload.size();
      integer(locations, opt.idBase + i, opt.locationVersion < 2 ? 2 : 4);
      if (opt.locationVersion != 0)
        integer(locations, inIdat ? 1 : 0, 2);
      integer(locations, 0, 2);
      integer(locations, opt.offsetWidth == 0 ? source + itemOffset : source, opt.baseWidth);

      // Split the primary image around metadata when the offset fields allow it.
      const bool fragmented = i == 1 && opt.offsetWidth != 0;
      integer(locations, fragmented ? 2 : 1, 2);
      integer(locations, 0, opt.indexWidth);
      integer(locations, (opt.baseWidth == 0 ? source : 0) + itemOffset, opt.offsetWidth);
      integer(locations,
              i == 1   ? (fragmented ? 2 : 4 + exifPayload.size())
              : i == 2 ? exifPayload.size()
                       : xmpPayload.size(),
              opt.lengthWidth);
      if (fragmented) {
        integer(locations, 0, opt.indexWidth);
        integer(locations, (opt.baseWidth == 0 ? source : 0) + 2 + exifPayload.size(), opt.offsetWidth);
        integer(locations, 2, opt.lengthWidth);
      }
    }

    // Allow either table order to catch parsers that require info before locations.
    auto iloc = box(bmffType("iloc"), locations, opt.extended);
    auto iinf = box(bmffType("iinf"), info, opt.extended);
    append(result, opt.locationFirst ? iloc : iinf);
    append(result, opt.locationFirst ? iinf : iloc);

    // Associate metadata with the primary image, optionally sharing it with another item.
    if (!opt.noReferences && !opt.noMetadata) {
      auto references = full(opt.referenceVersion);
      for (unsigned i = 2; i <= count; ++i) {
        Bytes ref;
        const unsigned width = opt.referenceVersion == 0 ? 2 : 4;
        integer(ref, opt.idBase + i, width);
        integer(ref, opt.sharedMetadata && i == 2 ? 2 : 1, 2);
        integer(ref, opt.idBase + 1, width);
        if (opt.sharedMetadata && i == 2)
          integer(ref, opt.idBase + 3, width);
        append(references, box(bmffType("cdsc"), ref, opt.extended));
      }
      append(result, box(bmffType("iref"), references, opt.extended));
    }

    // Construct image properties and optional auxiliary-image type information.
    Bytes properties;
    for (unsigned i = 0; i < opt.propertyCount; ++i) {
      auto value = full();
      integer(value, 32, 4);
      integer(value, 16, 4);
      if (i == 1 && !opt.auxiliaryType.empty()) {
        value = full();
        string(value, opt.auxiliaryType);
        append(properties, box(bmffType("auxC"), value, opt.extended));
      } else {
        append(properties, box(opt.propertyType, value, opt.extended));
      }
    }

    // Keep property associations one-based, including the optional wide index form.
    auto iprp = box(bmffType("ipco"), properties, opt.extended);
    auto associations = full(opt.associationVersion, opt.wideProperties ? 1 : 0);
    integer(associations, opt.auxiliaryType.empty() ? 1 : 2, 4);
    integer(associations, opt.idBase + 1, opt.associationVersion == 0 ? 2 : 4);
    integer(associations, opt.auxiliaryType.empty() ? 2 : 1, 1);
    integer(associations, (opt.wideProperties ? 0x8000 : 0x80) | 1, opt.wideProperties ? 2 : 1);
    if (!opt.auxiliaryType.empty()) {
      integer(associations, opt.idBase + 2, opt.associationVersion == 0 ? 2 : 4);
      integer(associations, 1, 1);
    }
    integer(associations, opt.propertyCount, opt.wideProperties ? 2 : 1);
    append(iprp, box(bmffType("ipma"), associations, opt.extended));
    append(result, box(bmffType("iprp"), iprp, opt.extended));

    // Embed idat storage only for fixtures using method-1 construction.
    if (opt.idat)
      append(result, box(bmffType("idat"), payload, opt.extended));
    append(result, opt.extraMeta);
    return box(bmffType("meta"), result, opt.extended);
  };

  // Measure meta before supplying the absolute start of the mdat payload.
  auto metadata = meta(0);
  metadata = meta(ftyp.size() + metadata.size() + opt.extraRoot.size() + headerSize);
  append(ftyp, metadata);
  append(ftyp, opt.extraRoot);
  append(ftyp, box(bmffType("mdat"), payload, opt.extended, opt.terminal));
  return ftyp;
}

// Parse borrowed fixture bytes with optional resource limits.
BmffDocument parse(const Bytes& bytes, const BmffLimits& limits = {}) {
  MemIo input(bytes.data(), bytes.size());
  return parseBmff(input, limits);
}

// Assert that a malformed fixture is rejected by the container parser.
void rejects(const Bytes& bytes) {
  EXPECT_THROW(parse(bytes), Error);
}

// Gather item bytes independently from their checked absolute source ranges.
Bytes payload(const Bytes& file, const BmffItem& item) {
  Bytes result;
  for (const auto& extent : item.location.extents) {
    if (extent.source.offset > file.size() || extent.source.size > file.size() - extent.source.offset)
      throw std::runtime_error("out-of-bounds model range");
    result.insert(result.end(), file.begin() + extent.source.offset,
                  file.begin() + extent.source.offset + extent.source.size);
  }
  return result;
}

}  // namespace

// Retain the complete item graph and fragmented payload coordinates.
TEST(BmffModel, retainsAllItemsExtentsAndMetadataAssociations) {
  const auto bytes = fixture();
  const auto document = parse(bytes);
  EXPECT_NO_THROW(enforceHeifWriteSupport(document));
  ASSERT_EQ(document.items.size(), 3u);
  EXPECT_EQ(document.primaryItem, 1u);
  EXPECT_EQ(document.majorBrand, bmffType("heic"));
  EXPECT_EQ(document.infoOrder, (std::vector<uint32_t>{1, 2, 3}));
  EXPECT_EQ(document.locationOrder, document.infoOrder);
  EXPECT_EQ(document.metadataItems(bmffType("Exif")), (std::vector<uint32_t>{2}));
  EXPECT_EQ(document.metadataItems(bmffType("mime")), (std::vector<uint32_t>{3}));
  EXPECT_EQ(document.items.at(3).info.contentType, "application/rdf+xml");
  EXPECT_EQ(document.items.at(2).info.flags, 1u);

  EXPECT_EQ(document.items.at(1).location.extents.size(), 2u);
  EXPECT_EQ(document.items.at(1).location.dataSize, 4u);
  EXPECT_EQ(payload(bytes, document.items.at(1)), (Bytes{'A', 'B', 'C', 'D'}));
  EXPECT_EQ(payload(bytes, document.items.at(2)), (Bytes{'e', 'x', 'i', 'f'}));
  EXPECT_EQ(payload(bytes, document.items.at(3)), (Bytes{'x', 'm', 'p'}));
  ASSERT_EQ(document.associations.size(), 1u);
  EXPECT_EQ(document.associations.front().properties.front().index, 1u);
  EXPECT_TRUE(document.associations.front().properties.front().essential);
  EXPECT_FALSE(document.associations.front().properties.back().essential);
}

// Resolve item records independently of box order and explicit base offsets.
TEST(BmffModel, acceptsLocationBeforeInfoAndExplicitBaseOffsets) {
  Options opt;
  opt.locationFirst = true;
  opt.baseWidth = 4;
  const auto bytes = fixture(opt);
  const auto document = parse(bytes);
  EXPECT_GT(document.items.at(1).location.baseOffset, 0u);

  EXPECT_EQ(payload(bytes, document.items.at(1)), (Bytes{'A', 'B', 'C', 'D'}));
}

// Resolve idat extents from the payload origin, distinct from file offsets.
TEST(BmffModel, resolvesIdatRelativeDataWithoutConfusingItWithFileOffsets) {
  Options opt;
  opt.idat = true;
  opt.locationVersion = 1;
  const auto bytes = fixture(opt);
  const auto document = parse(bytes);
  ASSERT_TRUE(document.itemData.has_value());
  EXPECT_EQ(document.items.at(1).location.constructionMethod, 1u);

  EXPECT_EQ(document.items.at(1).location.extents.front().source.offset, document.itemData->offset);
  EXPECT_EQ(payload(bytes, document.items.at(1)), (Bytes{'A', 'B', 'C', 'D'}));
  EXPECT_NO_THROW(enforceHeifWriteSupport(document));
}

// Preserve version-dependent wide IDs, offsets, and property indices.
TEST(BmffModel, retains32BitIds64BitFieldsAndWidePropertyIndices) {
  Options opt;
  opt.idBase = 65536;
  opt.locationVersion = 2;
  opt.infoVersion = 1;
  opt.entryVersion = 3;
  opt.primaryVersion = 1;
  opt.referenceVersion = 1;
  opt.associationVersion = 1;
  opt.offsetWidth = opt.lengthWidth = opt.baseWidth = opt.indexWidth = 8;
  opt.wideProperties = true;
  opt.propertyCount = 130;

  const auto document = parse(fixture(opt));
  EXPECT_EQ(document.primaryItem, 65537u);
  EXPECT_EQ(document.metadataItems(bmffType("Exif")), (std::vector<uint32_t>{65538}));
  EXPECT_EQ(document.locationFormat.indexSize, 8u);
  EXPECT_EQ(document.associations.front().properties.back().index, 130u);
  EXPECT_EQ(document.items.at(65537).info.version, 3u);
}

// Allow item locations expressed entirely through their base offsets.
TEST(BmffModel, acceptsZeroOffsetWidthAndNonzeroBaseOffsets) {
  Options opt;
  opt.offsetWidth = 0;
  opt.baseWidth = 8;
  const auto bytes = fixture(opt);

  const auto document = parse(bytes);
  EXPECT_EQ(document.locationFormat.offsetSize, 0u);
  EXPECT_EQ(payload(bytes, document.items.at(2)), (Bytes{'e', 'x', 'i', 'f'}));
}

// Retain both extended-size and end-of-file box representations.
TEST(BmffModel, preservesExtendedHeadersAndTerminalMediaBox) {
  Options opt;
  opt.extended = true;
  auto bytes = fixture(opt);

  auto document = parse(bytes);
  EXPECT_EQ(document.boxes.front().headerSize, 16u);
  EXPECT_EQ(payload(bytes, document.items.at(1)), (Bytes{'A', 'B', 'C', 'D'}));

  // A terminal mdat consumes the remainder of the file without an explicit size.
  opt.extended = false;
  opt.terminal = true;
  bytes = fixture(opt);
  document = parse(bytes);
  EXPECT_TRUE(document.boxes.back().extendsToEnd);
  EXPECT_EQ(document.boxes.back().span.size, 19u);
}

// Accept images without metadata and retain metadata lacking primary associations.
TEST(BmffModel, acceptsMetadataFreeFilesAndMissingReferences) {
  Options opt;
  opt.noMetadata = true;
  auto document = parse(fixture(opt));

  EXPECT_EQ(document.items.size(), 1u);
  EXPECT_TRUE(document.metadataItems(bmffType("Exif")).empty());
  EXPECT_NO_THROW(enforceHeifWriteSupport(document));

  // Unassociated metadata remains readable even without an iref box.
  opt.noMetadata = false;
  opt.noReferences = true;
  document = parse(fixture(opt));
  EXPECT_EQ(document.items.size(), 3u);
  EXPECT_TRUE(document.metadataItems(bmffType("Exif")).empty());
}

// Keep shared metadata and interpret cdsc references in their declared direction.
TEST(BmffModel, preservesSharedMetadataAndReferenceDirection) {
  Options opt;
  opt.sharedMetadata = true;
  auto bytes = fixture(opt);
  auto document = parse(bytes);

  EXPECT_EQ(document.references.front().from, 2u);
  EXPECT_EQ(document.references.front().to, (std::vector<uint32_t>{1, 3}));
  EXPECT_EQ(document.metadataItems(bmffType("Exif")), (std::vector<uint32_t>{2}));

  // Reverse cdsc to ensure an image-to-metadata edge is not mistaken for metadata.
  const auto cdsc = position(bytes, "cdsc");
  patch(bytes, cdsc + 8, 1, 2);
  patch(bytes, cdsc + 12, 2, 2);
  document = parse(bytes);
  EXPECT_TRUE(document.metadataItems(bmffType("Exif")).empty());
}

// Read opaque structures while refusing to assume they are safe to move.
TEST(BmffModel, retainsOpaqueBoxesButRejectsUnprovedRelocation) {
  Options opt;
  opt.extraMeta = box(bmffType("zzzz"), {'k', 'e', 'e', 'p'});
  auto bytes = fixture(opt);
  auto document = parse(bytes);
  const auto& opaque = document.boxes.at(1).children.back();
  EXPECT_EQ(opaque.type, bmffType("zzzz"));
  EXPECT_EQ(opaque.span.offset, position(bytes, "zzzz"));

  EXPECT_EQ(opaque.span.size, 12u);
  EXPECT_THROW(enforceHeifWriteSupport(document), Error);

  opt.extraMeta.clear();
  opt.extraRoot = box(bmffType("zzzz"), {1, 2, 3});
  EXPECT_THROW(enforceHeifWriteSupport(parse(fixture(opt))), Error);
  opt.extraRoot = box(bmffType("free"), {1, 2, 3});
  EXPECT_NO_THROW(enforceHeifWriteSupport(parse(fixture(opt))));

  opt.extraRoot.clear();
  opt.propertyType = bmffType("zzzz");
  document = parse(fixture(opt));
  EXPECT_EQ(document.properties.front().type, bmffType("zzzz"));
  EXPECT_THROW(enforceHeifWriteSupport(document), Error);
}

// Exclude unsupported brands and compressed primary XMP from the write envelope.
TEST(BmffModel, rejectsDeferredBrandsAndCompressedPrimaryXmp) {
  for (const auto brand : {bmffType("avif"), bmffType("avis"), bmffType("crx "), bmffType("jxl ")}) {
    Options opt;
    opt.brand = brand;

    EXPECT_THROW(enforceHeifWriteSupport(parse(fixture(opt))), Error);
  }

  // A compressed XMP packet is readable structurally but cannot be rewritten safely.
  Options opt;
  opt.encoding = "gzip";
  auto document = parse(fixture(opt));
  EXPECT_EQ(document.items.at(3).info.contentEncoding, "gzip");
  EXPECT_THROW(enforceHeifWriteSupport(document), Error);
}

// Reject every incomplete prefix of an otherwise valid container.
TEST(BmffModel, rejectsEveryTruncatedPrefix) {
  const auto complete = fixture();
  for (size_t length = 0; length < complete.size(); ++length) {
    SCOPED_TRACE(length);
    rejects(Bytes(complete.begin(), complete.begin() + length));
  }
}

// Reject malformed sizes before a child can escape its containing box.
TEST(BmffModel, rejectsInvalidHeadersAndContainerBounds) {
  const auto complete = fixture();
  for (const auto size : {uint64_t{2}, uint64_t{7}, uint64_t{0xffffffff}}) {
    auto bytes = complete;
    patch(bytes, 0, size, 4);
    rejects(bytes);
  }

  // Mutate child bounds independently of top-level size validation.
  auto bytes = complete;
  patch(bytes, position(bytes, "infe"), bytes.size(), 4);
  rejects(bytes);

  bytes = complete;
  patch(bytes, position(bytes, "infe"), 0, 4);
  rejects(bytes);

  Options opt;
  opt.extended = true;
  bytes = fixture(opt);
  patch(bytes, 8, std::numeric_limits<uint64_t>::max(), 8);
  rejects(bytes);
}

// Reject unimplemented versions and nonzero reserved flags.
TEST(BmffModel, rejectsUnsupportedVersionsAndReservedFlags) {
  for (const auto* type : {"meta", "pitm", "iinf", "infe", "iloc", "iref", "ipma", "hdlr"}) {
    auto bytes = fixture();
    const auto offset = position(bytes, type);
    patch(bytes, offset + 8, 255, 1);
    rejects(bytes);
  }

  // Reserved flags are rejected even when the version is supported.
  auto bytes = fixture();
  patch(bytes, position(bytes, "iloc") + 11, 128, 1);
  rejects(bytes);
}

// Enforce required structure and singleton constraints within each container.
TEST(BmffModel, rejectsMissingMandatoryBoxesAndDuplicateSingletons) {
  for (const auto* type : {"ftyp", "meta", "hdlr", "pitm", "iinf", "iloc", "iprp", "ipco"}) {
    auto bytes = fixture();
    const auto offset = position(bytes, type);
    patch(bytes, offset + 4, bmffType("free"), 4);
    rejects(bytes);
  }

  // Duplicate singleton boxes must fail at both nested and root levels.
  Options opt;
  auto duplicate = full();
  integer(duplicate, 1, 2);
  opt.extraMeta = box(bmffType("pitm"), duplicate);
  rejects(fixture(opt));

  opt.extraMeta.clear();
  opt.extraRoot = box(bmffType("ftyp"), Bytes(8));
  rejects(fixture(opt));
}

// Require unique nonzero IDs with both description and location records.
TEST(BmffModel, rejectsDuplicateMissingAndZeroItemIds) {
  for (const auto id : {0, 1, 42}) {
    auto bytes = fixture();
    const auto firstInfo = position(bytes, "infe");
    patch(bytes, firstInfo + 12, id, 2);
    if (id == 1) {
      const auto firstLocation = position(bytes, "iloc");
      patch(bytes, firstLocation + 38, 1, 2);  // Second location follows two primary extents.
    }
    rejects(bytes);
  }
}

// Validate every item reference and property index after parsing.
TEST(BmffModel, rejectsDanglingPrimaryReferencesAndProperties) {
  auto bytes = fixture();
  patch(bytes, position(bytes, "pitm") + 12, 42, 2);
  rejects(bytes);

  bytes = fixture();
  patch(bytes, position(bytes, "cdsc") + 8, 42, 2);
  rejects(bytes);

  bytes = fixture();
  patch(bytes, position(bytes, "cdsc") + 12, 42, 2);
  rejects(bytes);

  bytes = fixture();
  patch(bytes, position(bytes, "ipma") + 16, 42, 2);
  rejects(bytes);

  bytes = fixture();
  patch(bytes, position(bytes, "ipma") + 19, 127, 1);
  rejects(bytes);

  bytes = fixture();
  patch(bytes, position(bytes, "ipma") + 19, 0x80, 1);
  rejects(bytes);
}

// Reject declared counts that exceed bounded input or disagree with its contents.
TEST(BmffModel, rejectsHugeCountsAndCountDisagreement) {
  for (const auto count : {0u, 2u, 4u, 65535u}) {
    auto bytes = fixture();
    patch(bytes, position(bytes, "iinf") + 12, count, 2);
    rejects(bytes);
  }

  // Exercise count mismatches in each independently decoded table.
  auto bytes = fixture();
  patch(bytes, position(bytes, "iloc") + 14, 65535, 2);
  rejects(bytes);

  bytes = fixture();
  patch(bytes, position(bytes, "iloc") + 20, 65535, 2);
  rejects(bytes);

  bytes = fixture();
  patch(bytes, position(bytes, "ipma") + 12, 0xffffffff, 4);
  rejects(bytes);

  bytes = fixture();
  patch(bytes, position(bytes, "cdsc") + 10, 65535, 2);
  rejects(bytes);
}

// Reject external, protected, and unsupported item storage.
TEST(BmffModel, rejectsExternalProtectedAndUnsupportedItemConstruction) {
  auto bytes = fixture();
  patch(bytes, position(bytes, "infe") + 14, 1, 2);
  rejects(bytes);

  // Nonzero data references and unsupported construction methods cannot resolve locally.
  bytes = fixture();
  patch(bytes, position(bytes, "iloc") + 18, 1, 2);
  rejects(bytes);

  for (const auto method : {2u, 15u, 0x1001u}) {
    Options opt;
    opt.locationVersion = 1;
    bytes = fixture(opt);
    patch(bytes, position(bytes, "iloc") + 18, method, 2);
    rejects(bytes);
  }

  Options opt;
  opt.idat = true;
  opt.locationVersion = 1;
  bytes = fixture(opt);
  patch(bytes, position(bytes, "idat") + 4, bmffType("free"), 4);
  rejects(bytes);
}

// Reject illegal field widths, empty extents, and out-of-range addressing.
TEST(BmffModel, rejectsInvalidWidthsZeroLengthsAndExtentBounds) {
  for (const auto width : {0u, 1u, 3u, 5u, 9u, 15u}) {
    auto bytes = fixture();
    patch(bytes, position(bytes, "iloc") + 12, 0x40 | width, 1);
    rejects(bytes);
  }

  for (const auto length : {uint64_t{0}, uint64_t{0xffffffff}}) {
    auto bytes = fixture();
    patch(bytes, position(bytes, "iloc") + 26, length, 4);
    rejects(bytes);
  }

  // An extent must stay within media, including when addition would overflow.
  auto bytes = fixture();
  patch(bytes, position(bytes, "iloc") + 22, 0, 4);  // Into the ftyp header, not media.
  rejects(bytes);

  Options opt;
  opt.baseWidth = 8;
  opt.offsetWidth = 8;
  bytes = fixture(opt);
  patch(bytes, position(bytes, "iloc") + 20, std::numeric_limits<uint64_t>::max(), 8);
  rejects(bytes);
}

// Enforce aggregate parsing budgets and bounded terminated strings.
TEST(BmffModel, boundsAggregateResourcesAndStrings) {
  const auto bytes = fixture();
  for (unsigned limit = 0; limit < 8; ++limit) {
    BmffLimits limits;
    switch (limit) {
      case 0:
        limits.maxBoxes = 2;
        break;
      case 1:
        limits.maxItems = 2;
        break;
      case 2:
        limits.maxExtents = 1;
        break;
      case 3:
        limits.maxReferences = 1;
        break;
      case 4:
        limits.maxAssociations = 1;
        break;
      case 5:
        limits.maxStringBytes = 2;
        break;
      case 6:
        limits.maxBytesRead = 8;
        break;
      case 7:
        limits.maxDepth = 1;
        break;
    }
    EXPECT_THROW(parse(bytes, limits), Error) << limit;
  }

  // Check per-string limits and missing terminators separately from aggregate budgets.
  BmffLimits limits;
  limits.maxStringLength = 2;
  EXPECT_THROW(parse(bytes, limits), Error);
  auto malformed = bytes;
  const auto entry = position(malformed, "infe");
  patch(malformed, entry + 25, 'x', 1);  // Remove the image-name terminator.
  rejects(malformed);
}

// Propagate read and seek failures without modifying the borrowed input.
TEST(BmffModel, ioFailuresPropagateWithoutWriting) {
  // Borrow fixture bytes and inject either a short read or a failed seek.
  class FailingIo : public MemIo {
   public:
    using MemIo::read;

    // Keep the fixture storage borrowed for the lifetime of this test double.
    explicit FailingIo(const Bytes& bytes) : MemIo(bytes.data(), bytes.size()) {
    }
    bool failSeek{};

    //! @brief Optionally fail seeks before delegating normal positioning to MemIo.
    int seek(int64_t offset, Position position) override {
      return failSeek ? 1 : MemIo::seek(offset, position);
    }

    //! @brief Inject a short read regardless of the requested field.
    size_t read(byte*, size_t) override {
      return 0;
    }
  };

  const auto bytes = fixture();
  FailingIo input(bytes);
  EXPECT_THROW(parseBmff(input), Error);

  // Exercise seek failure separately from the injected short read.
  input.failSeek = true;
  EXPECT_THROW(parseBmff(input), Error);

  EXPECT_EQ(input.size(), bytes.size());
  EXPECT_EQ(std::memcmp(input.mmap(), bytes.data(), bytes.size()), 0);
}

// Parse a virtual large file using only bounded structural reads.
TEST(BmffModel, resolvesSparse64BitFileWithoutReadingMediaOrAllocatingItsSize) {
  if (sizeof(size_t) < 8)
    GTEST_SKIP() << "BasicIo size() cannot represent this input on a 32-bit host";

  // Expose an 8 GiB virtual file and detect reads outside its stored structure.
  class SparseIo : public MemIo {
   public:
    using MemIo::read;
    Bytes prefix;
    Bytes mediaHeader;
    uint64_t mediaOffset{uint64_t{1} << 33};
    uint64_t cursor{};
    size_t bytesRead{};
    bool forbiddenRead{};

    //! @brief Report the virtual length without allocating the media payload.
    size_t size() const override {
      return static_cast<size_t>(mediaOffset + 19);
    }

    //! @brief Accept bounded absolute seeks within the virtual file.
    int seek(int64_t offset, Position origin) override {
      if (origin != beg || offset < 0 || static_cast<uint64_t>(offset) > size())
        return 1;
      cursor = static_cast<uint64_t>(offset);
      return 0;
    }

    //! @brief Serve only stored structure and record any attempted media read.
    size_t read(byte* output, size_t count) override {
      const auto relative = cursor >= mediaOffset ? cursor - mediaOffset : cursor;
      const auto& region = cursor >= mediaOffset ? mediaHeader : prefix;
      if (relative > region.size() || count > region.size() - relative) {
        forbiddenRead = true;
        return 0;
      }
      std::copy_n(region.begin() + relative, count, output);
      cursor += count;
      bytesRead += count;
      return count;
    }
  } input;

  // Place media beyond an extended-size sparse gap exceeding 32-bit offsets.
  Options opt;
  opt.noMetadata = true;
  opt.baseWidth = 8;
  input.prefix = fixture(opt);
  input.prefix.resize(position(input.prefix, "mdat"));
  patch(input.prefix, position(input.prefix, "iloc") + 20, input.mediaOffset + 8, 8);
  const auto gap = input.mediaOffset - input.prefix.size();
  integer(input.prefix, 1, 4);
  integer(input.prefix, bmffType("free"), 4);
  integer(input.prefix, gap, 8);
  integer(input.mediaHeader, 19, 4);
  integer(input.mediaHeader, bmffType("mdat"), 4);

  // Parse only structural bytes and resolve the large absolute media position.
  const auto document = parseBmff(input);
  EXPECT_NO_THROW(enforceHeifWriteSupport(document));
  EXPECT_EQ(document.fileSize, input.mediaOffset + 19);
  EXPECT_EQ(document.items.at(1).location.extents.at(0).source, (BmffSpan{input.mediaOffset + 8, 2}));
  EXPECT_EQ(document.items.at(1).location.extents.at(1).source, (BmffSpan{input.mediaOffset + 14, 2}));
  EXPECT_EQ(document.boxes.at(2).headerSize, 16u);
  EXPECT_EQ(document.boxes.at(2).span.size, gap);

  EXPECT_FALSE(input.forbiddenRead);
  EXPECT_LT(input.bytesRead, 1024u);
}

// Check UUID header sizes and the known Canon container structure.
TEST(BmffModel, validatesUuidHeadersAndKnownCanonContainer) {
  Options opt;
  const Bytes uuid{0x85, 0xc0, 0xb6, 0x87, 0x82, 0x0f, 0x11, 0xe0, 0x81, 0x11, 0xf4, 0xce, 0x46, 0x2b, 0x6a, 0x48};
  Bytes content = uuid;
  append(content, box(bmffType("CNCV"), {'v', '1', 0}));
  opt.extraMeta = box(bmffType("uuid"), content, true);
  const auto document = parse(fixture(opt));
  const auto& canon = document.boxes.at(1).children.back();
  EXPECT_EQ(canon.headerSize, 32u);
  EXPECT_EQ(canon.children.size(), 1u);
  EXPECT_EQ(canon.payload().size, 11u);
  EXPECT_NO_THROW(enforceHeifWriteSupport(document));
  // An unknown UUID stays opaque, even if its content resembles boxes.
  content[0] = 0;
  opt.extraMeta = box(bmffType("uuid"), content);
  auto unknown = parse(fixture(opt));
  EXPECT_TRUE(unknown.boxes.at(1).children.back().children.empty());
  EXPECT_THROW(enforceHeifWriteSupport(unknown), Error);

  opt.extraMeta = box(bmffType("uuid"), Bytes(15));
  rejects(fixture(opt));

  content = uuid;
  append(content, box(bmffType("zzzz"), {}));
  opt.extraMeta = box(bmffType("uuid"), content);
  EXPECT_THROW(enforceHeifWriteSupport(parse(fixture(opt))), Error);
}

// Accept local data references while rejecting external storage and bad counts.
TEST(BmffModel, acceptsSelfContainedDataReferenceAndRejectsExternalReferences) {
  Options opt;
  auto dataReferences = full();
  integer(dataReferences, 1, 4);
  append(dataReferences, box(bmffType("url "), full(0, 1)));
  opt.extraMeta = box(bmffType("dinf"), box(bmffType("dref"), dataReferences));
  auto bytes = fixture(opt);
  EXPECT_NO_THROW(enforceHeifWriteSupport(parse(bytes)));

  // Flip the self-contained flag to turn the same reference into external storage.
  patch(bytes, position(bytes, "url ") + 11, 0, 1);
  rejects(bytes);

  bytes = fixture(opt);
  patch(bytes, position(bytes, "dref") + 12, 2, 4);
  rejects(bytes);
}

// Allow shared bytes on read while enforcing individual data-box boundaries.
TEST(BmffModel, acceptsSharedPayloadRangesButRejectsRangesCrossingDataBounds) {
  Options opt;
  opt.baseWidth = 4;
  auto bytes = fixture(opt);
  const auto loc = position(bytes, "iloc");
  // Give the second primary extent the same range as the first.
  patch(bytes, loc + 34, 0, 4);
  auto document = parse(bytes);
  EXPECT_EQ(document.items.at(1).location.extents.at(0).source, document.items.at(1).location.extents.at(1).source);
  EXPECT_EQ(payload(bytes, document.items.at(1)), (Bytes{'A', 'B', 'A', 'B'}));

  // Reject extents crossing either idat or mdat boundaries.
  opt.idat = true;
  opt.locationVersion = 1;
  bytes = fixture(opt);
  patch(bytes, position(bytes, "iloc") + 28, 10, 4);  // length 2 crosses idat's 11 bytes.
  rejects(bytes);

  bytes = fixture();
  // Even an extent inside the file must stay within its own mdat payload.
  patch(bytes, position(bytes, "iloc") + 22, position(bytes, "mdat") + 7, 4);
  rejects(bytes);
}

// Check span arithmetic at boundary values without allocating a large file.
TEST(BmffModel, validatesSpanArithmeticIndependentlyOfInputSize) {
  BmffBox box;
  box.headerSize = 8;
  box.span = {std::numeric_limits<uint64_t>::max() - 4, 8};
  EXPECT_THROW(static_cast<void>(box.payload()), Error);

  box.span = {0, 7};
  EXPECT_THROW(static_cast<void>(box.payload()), Error);

  box.span = {100, 8};
  EXPECT_EQ(box.payload(), (BmffSpan{108, 0}));
}

// Keep the generic BMFF image interface read-only.
TEST(BmffModel, leavesProductionBmffWriterDisabled) {
  const auto bytes = fixture();
  auto input = std::make_unique<MemIo>(bytes.data(), bytes.size());
  BmffImage image(std::move(input), defaultImageCtorParams(false));
  EXPECT_EQ(image.checkMode(mdExif), amRead);
  EXPECT_EQ(image.checkMode(mdXmp), amRead);
  EXPECT_THROW(image.writeMetadata(), Error);
  EXPECT_EQ(image.io().size(), bytes.size());
  EXPECT_EQ(std::memcmp(image.io().mmap(), bytes.data(), bytes.size()), 0);
}

// Describe a real-file item inventory used independently of the parser output.
struct CorpusCase {
  const char* file;
  uint32_t primary;
  size_t count;
  uint32_t exif;
  uint32_t xmp;
  bool heif;
};

// Run the same structural and editing checks over each corpus inventory.
class BmffCorpus : public testing::TestWithParam<CorpusCase> {};

// Match real-file item inventories independently recorded for the corpus.
TEST_P(BmffCorpus, agreesWithIndependentItemInventory) {
  const auto& expected = GetParam();
  FileIo input(std::string(TESTDATA_PATH) + "/" + expected.file);
  ASSERT_EQ(input.open("rb"), 0);

  const auto document = parseBmff(input);
  EXPECT_EQ(document.primaryItem, expected.primary);
  EXPECT_EQ(document.items.size(), expected.count);
  EXPECT_EQ(document.infoOrder.size(), expected.count);
  EXPECT_EQ(document.locationOrder.size(), expected.count);

  const auto exif = document.metadataItems(bmffType("Exif"));
  const auto xmp = document.metadataItems(bmffType("mime"));
  EXPECT_EQ(exif, expected.exif ? std::vector<uint32_t>{expected.exif} : std::vector<uint32_t>{});
  EXPECT_EQ(xmp, expected.xmp ? std::vector<uint32_t>{expected.xmp} : std::vector<uint32_t>{});

  // Compare write eligibility with the independent format classification.
  if (expected.heif) {
    EXPECT_NO_THROW(enforceHeifWriteSupport(document));
  } else {
    EXPECT_THROW(enforceHeifWriteSupport(document), Error);
  }
}

// Use existing corpus files with independently recorded item counts and IDs.
INSTANTIATE_TEST_SUITE_P(
    ExistingFixtures, BmffCorpus,
    testing::Values(CorpusCase{"Stonehenge.heic", 1, 3, 2, 3, true}, CorpusCase{"IMG_3578.heic", 49, 51, 51, 0, true},
                    CorpusCase{"2021-02-13-1929.heic", 49, 51, 51, 0, true},
                    CorpusCase{"Canon.HIF", 1, 8, 768, 769, true}, CorpusCase{"Sony.HIF", 10, 15, 14, 15, true},
                    CorpusCase{"heic.heic", 20004, 4, 0, 0, true},
                    CorpusCase{"issue_9292_sony_1_top_left.HIF", 1, 3, 2, 3, true},
                    CorpusCase{"issue_9292_sony_3_bottom_right.HIF", 1, 3, 2, 3, true},
                    CorpusCase{"issue_9292_sony_6_right_top.HIF", 1, 3, 2, 3, true},
                    CorpusCase{"issue_9292_sony_8_left-bottom.HIF", 1, 3, 2, 3, true},
                    CorpusCase{"avif.avif", 1, 2, 2, 0, false}, CorpusCase{"avif_exif_xmp.avif", 1, 3, 2, 3, false},
                    CorpusCase{"avif_metadata2.avif", 1, 3, 2, 3, false}));

// Reject non-item formats and malformed inputs from the regression corpus.
TEST(BmffModel, rejectsNonItemFormatsAndMalformedRegressionFiles) {
  for (const auto* file : {"Canon-R6-pruned.CR3", "Reagan.jxl", "issue_2233_poc1.jxl", "issue_2233_poc2.jxl",
                           "issue_1793_poc.heic", "pr_2612_poc.heic"}) {
    SCOPED_TRACE(file);
    FileIo input(std::string(TESTDATA_PATH) + "/" + file);
    ASSERT_EQ(input.open("rb"), 0);
    EXPECT_THROW(parseBmff(input), Error);
  }
}

namespace {

// Rewrite into separate memory and verify the borrowed source remains unchanged.
Bytes rewritten(const Bytes& bytes, const BmffMetadataUpdate& update = {}) {
  MemIo input(bytes.data(), bytes.size());
  MemIo output;
  rewriteBmff(input, output, parseBmff(input), update);

  EXPECT_EQ(input.size(), bytes.size());
  EXPECT_EQ(std::memcmp(input.mmap(), bytes.data(), bytes.size()), 0);
  return Bytes(output.mmap(), output.mmap() + output.size());
}

// Search all file bytes for a canary, including unreferenced storage.
bool contains(const Bytes& bytes, std::string_view text) {
  return std::search(bytes.begin(), bytes.end(), text.begin(), text.end()) != bytes.end();
}

// Append unreachable media bytes to test physical cleanup during rewriting.
Bytes withOrphanedBytes(Bytes bytes) {
  const std::string_view canary = "UNREACHABLE_PRIVATE_METADATA_296bc8";

  const auto media = position(bytes, "mdat");
  bytes.insert(bytes.end(), canary.begin(), canary.end());
  patch(bytes, media, bytes.size() - media, 4);
  return bytes;
}

// Assert rejection occurs before output is written or source bytes change.
void rejectsRewrite(const Bytes& bytes, const BmffMetadataUpdate& update = {}) {
  MemIo input(bytes.data(), bytes.size());
  MemIo output;
  const auto document = parseBmff(input);

  EXPECT_THROW(rewriteBmff(input, output, document, update), Error);
  EXPECT_EQ(output.size(), 0u);
  EXPECT_EQ(input.size(), bytes.size());
  EXPECT_EQ(std::memcmp(input.mmap(), bytes.data(), bytes.size()), 0);
}

}  // namespace

// Discard unreachable bytes and padding while preserving all retained payloads.
TEST(BmffRewrite, compactsOrphansAndPaddingWhilePreservingEveryRetainedItem) {
  Options opt;
  opt.extraMeta = box(bmffType("free"), {'P', 'R', 'I', 'V', 'A', 'T', 'E'});
  opt.extraRoot = box(bmffType("skip"), {'S', 'E', 'C', 'R', 'E', 'T'});

  const auto source = withOrphanedBytes(fixture(opt));
  const auto original = parse(source);

  const auto bytes = rewritten(source);
  const auto output = parse(bytes);

  // Check canaries across the entire output, then compare every retained payload.
  EXPECT_LT(bytes.size(), source.size());
  EXPECT_FALSE(contains(bytes, "UNREACHABLE_PRIVATE_METADATA_296bc8"));
  EXPECT_FALSE(contains(bytes, "PRIVATE"));
  EXPECT_FALSE(contains(bytes, "SECRET"));

  EXPECT_EQ(output.primaryItem, original.primaryItem);

  for (const auto& [id, item] : original.items)
    EXPECT_EQ(payload(source, item), payload(bytes, output.items.at(id)));

  // A second compaction must be byte-for-byte stable.
  EXPECT_EQ(rewritten(bytes), bytes);
}

// Remove primary metadata records, references, and obsolete payload bytes.
TEST(BmffRewrite, removesMetadataPayloadsAndTheirReferences) {
  const auto source = withOrphanedBytes(fixture());

  BmffMetadataUpdate update;
  update.exif = Bytes{};
  update.xmp = Bytes{};

  const auto bytes = rewritten(source, update);
  const auto output = parse(bytes);

  // Check both graph removal and absence of the discarded payload bytes.
  ASSERT_EQ(output.items.size(), 1u);
  EXPECT_TRUE(output.references.empty());
  EXPECT_TRUE(output.metadataItems(bmffType("Exif")).empty());
  EXPECT_TRUE(output.metadataItems(bmffType("mime")).empty());

  EXPECT_EQ(payload(bytes, output.items.at(1)), (Bytes{'A', 'B', 'C', 'D'}));
  ASSERT_EQ(output.mediaData.size(), 1u);
  EXPECT_EQ(output.mediaData.front().size, 4u);

  EXPECT_FALSE(contains(bytes, "exif"));
  EXPECT_FALSE(contains(bytes, "xmp"));
  EXPECT_FALSE(contains(bytes, "UNREACHABLE_PRIVATE_METADATA_296bc8"));
}

// Compact idat and discard stale copies from unreferenced media ranges.
TEST(BmffRewrite, repacksIdatAndRemovesUnreferencedMediaCopies) {
  Options opt;
  opt.idat = true;
  opt.locationVersion = 1;
  auto source = fixture(opt);

  const auto loc = position(source, "iloc");
  patch(source, loc + 42, 1, 2);  // Exif construction method.
  patch(source, loc + 48, 2, 4);  // Exif offset into idat.
  patch(source, loc + 58, 1, 2);  // XMP construction method.
  patch(source, loc + 64, 8, 4);  // XMP offset into idat.

  // Clear both metadata categories while keeping the idat-backed image.
  BmffMetadataUpdate update;
  update.exif = Bytes{};
  update.xmp = Bytes{};

  const auto bytes = rewritten(source, update);
  const auto output = parse(bytes);

  ASSERT_TRUE(output.itemData.has_value());
  EXPECT_EQ(output.itemData->size, 4u);
  EXPECT_TRUE(output.mediaData.empty());

  const auto& image = output.items.at(1);
  EXPECT_EQ(image.location.constructionMethod, 1u);
  EXPECT_EQ(image.location.extents.at(0).offset, 0u);
  EXPECT_EQ(image.location.extents.at(1).offset, 2u);
  EXPECT_EQ(payload(bytes, image), (Bytes{'A', 'B', 'C', 'D'}));

  EXPECT_FALSE(contains(bytes, "exif"));
  EXPECT_FALSE(contains(bytes, "xmp"));
}

// Insert primary metadata without discarding independent unassociated items.
TEST(BmffRewrite, insertsIntoMetadataFreeFilesAndPreservesUnassociatedItems) {
  Options opt;
  opt.noMetadata = true;

  BmffMetadataUpdate update;
  update.exif = Bytes{'n', 'e', 'w', 'e', 'x', 'i', 'f'};
  update.xmp = Bytes{'n', 'e', 'w', 'x', 'm', 'p'};

  auto bytes = rewritten(fixture(opt), update);
  auto document = parse(bytes);

  // Check newly allocated primary metadata records and their payloads.
  ASSERT_EQ(document.items.size(), 3u);
  ASSERT_EQ(document.metadataItems(bmffType("Exif")).size(), 1u);
  ASSERT_EQ(document.metadataItems(bmffType("mime")).size(), 1u);
  EXPECT_EQ(payload(bytes, document.items.at(document.metadataItems(bmffType("Exif")).front())), *update.exif);
  EXPECT_EQ(payload(bytes, document.items.at(document.metadataItems(bmffType("mime")).front())), *update.xmp);

  // Keep unassociated original metadata when inserting a new primary association.
  opt.noMetadata = false;
  opt.noReferences = true;
  bytes = rewritten(fixture(opt), update);
  document = parse(bytes);
  EXPECT_EQ(document.items.size(), 5u);
  EXPECT_EQ(payload(bytes, document.items.at(2)), (Bytes{'e', 'x', 'i', 'f'}));
  EXPECT_EQ(payload(bytes, document.items.at(3)), (Bytes{'x', 'm', 'p'}));
}

// Keep repeated metadata edits compact and remove earlier payload canaries.
TEST(BmffRewrite, replacesGrowsAndShrinksWithoutRetainingEarlierPayloads) {
  const auto original = fixture();
  auto current = original;
  size_t smallSize = 0;
  for (unsigned iteration = 0; iteration < 8; ++iteration) {
    // Grow metadata with unique canaries, then shrink it in the same edit cycle.
    const auto previous = "PRIVATE_OLD_VALUE_" + std::to_string(iteration);
    BmffMetadataUpdate update;
    update.exif = Bytes(70000, 'q');
    update.exif->insert(update.exif->end(), previous.begin(), previous.end());
    current = rewritten(current, update);
    EXPECT_TRUE(contains(current, previous));

    update.exif = Bytes{'n', 'e', 'w'};
    current = rewritten(current, update);
    EXPECT_FALSE(contains(current, previous));

    // Earlier payloads must disappear without cumulative file growth.
    const auto output = parse(current);
    EXPECT_EQ(payload(current, output.items.at(1)), (Bytes{'A', 'B', 'C', 'D'}));

    if (iteration == 0)
      smallSize = current.size();
    else
      EXPECT_EQ(current.size(), smallSize);
  }
}

// Reject edits whose discarded bytes are still owned by another item.
TEST(BmffRewrite, rejectsSharedMetadataAndOverlappingRetainedDataBeforeWriting) {
  Options opt;
  opt.sharedMetadata = true;

  BmffMetadataUpdate update;
  update.exif = Bytes{};
  rejectsRewrite(fixture(opt), update);

  update.exif = Bytes{'n', 'e', 'w'};
  rejectsRewrite(fixture(opt), update);

  // A retained image sharing the removed metadata bytes also prevents the edit.
  auto source = fixture();
  const auto loc = position(source, "iloc");
  patch(source, loc + 44, position(source, "mdat") + 8, 4);  // Exif overlaps first image extent.
  rejectsRewrite(source, update);
  // Unmodified sharing is representable and compacted without duplicating bytes.
  const auto bytes = rewritten(source);
  EXPECT_EQ(payload(bytes, parse(bytes).items.at(1)), (Bytes{'A', 'B', 'C', 'D'}));
}

// Rewrite wide fields and unusual box headers without losing item identities.
TEST(BmffRewrite, handlesWideIdsFieldsExtendedAndTerminalHeaders) {
  Options opt;
  opt.idBase = 65536;
  opt.locationVersion = 2;
  opt.infoVersion = 2;
  opt.entryVersion = 3;
  opt.primaryVersion = 1;
  opt.referenceVersion = 1;
  opt.associationVersion = 1;
  opt.offsetWidth = opt.lengthWidth = opt.baseWidth = opt.indexWidth = 8;
  opt.extended = true;
  opt.wideProperties = true;

  BmffMetadataUpdate update;
  update.exif = Bytes{'r', 'e', 'p', 'l', 'a', 'c', 'e'};

  auto bytes = rewritten(fixture(opt), update);
  auto output = parse(bytes);

  // Verify wide IDs and field widths remain consistent after replacement.
  EXPECT_EQ(output.primaryItem, 65537u);
  EXPECT_EQ(output.locationFormat.version, 2u);
  EXPECT_EQ(output.items.at(65537).location.baseOffset, 0u);
  EXPECT_EQ(output.locationFormat.indexSize, 8u);
  EXPECT_EQ(payload(bytes, output.items.at(65537)), (Bytes{'A', 'B', 'C', 'D'}));

  // Exercise a terminal media box separately from extended-size headers.
  opt = Options{};
  opt.terminal = true;
  bytes = rewritten(fixture(opt), update);
  output = parse(bytes);
  EXPECT_FALSE(output.boxes.back().extendsToEnd);
  EXPECT_EQ(output.mediaData.size(), 2u);
}

// Find an unused metadata ID even when the highest ID is already occupied.
TEST(BmffRewrite, reusesAvailableIdWhenLargestIdIsOccupied) {
  Options opt;
  opt.noMetadata = true;
  opt.idBase = std::numeric_limits<uint32_t>::max() - 1;
  opt.locationVersion = 2;
  opt.entryVersion = 3;
  opt.primaryVersion = 1;
  opt.associationVersion = 1;
  BmffMetadataUpdate update;
  update.exif = Bytes{'n', 'e', 'w'};
  const auto bytes = rewritten(fixture(opt), update);
  const auto output = parse(bytes);
  EXPECT_EQ(output.primaryItem, std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(output.metadataItems(bmffType("Exif")), (std::vector<uint32_t>{1}));
}

// Preserve property index positions while removing padding payload bytes.
TEST(BmffRewrite, keepsIndexedPaddingSlotsButDiscardsTheirBodies) {
  Options opt;
  opt.propertyType = bmffType("free");
  const auto bytes = rewritten(fixture(opt));

  const auto output = parse(bytes);
  ASSERT_EQ(output.properties.size(), 2u);
  EXPECT_EQ(output.properties.front().payload().size, 0u);
  EXPECT_EQ(output.associations.front().properties.back().index, 2u);
}

// Stop on staging I/O failures and preserve the original source bytes.
TEST(BmffRewrite, propagatesShortWritesAndReadFailuresWithoutMutatingSource) {
  // Limit staging capacity to force a short write during serialization.
  class ShortOutput : public MemIo {
   public:
    using MemIo::write;
    size_t remaining{70};

    //! @brief Accept only the bytes that fit in the simulated output capacity.
    size_t write(const byte* bytes, size_t count) override {
      const auto n = std::min(count, remaining);
      remaining -= n;
      return MemIo::write(bytes, n);
    }
  } output;

  const auto source = fixture();
  MemIo input(source.data(), source.size());
  const auto document = parseBmff(input);

  EXPECT_THROW(rewriteBmff(input, output, document), Error);
  EXPECT_EQ(input.size(), source.size());
  EXPECT_EQ(std::memcmp(input.mmap(), source.data(), source.size()), 0);

  // Borrow valid fixture bytes but fail payload reads during rewriting.
  class ShortInput : public MemIo {
   public:
    using MemIo::read;

    // Keep the source fixture borrowed for this failure test.
    explicit ShortInput(const Bytes& bytes) : MemIo(bytes.data(), bytes.size()) {
    }

    //! @brief Inject an empty read instead of returning the requested source bytes.
    size_t read(byte*, size_t) override {
      return 0;
    }
  } failing(source);

  // Repeat the failure check with unreadable source payloads and an empty sink.
  MemIo other;
  EXPECT_THROW(rewriteBmff(failing, other, document), Error);
  EXPECT_EQ(std::memcmp(failing.mmap(), source.data(), source.size()), 0);
}

// Reject a prepared output whose copied payload differs from the source.
TEST(BmffRewrite, detectsCorruptedPreparedOutput) {
  // Corrupt a staged payload to exercise post-write byte verification.
  class CorruptOutput : public MemIo {
   public:
    using MemIo::write;

    //! @brief Flip a byte in the recognized payload before storing the output chunk.
    size_t write(const byte* bytes, size_t count) override {
      Bytes data(bytes, bytes + count);
      if (contains(data, "ABexifCDxmp"))
        data.back() ^= 1;
      return MemIo::write(data.data(), data.size());
    }
  } output;

  const auto source = fixture();
  MemIo input(source.data(), source.size());

  EXPECT_THROW(rewriteBmff(input, output, parseBmff(input)), Error);
  EXPECT_EQ(std::memcmp(input.mmap(), source.data(), source.size()), 0);
}

// Promote offsets beyond 32 bits while bounding copy buffers and allocation.
TEST(BmffRewrite, promotesOutputOffsetsAndStreamsLargePayloadsWithBoundedIo) {
  if (sizeof(size_t) < 8)
    GTEST_SKIP() << "BasicIo size() cannot represent this input on a 32-bit host";

  // Represent a large seekable file as stored segments with implicit zero-filled gaps.
  class SparseIo : public MemIo {
   public:
    using MemIo::read;
    using MemIo::write;
    std::map<uint64_t, Bytes> segments;
    uint64_t length{};
    uint64_t position{};
    size_t largestWrite{};
    size_t writeCalls{};

    //! @brief Report the virtual file length without materializing its gaps.
    size_t size() const override {
      return static_cast<size_t>(length);
    }

    //! @brief Return the current absolute position in the virtual file.
    size_t tell() const override {
      return static_cast<size_t>(position);
    }

    //! @brief Accept bounded absolute seeks in the sparse stream.
    int seek(int64_t offset, Position origin) override {
      if (origin != beg || offset < 0 || static_cast<uint64_t>(offset) > length)
        return 1;
      position = static_cast<uint64_t>(offset);
      return 0;
    }

    //! @brief Read stored segments over a zero-filled representation of sparse gaps.
    size_t read(byte* output, size_t count) override {
      if (position > length || count > length - position)
        return 0;

      // Fill gaps with zeros, then overlay only intersecting stored segments.
      std::memset(output, 0, count);
      auto it = segments.upper_bound(position);
      if (it != segments.begin())
        --it;
      for (; it != segments.end() && it->first < position + count; ++it) {
        const auto start = std::max(position, it->first);
        const auto end = std::min<uint64_t>(position + count, it->first + it->second.size());
        if (start < end)
          std::memcpy(output + start - position, it->second.data() + start - it->first, end - start);
      }

      position += count;
      return count;
    }

    //! @brief Track write bounds and retain only chunks needed to reparse the output.
    size_t write(const byte* data, size_t count) override {
      largestWrite = std::max(largestWrite, count);
      ++writeCalls;
      // Only the large zero-filled media payload uses full 64 KiB chunks here.
      // Store all structural and short payload writes for the output reparse.
      if (count != 64 * 1024)
        segments.emplace(position, Bytes(data, data + count));
      else if (data[0] != 0 || data[count - 1] != 0)
        throw std::runtime_error("unexpected nonzero virtual media");

      position += count;
      length = std::max(length, position);
      return count;
    }
  } input, output;

  // Construct a small prefix describing a virtual image larger than 4 GiB.
  Options opt;
  opt.noMetadata = true;
  opt.offsetWidth = 0;
  opt.lengthWidth = opt.baseWidth = 8;
  auto prefix = fixture(opt);
  const auto media = position(prefix, "mdat");
  prefix.resize(media);

  const uint64_t imageSize = (uint64_t{5} << 30) + 7;
  patch(prefix, position(prefix, "iloc") + 20, media + 16, 8);
  patch(prefix, position(prefix, "iloc") + 30, imageSize, 8);
  integer(prefix, 1, 4);
  integer(prefix, bmffType("mdat"), 4);
  integer(prefix, imageSize + 16, 8);
  input.length = prefix.size() + imageSize;
  input.segments.emplace(0, prefix);

  const auto original = parseBmff(input);

  // Place new metadata beyond the image so output offsets must widen.
  BmffMetadataUpdate update;
  update.exif = Bytes{'n', 'e', 'w', '6', '4'};
  rewriteBmff(input, output, original, update);

  const auto document = parseBmff(output);

  // Verify promoted locations and bounded streaming without allocating the image.
  EXPECT_EQ(document.locationFormat.offsetSize, 8u);
  EXPECT_EQ(document.locationFormat.lengthSize, 8u);
  EXPECT_EQ(document.items.at(1).location.dataSize, imageSize);
  const auto exif = document.metadataItems(bmffType("Exif")).front();
  EXPECT_GT(document.items.at(exif).location.extents.front().offset, std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(readBmffItem(output, document.items.at(exif)), *update.exif);

  EXPECT_LE(output.largestWrite, 64u * 1024);
  EXPECT_EQ(input.writeCalls, 0u);
  EXPECT_LT(output.segments.size(), 100u);

  // A source that fits the seek contract can still overflow after metadata growth.
  const auto maximum = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
  patch(prefix, position(prefix, "iloc") + 30, maximum - prefix.size(), 8);
  patch(prefix, media + 8, maximum - media, 8);
  input.segments.at(0) = prefix;
  input.length = maximum;
  const auto tooLarge = parseBmff(input);
  SparseIo untouched;
  EXPECT_THROW(rewriteBmff(input, untouched, tooLarge, update), Error);
  EXPECT_EQ(untouched.writeCalls, 0u);
  EXPECT_EQ(input.writeCalls, 0u);
}

// Reject unsupported layouts and unsafe output streams before writing.
TEST(BmffRewrite, rejectsUnsupportedLayoutsAndNonemptySinksBeforeWriting) {
  Options opt;
  opt.extraMeta = box(bmffType("zzzz"), {1, 2, 3});
  rejectsRewrite(fixture(opt));

  opt.extraMeta.clear();
  opt.brand = bmffType("avif");
  rejectsRewrite(fixture(opt));

  // Reject both aliased input/output and a separate nonempty staging stream.
  auto source = fixture();
  MemIo input(source.data(), source.size());
  MemIo output(source.data(), source.size());
  const auto document = parseBmff(input);
  EXPECT_THROW(rewriteBmff(input, input, document), Error);
  EXPECT_THROW(rewriteBmff(input, output, document), Error);
  EXPECT_EQ(std::memcmp(input.mmap(), source.data(), source.size()), 0);
  EXPECT_EQ(std::memcmp(output.mmap(), source.data(), source.size()), 0);
}

// Preserve corpus payloads on supported rewrites and reject other formats.
TEST_P(BmffCorpus, preservesOrRejectsCorpusAccordingToWriteEnvelope) {
  const auto& expected = GetParam();
  FileIo input(std::string(TESTDATA_PATH) + "/" + expected.file);
  ASSERT_EQ(input.open("rb"), 0);
  const auto original = parseBmff(input);
  MemIo output;
  if (!expected.heif) {
    EXPECT_THROW(rewriteBmff(input, output, original), Error);
    EXPECT_EQ(output.size(), 0u);
    return;
  }

  // A preserving rewrite must keep every independently inventoried item payload.
  rewriteBmff(input, output, original);
  const auto document = parseBmff(output);
  for (const auto& [id, item] : original.items)
    EXPECT_EQ(readBmffItem(input, item), readBmffItem(output, document.items.at(id))) << id;

  // Clear metadata and compare every remaining item against the original stream.
  BmffMetadataUpdate update;
  update.exif = Bytes{};
  update.xmp = Bytes{};
  MemIo stripped;

  rewriteBmff(input, stripped, original, update);
  const auto cleaned = parseBmff(stripped);

  EXPECT_TRUE(cleaned.metadataItems(bmffType("Exif")).empty());
  EXPECT_TRUE(cleaned.metadataItems(bmffType("mime")).empty());

  for (const auto& [id, item] : cleaned.items)
    EXPECT_EQ(readBmffItem(input, original.items.at(id)), readBmffItem(stripped, item)) << id;
}

namespace {

// Encode fresh TIFF metadata behind the HEIF Exif item offset prefix.
Bytes tiffItem(ExifData exif, const XmpData& xmp = {}, ByteOrder order = littleEndian) {
  MemIo output;
  TiffParser::encode(output, nullptr, 0, order, exif, IptcData{}, xmp);

  Bytes bytes(4, 0);
  bytes.insert(bytes.end(), output.mmap(), output.mmap() + output.size());
  return bytes;
}

// Build a metadata-free HEIF fixture, then add independently selected metadata.
Bytes heifWithMetadata(const ExifData& exif = {}, std::string_view xmp = {}, ByteOrder order = littleEndian) {
  Options options;
  options.noMetadata = true;
  BmffMetadataUpdate update;

  if (!exif.empty())
    update.exif = tiffItem(exif, {}, order);
  if (!xmp.empty())
    update.xmp = Bytes(xmp.begin(), xmp.end());
  return rewritten(fixture(options), update);
}

// Open owned fixture storage through ImageFactory and read its metadata.
Image::UniquePtr openHeif(const Bytes& bytes) {
  auto owned = std::make_unique<MemIo>();
  owned->write(bytes.data(), bytes.size());
  auto image = ImageFactory::open(std::move(owned));
  if (image->imageType() != ImageType::heif)
    throw std::runtime_error("HEIF was not classified separately");
  image->readMetadata();
  return image;
}

// Construct a minimal raw XMP packet containing a controlled source value.
std::string rawXmpSource(std::string_view value) {
  return "<x:xmpmeta xmlns:x='adobe:ns:meta/'><rdf:RDF "
         "xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'>"
         "<rdf:Description rdf:about='' xmlns:dc='http://purl.org/dc/elements/1.1/' "
         "dc:source='" +
         std::string(value) + "'/></rdf:RDF></x:xmpmeta>";
}

// Set equivalent XMP data with or without the structured XMP toolkit.
void setHeifXmpSource(Image& image, const std::string& value) {
#ifdef EXV_HAVE_XMP_TOOLKIT
  image.xmpData()["Xmp.dc.source"] = value;
#else
  image.setXmpPacket(rawXmpSource(value));
#endif
}

// Read the complete current image stream for preservation and canary checks.
Bytes imageBytes(Image& image) {
  auto& io = image.io();
  io.open();
  auto data = io.read(io.size());

  return Bytes(data.c_data(), data.c_data() + data.size());
}

}  // namespace

// Exercise public metadata edits while preserving encoded image bytes.
TEST(HeifImage, addsGrowsShrinksDeletesAndPreservesEncodedImage) {
  auto image = openHeif(heifWithMetadata());
  EXPECT_EQ(image->checkMode(mdExif), amReadWrite);
  EXPECT_EQ(image->checkMode(mdXmp), amReadWrite);
  EXPECT_EQ(image->checkMode(mdIptc), amRead);

  size_t smallSize = 0;
  for (unsigned cycle = 0; cycle < 4; ++cycle) {
    // Grow metadata using a unique canary for each iteration.
    const auto canary = "PRIVATE_HEIF_DESCRIPTION_" + std::to_string(cycle);
    image->exifData()["Exif.Image.ImageDescription"] = std::string(70000, 'v') + canary;
    setHeifXmpSource(*image, "Příliš žluťoučký kůň — " + canary);
    ASSERT_NO_THROW(image->writeMetadata());
    auto large = imageBytes(*image);
    EXPECT_TRUE(contains(large, canary));

    // Shrink the same metadata and require the earlier bytes to disappear.
    image->exifData()["Exif.Image.ImageDescription"] = "short";
    setHeifXmpSource(*image, "short");
    ASSERT_NO_THROW(image->writeMetadata());
    auto small = imageBytes(*image);
    EXPECT_FALSE(contains(small, canary));

    auto parsed = parse(small);
    EXPECT_EQ(payload(small, parsed.items.at(parsed.primaryItem)), (Bytes{'A', 'B', 'C', 'D'}));

    if (cycle != 0) {
      EXPECT_EQ(small.size(), smallSize);
    }
    smallSize = small.size();
  }

  // Clear both categories and verify that the image payload remains intact.
  image->clearMetadata();
  ASSERT_NO_THROW(image->writeMetadata());
  auto clean = imageBytes(*image);
  EXPECT_FALSE(contains(clean, "short"));
  EXPECT_TRUE(parse(clean).metadataItems(bmffType("Exif")).empty());
  EXPECT_TRUE(parse(clean).metadataItems(bmffType("mime")).empty());
}

// Preserve a true no-op but compact stale bytes after explicit empty removal.
TEST(HeifImage, preservesNoOpAndCleansExplicitEmptyRemoval) {
  ExifData exif;
  exif["Exif.Image.Artist"] = "original";

  auto bytes = heifWithMetadata(exif);
  auto image = openHeif(bytes);
  image->writeMetadata();
  EXPECT_EQ(imageBytes(*image), bytes);

  // An explicit empty removal must compact orphaned bytes even without active metadata.
  bytes = withOrphanedBytes(heifWithMetadata());
  image = openHeif(bytes);
  image->clearExifData();
  image->clearXmpPacket();
  image->writeMetadata();
  EXPECT_FALSE(contains(imageBytes(*image), "UNREACHABLE_PRIVATE_METADATA_296bc8"));
}

// Serialize fresh TIFF data so removed tags and original slack disappear.
TEST(HeifImage, removesDeletedTagsAndOriginalTiffSlack) {
  ExifData exif;
  exif["Exif.Image.Artist"] = "KEEP_ARTIST";
  exif["Exif.Photo.UserComment"] = "charset=Ascii PRIVATE_REMOVED_USER_COMMENT_841bb0";
  auto item = tiffItem(exif, {}, bigEndian);

  // Plant a canary in TIFF slack that a fresh serialization must discard.
  const std::string slack = "PRIVATE_UNUSED_TIFF_STORAGE_2223f1";
  item.insert(item.end(), slack.begin(), slack.end());
  BmffMetadataUpdate update;
  update.exif = item;
  auto image = openHeif(rewritten(heifWithMetadata(), update));

  // Delete a tag and check the full output for both active and stale private bytes.
  image->exifData().erase(image->exifData().findKey(ExifKey("Exif.Photo.UserComment")));
  image->writeMetadata();
  auto bytes = imageBytes(*image);

  EXPECT_FALSE(contains(bytes, slack));
  EXPECT_FALSE(contains(bytes, "PRIVATE_REMOVED_USER_COMMENT_841bb0"));
  EXPECT_TRUE(contains(bytes, "KEEP_ARTIST"));

  // Verify that fresh serialization preserves the original big-endian TIFF order.
  auto doc = parse(bytes);
  auto tiff = payload(bytes, doc.items.at(doc.metadataItems(bmffType("Exif")).front()));
  ASSERT_GT(tiff.size(), 8u);
  EXPECT_EQ(tiff[4], 'M');
  EXPECT_EQ(tiff[5], 'M');
}

// Apply XMP removal consistently across separate and embedded storage.
TEST(HeifImage, clearsEmbeddedXmpAndPreservesItWhenOnlyExifIsRemoved) {
#ifndef EXV_HAVE_XMP_TOOLKIT
  GTEST_SKIP() << "requires structured XMP decoding; raw packet behavior is tested separately";
#endif
  ExifData exif;
  exif["Exif.Image.Artist"] = "keep artist";
  XmpData xmp;
  xmp["Xmp.dc.source"] = "PRIVATE_EMBEDDED_XMP_350597";
  BmffMetadataUpdate update;
  update.exif = tiffItem(exif, xmp);
  auto bytes = rewritten(heifWithMetadata(), update);
  auto image = openHeif(bytes);
  ASSERT_FALSE(image->xmpData().empty());

  // Clear XMP through the image API and check both possible storage forms.
  image->clearXmpPacket();
  image->writeMetadata();
  auto cleaned = imageBytes(*image);
  EXPECT_FALSE(contains(cleaned, "PRIVATE_EMBEDDED_XMP_350597"));
  EXPECT_TRUE(contains(cleaned, "keep artist"));

  // Deleting the embedded XML tag must also remove its serialized packet.
  image = openHeif(bytes);
  auto xml = image->exifData().findKey(ExifKey("Exif.Image.XMLPacket"));
  ASSERT_NE(xml, image->exifData().end());
  image->exifData().erase(xml);
  image->writeMetadata();
  EXPECT_FALSE(contains(imageBytes(*image), "PRIVATE_EMBEDDED_XMP_350597"));

  // Removing only Exif must preserve embedded XMP as separate metadata.
  image = openHeif(bytes);
  image->clearExifData();
  image->writeMetadata();
  EXPECT_EQ(image->xmpData()["Xmp.dc.source"].toString(), "PRIVATE_EMBEDDED_XMP_350597");
  EXPECT_TRUE(image->exifData().empty());
}

// Support raw XMP and metadata copying while rejecting unsupported categories.
TEST(HeifImage, usesRawXmpPacketsSetMetadataAndRejectsUnsupportedCategories) {
#ifndef EXV_HAVE_XMP_TOOLKIT
  GTEST_SKIP() << "requires structured XMP decoding; raw packet behavior is tested separately";
#endif
  auto source = openHeif(heifWithMetadata());
  XmpData xmp;
  xmp["Xmp.dc.source"] = "RAW_XMP_říční";
  std::string packet;
  ASSERT_EQ(XmpParser::encode(packet, xmp), 0);

  source->setXmpPacket(packet);
  source->exifData()["Exif.Image.Artist"] = "copied";
  source->writeMetadata();
  auto bytes = imageBytes(*source);
  auto doc = parse(bytes);

  // Check raw packet handling and metadata copying through the public API.
  EXPECT_EQ(payload(bytes, doc.items.at(doc.metadataItems(bmffType("mime")).front())),
            Bytes(packet.begin(), packet.end()));
  auto target = openHeif(heifWithMetadata());
  target->setMetadata(*source);
  target->writeMetadata();
  EXPECT_EQ(target->exifData()["Exif.Image.Artist"].toString(), "copied");
  EXPECT_EQ(target->xmpData()["Xmp.dc.source"].toString(), "RAW_XMP_říční");

  // Unsupported metadata categories must fail without altering the source.
  EXPECT_THROW(target->setIptcData(IptcData{}), Error);
  EXPECT_THROW(target->setComment("comment"), Error);

  target->clearXmpData();
  target->writeMetadata();
  EXPECT_TRUE(target->xmpData().empty());
  EXPECT_FALSE(contains(imageBytes(*target), "RAW_XMP"));
}

// Preserve pending edits on transfer failure and reject unsafe layouts earlier.
TEST(HeifImage, failsBeforeTransferAndDoesNotOverwritePendingEdits) {
  // Record final-transfer attempts and inject a failure before source replacement.
  class TransferIo : public MemIo {
   public:
    // Borrow the fixture bytes used to detect any premature source mutation.
    explicit TransferIo(const Bytes& bytes) : MemIo(bytes.data(), bytes.size()) {
    }

    //! @brief Record the transfer boundary and fail without copying prepared bytes.
    void transfer(BasicIo&) override {
      transferred = true;
      throw Error(ErrorCode::kerImageWriteFailed);
    }
    bool transferred{};
  };

  auto bytes = heifWithMetadata();
  auto source = std::make_unique<TransferIo>(bytes);
  auto* observer = source.get();
  auto image = ImageFactory::open(std::move(source));
  image->readMetadata();

  image->exifData()["Exif.Image.Artist"] = "pending";

  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_TRUE(observer->transferred);
  EXPECT_EQ(imageBytes(*image), bytes);
  EXPECT_EQ(image->exifData()["Exif.Image.Artist"].toString(), "pending");

  // An unsupported container must fail before the final transfer is attempted.
  append(bytes, box(bmffType("moov"), {}));
  source = std::make_unique<TransferIo>(bytes);
  observer = source.get();
  image = ImageFactory::open(std::move(source));
  image->exifData()["Exif.Image.Artist"] = "pending";
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_FALSE(observer->transferred);
  EXPECT_EQ(imageBytes(*image), bytes);
}

// Exercise public HEIF editing against the corpus without changing other items.
TEST_P(BmffCorpus, editsThroughPublicApiAndKeepsOtherItems) {
  const auto& expected = GetParam();
  FileIo original(std::string(TESTDATA_PATH) + "/" + expected.file);
  ASSERT_EQ(original.open(), 0);
  auto data = original.read(original.size());
  Bytes bytes(data.c_data(), data.c_data() + data.size());
  auto image = ImageFactory::open(bytes.data(), bytes.size());

  if (!expected.heif) {
    EXPECT_EQ(image->imageType(), ImageType::bmff);
    EXPECT_THROW(image->writeMetadata(), Error);
    return;
  }

  // Edit primary metadata and compare each unrelated item with the original.
  ASSERT_EQ(image->imageType(), ImageType::heif);
  image->readMetadata();
  image->exifData()["Exif.Image.ImageDescription"] = "PUBLIC_API_CANARY_6f9326";
  setHeifXmpSource(*image, "PUBLIC_API_CANARY_6f9326");
  ASSERT_NO_THROW(image->writeMetadata());

  auto output = imageBytes(*image);
  auto before = parse(bytes), after = parse(output);
  auto exif = before.metadataItems(bmffType("Exif"));
  auto xmp = before.metadataItems(bmffType("mime"));
  for (const auto& [id, item] : before.items) {
    if (std::find(exif.begin(), exif.end(), id) != exif.end() || std::find(xmp.begin(), xmp.end(), id) != xmp.end())
      continue;
    ASSERT_TRUE(after.items.contains(id));
    EXPECT_EQ(payload(bytes, item), payload(output, after.items.at(id)));
  }

  // A true no-op keeps the entire file byte-for-byte unchanged.
  auto first = output;
  image->writeMetadata();
  EXPECT_EQ(imageBytes(*image), first);

  // Repeated large and small edits must not accumulate obsolete payload bytes.
  const auto tagCount = image->exifData().count();
  size_t steadySize = 0;
  for (unsigned cycle = 0; cycle != 3; ++cycle) {
    image->exifData()["Exif.Image.ImageDescription"] = "PUBLIC_API_CANARY_6f9327";
    image->writeMetadata();
    image->exifData()["Exif.Image.ImageDescription"] = "PUBLIC_API_CANARY_6f9326";
    image->writeMetadata();
    EXPECT_EQ(image->exifData().count(), tagCount);
    const auto currentSize = imageBytes(*image).size();
    // Editing Exif alone may separate it from retained XMP into one more mdat.
    EXPECT_LE(currentSize, first.size() + 8);
    if (cycle != 0) {
      EXPECT_EQ(currentSize, steadySize);
    }
    steadySize = currentSize;
  }

  // Clear all supported metadata after the edit sequence.
  image->clearMetadata();
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_FALSE(contains(imageBytes(*image), "PUBLIC_API_CANARY_6f9326"));
}

// Merge primary metadata in file order and replace obsolete copies together.
TEST(HeifImage, readsAndReplacesMultiplePrimaryMetadataItems) {
  ExifData first, second;
  first["Exif.Image.Artist"] = "PRIVATE_FIRST_PRIMARY_450a";
  first["Exif.Photo.DateTimeOriginal"] = "2001:02:03 04:05:06";
  second["Exif.Image.Artist"] = "PRIVATE_SECOND_PRIMARY_917a";
  Options options;
  options.exifPayload = tiffItem(first);
  options.xmpPayload = tiffItem(second);
  options.secondExif = true;
  auto source = fixture(options);
  auto image = openHeif(source);

  // Later primary Exif items take precedence when their tags overlap.
  EXPECT_EQ(image->exifData()["Exif.Image.Artist"].toString(), "PRIVATE_SECOND_PRIMARY_917a");
  EXPECT_EQ(image->exifData()["Exif.Photo.DateTimeOriginal"].toString(), "2001:02:03 04:05:06");

  image->exifData()["Exif.Image.Artist"] = "replacement";
  image->writeMetadata();
  auto bytes = imageBytes(*image);

  EXPECT_EQ(parse(bytes).metadataItems(bmffType("Exif")).size(), 1u);
  EXPECT_FALSE(contains(bytes, "PRIVATE_FIRST_PRIMARY_450a"));
  EXPECT_FALSE(contains(bytes, "PRIVATE_SECOND_PRIMARY_917a"));

  // A metadata item shared with another image cannot be replaced independently.
  options.sharedMetadata = true;
  source = fixture(options);
  image = openHeif(source);
  image->clearExifData();
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(imageBytes(*image), source);
}

// Preserve duplicate tag groups while later metadata items take precedence.
TEST(HeifImage, mergesDuplicateExifGroupsInFileOrder) {
  ExifData first, second;
  first["Exif.Image.Make"] = "retained";
  first["Exif.Image.Artist"] = "old first";
  auto value = Value::create(asciiString);
  value->read("old second");
  first.add(ExifKey("Exif.Image.Artist"), value.get());

  // The later item replaces a whole duplicate-tag group while preserving its order.
  second["Exif.Image.Model"] = "new model";
  second["Exif.Image.Artist"] = "new first";
  value->read("new second");
  second.add(ExifKey("Exif.Image.Artist"), value.get());

  Options options;
  options.secondExif = true;
  options.exifPayload = tiffItem(first);
  options.xmpPayload = tiffItem(second);
  auto image = openHeif(fixture(options));

  // Collect merged values in iteration order to check duplicate preservation.
  std::vector<std::string> artists;
  for (const auto& datum : image->exifData())
    if (datum.key() == "Exif.Image.Artist")
      artists.push_back(datum.toString());

  EXPECT_EQ(artists, (std::vector<std::string>{"new first", "new second"}));
  EXPECT_EQ(image->exifData()["Exif.Image.Make"].toString(), "retained");
  EXPECT_EQ(image->exifData()["Exif.Image.Model"].toString(), "new model");
}

// Edit idat metadata with wide IDs through the public image interface.
TEST(HeifImage, updatesIdatMetadataAndWideItemIds) {
  ExifData exif;
  exif["Exif.Image.Artist"] = "PRIVATE_IDAT_METADATA_7703";

  Options options;
  options.idat = options.metadataIdat = true;
  options.exifPayload = tiffItem(exif);
  options.xmpPayload = tiffItem(exif);
  options.secondExif = true;
  options.idBase = 70000;
  options.locationVersion = 2;
  options.entryVersion = 3;
  options.primaryVersion = options.referenceVersion = options.associationVersion = 1;

  // Write through ImageFactory, then inspect the rewritten item locations directly.
  auto image = openHeif(fixture(options));
  image->exifData()["Exif.Image.Artist"] = "new";
  image->writeMetadata();

  auto bytes = imageBytes(*image);
  auto document = parse(bytes);
  EXPECT_FALSE(contains(bytes, "PRIVATE_IDAT_METADATA_7703"));
  EXPECT_EQ(document.primaryItem, 70001u);
  EXPECT_EQ(payload(bytes, document.items.at(70001)), (Bytes{'A', 'B', 'C', 'D'}));
  EXPECT_EQ(document.items.at(70001).location.constructionMethod, 1u);
}

// Preserve unrelated metadata and exclude AVIF with a generic compatible brand.
TEST(HeifImage, preservesIndependentMetadataAndRejectsGenericAvifBrand) {
  ExifData exif;
  exif["Exif.Image.Artist"] = "INDEPENDENT_METADATA_f880";
  Options options;
  options.exifPayload = tiffItem(exif);
  options.xmpPayload = tiffItem(exif);
  options.secondExif = true;
  options.noReferences = true;

  auto bytes = fixture(options);
  auto image = openHeif(bytes);
  EXPECT_TRUE(image->exifData().empty());

  // Clearing primary metadata must retain independent unassociated metadata items.
  image->clearExifData();
  image->writeMetadata();
  auto result = imageBytes(*image);
  EXPECT_EQ(payload(bytes, parse(bytes).items.at(2)), payload(result, parse(result).items.at(2)));

  image->exifData()["Exif.Image.Artist"] = "primary metadata";
  image->writeMetadata();
  result = imageBytes(*image);
  EXPECT_TRUE(contains(result, "INDEPENDENT_METADATA_f880"));
  EXPECT_EQ(parse(result).metadataItems(bmffType("Exif")).size(), 1u);

  // A generic compatible brand must not make an AVIF file writable as HEIF.
  bytes = heifWithMetadata();
  patch(bytes, 8, bmffType("mif1"), 4);
  patch(bytes, 16, bmffType("avif"), 4);
  image = ImageFactory::open(bytes.data(), bytes.size());
  EXPECT_EQ(image->imageType(), ImageType::bmff);
  EXPECT_EQ(image->checkMode(mdExif), amRead);
  EXPECT_THROW(image->writeMetadata(), Error);
}

// Retain embedded IPTC and thumbnail data during supported Exif edits.
TEST(HeifImage, preservesEmbeddedIptcAndThumbnailDataOnExifEdits) {
  ExifData exif;
  exif["Exif.Image.Artist"] = "original";

  const Bytes thumbnail{0xff, 0xd8, 1, 2, 3, 4, 0xff, 0xd9};
  ExifThumb(exif).setJpegThumbnail(thumbnail.data(), thumbnail.size());

  IptcData iptc;
  iptc["Iptc.Application2.Caption"] = "retained IPTC";

  // Embed IPTC and thumbnail content in the original serialized Exif item.
  MemIo output;
  TiffParser::encode(output, nullptr, 0, littleEndian, exif, iptc, XmpData{});
  BmffMetadataUpdate update;
  update.exif = Bytes(4, 0);
  update.exif->insert(update.exif->end(), output.mmap(), output.mmap() + output.size());
  auto image = openHeif(rewritten(heifWithMetadata(), update));

  image->exifData()["Exif.Image.Artist"] = "new";
  image->writeMetadata();

  // Check preserved embedded content before trying an unsupported IPTC mutation.
  EXPECT_EQ(image->iptcData()["Iptc.Application2.Caption"].toString(), "retained IPTC");
  auto copied = ExifThumbC(image->exifData()).copy();
  EXPECT_EQ(Bytes(copied.c_data(), copied.c_data() + copied.size()), thumbnail);

  auto before = imageBytes(*image);
  image->iptcData()["Iptc.Application2.Caption"] = "unsupported";
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(imageBytes(*image), before);
}

// Remove embedded XMP slack and preserve pending edits during structure printing.
TEST(HeifImage, clearsOrphanedEmbeddedXmpAndPrintsWithoutDiscardingEdits) {
  ExifData exif;
  exif["Exif.Image.Artist"] = "original";
  auto item = tiffItem(exif);

  // Leave an XMP canary in TIFF slack rather than an active XML tag.
  const std::string canary = "PRIVATE_ORPHANED_TIFF_XMP_9c1f";
  item.insert(item.end(), canary.begin(), canary.end());
  BmffMetadataUpdate update;
  update.exif = item;
  auto image = openHeif(rewritten(heifWithMetadata(), update));

  image->exifData()["Exif.Image.Artist"] = "pending";
  std::ostringstream trace;

  // Printing structure must not discard pending in-memory metadata edits.
  EXPECT_NO_THROW(image->printStructure(trace, kpsBasic, 0));
  EXPECT_EQ(image->exifData()["Exif.Image.Artist"].toString(), "pending");

  image->clearXmpPacket();
  image->writeMetadata();
  EXPECT_FALSE(contains(imageBytes(*image), canary));
}

// Reject ambiguous primary XMP encodings without replacing the source.
TEST(HeifImage, rejectsAmbiguousPrimaryXmpEncodingsBeforeTransfer) {
  ExifData exif;
  exif["Exif.Image.Artist"] = "original";
  Options options;
  options.exifPayload = tiffItem(exif);
  const std::string packet =
      "<x:xmpmeta xmlns:x='adobe:ns:meta/'><rdf:RDF "
      "xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'/></x:xmpmeta>";
  options.xmpPayload = Bytes(packet.begin(), packet.end());
  for (const auto* type : {"APPLICATION/RDF+XML", "application/rdf+xml; charset=utf-8", " application/rdf+xml "}) {
    options.mimeType = type;
    auto bytes = fixture(options);
    auto image = ImageFactory::open(bytes.data(), bytes.size());
    image->clearXmpData();
    EXPECT_THROW(image->writeMetadata(), Error);
    EXPECT_EQ(imageBytes(*image), bytes);
  }
}

// Remove Unicode comment bytes in both supported TIFF byte orders.
TEST(HeifImage, removesUnicodeUserCommentsInBothTiffByteOrders) {
  const std::string canary = "PRIVATE_UNICODE_COMMENT_7ead";
  Bytes big, little;
  for (unsigned char c : canary) {
    integer(big, c, 2);
    little.push_back(c);
    little.push_back(0);
  }

  // Search raw bytes for either Unicode encoding, including unreferenced TIFF storage.
  auto has = [](const Bytes& data, const Bytes& needle) {
    return std::search(data.begin(), data.end(), needle.begin(), needle.end()) != data.end();
  };
  for (const auto order : {littleEndian, bigEndian}) {
    ExifData exif;
    exif["Exif.Image.Artist"] = "retained";
    exif["Exif.Photo.UserComment"] = "charset=Unicode " + canary + " žluťoučký";
    auto bytes = heifWithMetadata(exif, {}, order);

    ASSERT_TRUE(has(bytes, big) || has(bytes, little));

    // Delete the user comment and require both raw encodings to disappear.
    auto image = openHeif(bytes);
    image->exifData().erase(image->exifData().findKey(ExifKey("Exif.Photo.UserComment")));
    image->writeMetadata();
    bytes = imageBytes(*image);

    EXPECT_FALSE(has(bytes, big));
    EXPECT_FALSE(has(bytes, little));
  }
}

// Preserve auxiliary and thumbnail relationships without decoding their images.
TEST(HeifImage, preservesAuxiliaryAndThumbnailGraphsAsOpaqueImageData) {
  // A structural fixture: compressed-image decoding is intentionally outside
  // this test. The writer must preserve every image item and relationship.
  for (const auto* uri : {"urn:mpeg:hevc:2015:auxid:1", "urn:mpeg:hevc:2015:auxid:2"}) {
    Options options;
    options.secondExif = true;
    options.auxiliaryType = uri;
    options.exifPayload = Bytes{'A', 'L', 'P', 'H', 'A'};
    options.xmpPayload = Bytes{'T', 'H', 'U', 'M', 'B'};
    auto bytes = fixture(options);
    auto before = parse(bytes);

    // Repurpose the extra fixture items as opaque auxiliary and thumbnail images.
    for (const auto id : {2, 3}) {
      const auto& info = before.items.at(id).info;
      patch(bytes, info.box.offset + 16, bmffType("hvc1"), 4);
    }

    for (const auto& ref : before.references)
      patch(bytes, ref.box.offset + 4, ref.from == 2 ? bmffType("auxl") : bmffType("thmb"), 4);
    before = parse(bytes);

    // Add primary metadata without changing the retained image graph or payloads.
    auto image = openHeif(bytes);
    EXPECT_TRUE(image->exifData().empty());
    image->exifData()["Exif.Image.Artist"] = "new metadata";
    image->writeMetadata();

    auto output = imageBytes(*image);
    const auto after = parse(output);
    for (const auto& [id, item] : before.items)
      EXPECT_EQ(payload(bytes, item), payload(output, after.items.at(id)));
    ASSERT_EQ(after.references.size(), before.references.size() + 1);
    for (size_t i = 0; i != before.references.size(); ++i) {
      EXPECT_EQ(after.references[i].type, before.references[i].type);
      EXPECT_EQ(after.references[i].from, before.references[i].from);
      EXPECT_EQ(after.references[i].to, before.references[i].to);
    }
    EXPECT_EQ(after.associations.size(), before.associations.size());
    EXPECT_TRUE(contains(output, uri));
  }
}

// Preserve raw XMP during Exif edits and remove its bytes on explicit clearing.
TEST(HeifImage, preservesRawXmpDuringExifEditsAndClearsItsStorage) {
  ExifData exif;
  exif["Exif.Image.Artist"] = "artist";
  const auto packet = rawXmpSource("PRIVATE_RAW_PACKET_594e");
  auto image = openHeif(heifWithMetadata(exif, packet));
#ifdef EXV_HAVE_XMP_TOOLKIT
  EXPECT_EQ(image->xmpData()["Xmp.dc.source"].toString(), "PRIVATE_RAW_PACKET_594e");
#else
  EXPECT_EQ(image->xmpPacket(), packet);
#endif

  // An untouched image must preserve even the exact raw packet representation.
  auto before = imageBytes(*image);
  image->writeMetadata();
  EXPECT_EQ(imageBytes(*image), before);

  // Changing Exif alone must retain the separate raw XMP packet.
  image->exifData()["Exif.Image.Artist"] = "edited";
  ASSERT_NO_THROW(image->writeMetadata());
  auto bytes = imageBytes(*image);
  auto document = parse(bytes);
  const auto ids = document.metadataItems(bmffType("mime"));
  ASSERT_EQ(ids.size(), 1u);
  EXPECT_EQ(payload(bytes, document.items.at(ids.front())), Bytes(packet.begin(), packet.end()));

  // Replace the packet, then clear it and inspect all remaining file bytes.
  image->setXmpPacket(rawXmpSource("PRIVATE_REPLACEMENT_PACKET_653c"));
  image->writeMetadata();
  EXPECT_FALSE(contains(imageBytes(*image), "PRIVATE_RAW_PACKET_594e"));

  image->clearXmpData();
  image->writeMetadata();
  EXPECT_FALSE(contains(imageBytes(*image), "PRIVATE_REPLACEMENT_PACKET_653c"));
  EXPECT_TRUE(image->xmpPacket().empty());

  image->setXmpPacket(packet);
  image->writeMetadata();
  image->setXmpData(XmpData{});
  image->writeMetadata();
  EXPECT_FALSE(contains(imageBytes(*image), "PRIVATE_RAW_PACKET_594e"));
}

#ifndef EXV_HAVE_XMP_TOOLKIT

// Handle embedded raw XMP without the toolkit and reject ambiguous merges.
TEST(HeifImage, preservesEmbeddedRawXmpWithoutToolkitAndRejectsUnmergeablePackets) {
  ExifData exif;
  exif["Exif.Image.Artist"] = "artist";
  const auto packet = rawXmpSource("PRIVATE_EMBEDDED_RAW_819c");
  auto value = Value::create(unsignedByte);
  value->read(reinterpret_cast<const byte*>(packet.data()), packet.size(), invalidByteOrder);
  exif.add(ExifKey("Exif.Image.XMLPacket"), value.get());
  auto image = openHeif(heifWithMetadata(exif));
  EXPECT_EQ(image->xmpPacket(), packet);

  // Preserve embedded raw XMP through Exif edits without parsing its XML.
  image->exifData()["Exif.Image.Artist"] = "edited";
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_EQ(image->xmpPacket(), packet);

  // When Exif is removed, promote its embedded packet to a separate XMP item.
  image->clearExifData();
  image->writeMetadata();
  auto bytes = imageBytes(*image);
  auto document = parse(bytes);
  EXPECT_TRUE(document.metadataItems(bmffType("Exif")).empty());
  ASSERT_EQ(document.metadataItems(bmffType("mime")).size(), 1u);
  EXPECT_EQ(image->xmpPacket(), packet);

  image->clearXmpData();
  image->writeMetadata();
  EXPECT_FALSE(contains(imageBytes(*image), "PRIVATE_EMBEDDED_RAW_819c"));

  // Distinct raw packets cannot be merged safely without the XMP toolkit.
  const auto other = rawXmpSource("PRIVATE_DIFFERENT_RAW_0551");
  bytes = heifWithMetadata(exif, other);
  image = openHeif(bytes);

  image->writeMetadata();
  EXPECT_EQ(imageBytes(*image), bytes);

  // Reject an ambiguous edit but allow explicit removal of all XMP copies.
  image->exifData()["Exif.Image.Artist"] = "pending";
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(imageBytes(*image), bytes);

  image->clearXmpData();
  ASSERT_NO_THROW(image->writeMetadata());
  EXPECT_FALSE(contains(imageBytes(*image), "PRIVATE_EMBEDDED_RAW_819c"));
  EXPECT_FALSE(contains(imageBytes(*image), "PRIVATE_DIFFERENT_RAW_0551"));

  // Structured XMP requests remain unsupported when the toolkit is disabled.
  image->xmpData()["Xmp.dc.source"] = "requires toolkit";
  bytes = imageBytes(*image);
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(imageBytes(*image), bytes);
}
#endif

#endif  // EXV_ENABLE_BMFF
