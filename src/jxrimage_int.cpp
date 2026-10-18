// SPDX-License-Identifier: GPL-2.0-or-later

#include "jxrimage_int.hpp"

#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "photoshop.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "tiffvisitor_int.hpp"
#include "value.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <ostream>
#include <tuple>

namespace Exiv2::Internal {
namespace {

constexpr uint16_t imageOffsetTag = 0xbcc0;
constexpr uint16_t imageCountTag = 0xbcc1;
constexpr uint16_t alphaOffsetTag = 0xbcc2;
constexpr uint16_t alphaCountTag = 0xbcc3;
constexpr uint16_t xmpTag = 0x02bc;
constexpr uint16_t iptcTag = 0x83bb;
constexpr uint16_t resourcesTag = 0x8649;
constexpr uint16_t iccTag = 0x8773;

// Native fields describe decoding, layout and image associations, not Exif metadata.
bool protectedTag(uint16_t tag) {
  return (tag >= 0xbc00 && tag <= 0xbcff) || tag == 0x0129;
}

// TIFF pointers to other payload layouts cannot be relocated as JPEG XR streams.
bool unsupportedPointer(uint16_t tag) {
  switch (tag) {
    case 0x0111:  // StripOffsets
    case 0x0117:  // StripByteCounts
    case 0x0120:  // FreeOffsets
    case 0x0121:  // FreeByteCounts
    case 0x0144:  // TileOffsets
    case 0x0145:  // TileByteCounts
    case 0x014a:  // SubIFDs
    case 0x0201:  // JPEGInterchangeFormat
    case 0x0202:  // JPEGInterchangeFormatLength
    case 0x0207:  // JPEGQTables
    case 0x0208:  // JPEGDCTables
    case 0x0209:  // JPEGACTables
    case 0x927c:  // Opaque makernotes may contain file-relative pointers.
      return true;
    default:
      return false;
  }
}

// Select native fields for the shared TIFF copier without affecting TIFF/RAW defaults.
class JxrHeader final : public TiffHeaderBase {
 public:
  // Emit the version-1 little-endian container header.
  JxrHeader() : TiffHeaderBase(444, 8, littleEndian, 8) {
  }

  // Preserve native decoding and image-association fields through category clears.
  bool isImageTag(uint16_t tag, IfdId group, const PrimaryGroups&) const override {
    return group == IfdId::ifd0Id && protectedTag(tag);
  }
};

// One image owns its directory tree and borrows its compressed ranges from the file.
struct JxrDirectory {
  TiffComponent::UniquePtr tree;
  ExifData exif;
  IptcData iptc;
  XmpData xmp;
  std::vector<TiffReadContext::Range> streams;
};

// Parsed images retain their original order, including previews before primary images.
struct JxrFile {
  std::vector<JxrDirectory> images;
  size_t primary{};
};

// Keep TIFF's category precedence, but leave raw XML opaque when no toolkit exists.
DecoderFct findDecoder(std::string_view make, uint32_t tag, IfdId group) {
#ifndef EXV_HAVE_XMP_TOOLKIT
  if (tag == xmpTag && group == IfdId::ifd0Id)
    return &TiffDecoder::decodeStdTiffEntry;
#endif
  return TiffMapping::findDecoder(make, tag, group);
}

// Find an entry in one image, without mixing its metadata with another image.
TiffEntryBase* entry(TiffComponent& tree, uint16_t tag, IfdId group = IfdId::ifd0Id) {
  TiffFinder finder(tag, group);
  tree.accept(finder);
  return dynamic_cast<TiffEntryBase*>(finder.result());
}

// Read a native scalar with one of the container's permitted unsigned encodings.
uint32_t scalar(TiffComponent& tree, uint16_t tag, bool required = true) {
  auto field = entry(tree, tag);
  if (!field && !required)
    return 0;
  enforce(field && field->pValue() && field->count() == 1, ErrorCode::kerCorruptedMetadata);
  const auto type = field->pValue()->typeId();
  enforce(type == unsignedByte || type == unsignedShort || type == unsignedLong, ErrorCode::kerCorruptedMetadata);
  return field->pValue()->toUint32();
}

// Detach an entry from the read-only source before TIFF's mutable value updater.
void replaceValue(TiffEntryBase& field, Value::UniquePtr value) {
  field.setData(std::make_shared<DataBuf>(value->size()));
  field.updateValue(std::move(value), littleEndian);
}

// Resolve an EOF-sized image and associate the shared TIFF image entry with its bytes.
TiffReadContext::Range stream(TiffComponent& tree, const byte* data, size_t size, uint16_t offsetTag) {
  auto offset = entry(tree, offsetTag);
  auto count = entry(tree, offsetTag + 1);
  enforce(offset && count, ErrorCode::kerCorruptedMetadata);
  const size_t start = scalar(tree, offsetTag);
  size_t length = scalar(tree, offsetTag + 1);
  enforce(start >= 8 && start < size, ErrorCode::kerCorruptedMetadata);
  const bool throughEof = length == 0 && offsetTag == imageOffsetTag;
  if (throughEof)
    length = size - start;
  enforce(length != 0 && length <= size - start, ErrorCode::kerCorruptedMetadata);

  // Annex A permits BYTE/USHORT/ULONG offsets and lengths. Promote these
  // layout fields to ULONG so relocated output offsets never truncate.
  for (const auto& [field, value] : {std::pair{offset, start}, std::pair{count, length}}) {
    auto encoded = Value::create(unsignedLong);
    encoded->read(std::to_string(value));
    replaceValue(*field, std::move(encoded));
  }
  if (throughEof) {
    dynamic_cast<TiffImageEntry&>(*offset).setStrips(count->pValue(), data, size, 0);
  }
  return {start, length};
}

// Parse directories through TIFF's field decoder, then validate complete range ownership.
JxrFile parseFile(const byte* data, size_t size, const DecodeParams& dp) {
  enforce(data && size >= 8 && size <= std::numeric_limits<uint32_t>::max() && std::memcmp(data, "II\xbc\x01", 4) == 0,
          ErrorCode::kerNotAnImage, "JPEG XR");
  TiffReadContext context;
  JxrFile result;
  uint32_t offset = getULong(data + 4, littleEndian);
  bool foundPrimary = false;
  std::vector<TiffReadContext::Range> streams;
  while (offset) {
    enforce(result.images.size() < 1024, ErrorCode::kerCorruptedMetadata);
    JxrDirectory image;
    image.tree = TiffParserWorker::parseDirectory(data, size, offset, context);
    offset = context.nextOffset;

    // Required native geometry and pixel format remain independent of Exif dimensions.
    auto format = entry(*image.tree, 0xbc01);
    enforce(format && format->tiffType() == ttUnsignedByte && format->count() == 16 &&
                scalar(*image.tree, 0xbc80) != 0 && scalar(*image.tree, 0xbc81) != 0,
            ErrorCode::kerCorruptedMetadata);
    const auto type = scalar(*image.tree, 0xbc04, false);
    enforce(type <= 3, ErrorCode::kerCorruptedMetadata);
    if (!foundPrimary && (type & 1) == 0) {
      result.primary = result.images.size();
      foundPrimary = true;
    }

    image.streams.push_back(stream(*image.tree, data, size, imageOffsetTag));
    enforce(bool(entry(*image.tree, alphaOffsetTag)) == bool(entry(*image.tree, alphaCountTag)),
            ErrorCode::kerCorruptedMetadata);
    if (entry(*image.tree, alphaOffsetTag))
      image.streams.push_back(stream(*image.tree, data, size, alphaOffsetTag));
    streams.insert(streams.end(), image.streams.begin(), image.streams.end());

    TiffDecoder decoder(image.exif, image.iptc, image.xmp, image.tree.get(), findDecoder, dp);
    image.tree->accept(decoder);
    result.images.push_back(std::move(image));
  }
  enforce(!result.images.empty() && foundPrimary, ErrorCode::kerCorruptedMetadata);

  // No compressed stream may own metadata bytes or another compressed stream.
  context.ranges.insert(context.ranges.end(), streams.begin(), streams.end());
  context.ranges.push_back({0, 8});
  std::sort(context.ranges.begin(), context.ranges.end(),
            [](const auto& a, const auto& b) { return std::tie(a.offset, a.size) < std::tie(b.offset, b.size); });
  size_t end = 0;
  for (const auto& range : context.ranges) {
    enforce(range.offset >= end, ErrorCode::kerCorruptedMetadata);
    end = range.offset + range.size;
  }
  return result;
}

// Compare typed bytes, preserving duplicates while excluding regenerated carrier entries.
using MetadataValues = std::vector<std::tuple<std::string, TypeId, std::vector<byte>>>;

// Canonicalize Exif for semantic verification; location fields are intentionally relocated.
MetadataValues exifValues(const ExifData& exif) {
  MetadataValues values;
  for (const auto& datum : exif) {
    const auto tag = datum.tag();
    if (datum.ifdId() == IfdId::ifd0Id &&
        (tag == imageOffsetTag || tag == alphaOffsetTag || tag == imageCountTag || tag == alphaCountTag ||
         tag == 0x8769 || tag == 0x8825 || tag == xmpTag || tag == iptcTag || tag == resourcesTag || tag == iccTag))
      continue;
    if (datum.ifdId() == IfdId::exifId && tag == 0xa005)
      continue;
    std::vector<byte> bytes(datum.size());
    datum.copy(bytes.data(), littleEndian);
    values.emplace_back(datum.key(), datum.typeId(), std::move(bytes));
  }
  std::sort(values.begin(), values.end());
  return values;
}

// A metadata-only replacement may omit native fields. A supplied native view
// must remain complete and byte-identical, including duplicate/type information.
MetadataValues nativeValues(const ExifData& exif) {
  MetadataValues values;
  for (const auto& datum : exif) {
    if (datum.ifdId() != IfdId::ifd0Id || !protectedTag(datum.tag()))
      continue;
    std::vector<byte> bytes(datum.size());
    datum.copy(bytes.data(), littleEndian);
    values.emplace_back(datum.key(), datum.typeId(), std::move(bytes));
  }
  std::sort(values.begin(), values.end());
  return values;
}

// Read raw carrier bytes without passing them through an XMP serializer or ICC validator.
std::string rawValue(TiffComponent& tree, uint16_t tag) {
  const auto field = entry(tree, tag);
  if (!field)
    return {};
  return {reinterpret_cast<const char*>(field->pData()), field->size()};
}

// Drop a carrier from the primary view before inserting its freshly serialized value.
void eraseTag(ExifData& exif, uint16_t tag) {
  exif.erase(std::remove_if(exif.begin(), exif.end(),
                            [tag](const auto& datum) { return datum.ifdId() == IfdId::ifd0Id && datum.tag() == tag; }),
             exif.end());
}

// Install exact carrier bytes in the metadata tree; compressed streams never enter this path.
void setBytes(ExifData& exif, uint16_t tag, const byte* bytes, size_t size) {
  eraseTag(exif, tag);
  if (size) {
    DataValue value(bytes, size, invalidByteOrder, tag == iccTag ? undefined : unsignedByte);
    exif.add(ExifKey(tag, "Image"), &value);
  }
}

// Compare decoded category values without depending on padding or XML formatting.
template <typename Container>
auto categoryValues(const Container& data) {
  std::vector<std::tuple<std::string, TypeId, size_t, std::string>> values;
  for (const auto& datum : data)
    values.emplace_back(datum.key(), datum.typeId(), datum.count(), datum.toString());
  std::sort(values.begin(), values.end());
  return values;
}

/*! @brief Private JPEG XR metadata adapter; owns buffered primary metadata and clear intent.
 *  Parsed trees borrow source storage only during individual read/write operations.
 */
class JxrImage final : public Image {
 public:
  /*! @brief Take ownership of I/O for an existing version-1 JPEG XR file. */
  JxrImage(BasicIo::UniquePtr io, const ImageCtorParams& params) :
      Image(ImageType::jxr, mdExif | mdIptc | mdXmp | mdIccProfile, std::move(io), params) {
    if (params.create())
      throw Error(ErrorCode::kerWritingImageFormatUnsupported, "JPEG XR creation");
  }

  /*! @brief Load the first non-preview image and reset pending explicit removal intent. */
  void readMetadata() override;
  /*! @brief Prepare and verify a complete replacement before transferring source bytes. */
  void writeMetadata() override;
  /*! @brief Reuse TIFF's structure printer after validating the JPEG XR container. */
  void printStructure(std::ostream& out, PrintStructureOption option, size_t depth) override;
  /*! @brief Return the JPEG XR media type. */
  std::string mimeType() const override {
    return "image/jxr";
  }
  /*! @brief Replace the primary Exif view; an empty setter clears Exif throughout. */
  void setExifData(const ExifData& data) override;
  /*! @brief Request Exif removal from all images, retaining decoding fields. */
  void clearExifData() override;
  /*! @brief Replace primary IPTC; an empty setter clears IPTC throughout. */
  void setIptcData(const IptcData& data) override;
  /*! @brief Request IPTC removal from all images. */
  void clearIptcData() override;
  /*! @brief Replace primary raw XMP; an empty packet clears XMP throughout. */
  void setXmpPacket(const std::string& packet) override;
  /*! @brief Request XMP removal from all images and clear both buffered representations. */
  void clearXmpPacket() override;
  /*! @brief Replace primary structured XMP, rejecting unavailable serialization. */
  void setXmpData(const XmpData& data) override;
  /*! @brief Request XMP removal from all images and clear both buffered representations. */
  void clearXmpData() override;
  /*! @brief Replace primary ICC; an empty setter clears ICC throughout. */
  void setIccProfile(DataBuf&& profile, bool validate) override;
  /*! @brief Request ICC removal from all images. */
  void clearIccProfile() override;
  /*! @brief Copy all supported categories, snapshotting first to permit self-copy. */
  void setMetadata(const Image& source) override;
  /*! @brief JPEG XR has no separate image-comment category. */
  void setComment(const std::string&) override {
    throw Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "JPEG XR");
  }

 private:
  // Each flag applies to secondary images even after new primary metadata is assigned.
  bool clearExif_{};
  bool clearIptc_{};
  bool clearXmp_{};
  bool clearIcc_{};

  // Adopt verified primary metadata without invoking user-facing clearing APIs.
  void adopt(JxrFile& file, bool usePacket = false);
};

void JxrImage::adopt(JxrFile& file, bool usePacket) {
  auto& image = file.images.at(file.primary);
  exifData_ = image.exif;
  iptcData_ = image.iptc;
  xmpData_ = image.xmp;
  xmpPacket_ = rawValue(*image.tree, xmpTag);
  const auto icc = rawValue(*image.tree, iccTag);
  iccProfile_ = DataBuf(reinterpret_cast<const byte*>(icc.data()), icc.size());
  pixelWidth_ = scalar(*image.tree, 0xbc80);
  pixelHeight_ = scalar(*image.tree, 0xbc81);
  setByteOrder(littleEndian);
  comment_.clear();
  nativePreviews_.clear();
  writeXmpFromPacket(usePacket);
  clearExif_ = clearIptc_ = clearXmp_ = clearIcc_ = false;
}

void JxrImage::readMetadata() {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  IoCloser closer(*io_);
  auto file = parseFile(io_->mmap(), io_->size(), DecodeParams(max_recursion_depth_));
  adopt(file);
}

void JxrImage::printStructure(std::ostream& out, PrintStructureOption option, size_t depth) {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  IoCloser closer(*io_);
  auto file = parseFile(io_->mmap(), io_->size(), DecodeParams(max_recursion_depth_));
  // Binary extraction follows the same primary-image view as metadata APIs.
  if (option == kpsXMP || option == kpsIccProfile) {
    const auto bytes = rawValue(*file.images.at(file.primary).tree, option == kpsXMP ? xmpTag : iccTag);
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
    return;
  }
  io_->seek(0, BasicIo::beg);
  printTiffStructure(*io_, out, option, depth);
}

void JxrImage::setExifData(const ExifData& data) {
  if (data.empty())
    clearExifData();
  else
    Image::setExifData(data);
}

void JxrImage::clearExifData() {
  Image::clearExifData();
  clearExif_ = true;
}

void JxrImage::setIptcData(const IptcData& data) {
  if (data.empty())
    clearIptcData();
  else
    Image::setIptcData(data);
}

void JxrImage::clearIptcData() {
  Image::clearIptcData();
  clearIptc_ = true;
}

void JxrImage::setXmpPacket(const std::string& packet) {
  if (packet.empty()) {
    clearXmpPacket();
    return;
  }
  // Raw packets remain writable without the optional toolkit.
  XmpData decoded;
#ifdef EXV_HAVE_XMP_TOOLKIT
  if (XmpParser::decode(decoded, packet, DecodeParams(max_recursion_depth_)))
    throw Error(ErrorCode::kerInvalidXMP);
#endif
  xmpPacket_ = packet;
  xmpData_ = std::move(decoded);
  xmpData_.setPacket(packet);
  writeXmpFromPacket(true);
}

void JxrImage::clearXmpPacket() {
  Image::clearXmpPacket();
  xmpData_.clear();
  clearXmp_ = true;
}

void JxrImage::setXmpData(const XmpData& data) {
  if (data.empty()) {
    clearXmpData();
    return;
  }
#ifndef EXV_HAVE_XMP_TOOLKIT
  throw Error(ErrorCode::kerInvalidSettingForImage, "Structured XMP without toolkit", "JPEG XR");
#else
  Image::setXmpData(data);
#endif
}

void JxrImage::clearXmpData() {
  clearXmpPacket();
}

void JxrImage::setIccProfile(DataBuf&& profile, bool validate) {
  if (profile.empty())
    clearIccProfile();
  else
    Image::setIccProfile(std::move(profile), validate);
}

void JxrImage::clearIccProfile() {
  Image::clearIccProfile();
  clearIcc_ = true;
}

void JxrImage::setMetadata(const Image& source) {
  auto exif = source.exifData();
  const auto iptc = source.iptcData();
  const auto xmp = source.xmpData();
  const auto packet = source.xmpPacket();
  auto icc = source.iccProfile().clone();
#ifdef EXV_HAVE_XMP_TOOLKIT
  const bool fromPacket = source.writeXmpFromPacket();
#endif
#ifndef EXV_HAVE_XMP_TOOLKIT
  if (!xmp.empty())
    throw Error(ErrorCode::kerInvalidSettingForImage, "Structured XMP without toolkit", "JPEG XR");
#endif
  // Copy writable metadata, never the source image's decoding/layout fields.
  exif.erase(
      std::remove_if(exif.begin(), exif.end(),
                     [](const auto& datum) { return datum.ifdId() == IfdId::ifd0Id && protectedTag(datum.tag()); }),
      exif.end());
  setExifData(exif);
  setIptcData(iptc);
  setIccProfile(std::move(icc), false);
#ifdef EXV_HAVE_XMP_TOOLKIT
  if (fromPacket)
    setXmpPacket(packet);
  else
    setXmpData(xmp);
#else
  setXmpPacket(packet);
#endif
}

// Decode one independently addressable tree using TIFF's existing category precedence.
JxrDirectory decodedTree(TiffComponent& tree, const DecodeParams& dp) {
  JxrDirectory result;
  TiffDecoder decoder(result.exif, result.iptc, result.xmp, &tree, findDecoder, dp);
  tree.accept(decoder);
  return result;
}

// Remove only requested categories from a secondary image's parsed entries.
void clearCategories(TiffComponent& tree, bool exif, bool iptc, bool xmp, bool icc) {
  auto& entries = dynamic_cast<TiffDirectory&>(tree).components();
  std::erase_if(entries, [&](const auto& component) {
    const auto tag = component->tag();
    if (protectedTag(tag))
      return false;
    if (tag == xmpTag)
      return xmp;
    if (tag == iccTag)
      return icc;
    if (tag == iptcTag)
      return iptc;
    if (tag == resourcesTag) {
      // An Exif clear drops opaque Photoshop resources, retaining decoded IPTC
      // in a fresh standard carrier when that category was not also cleared.
      if (exif)
        return true;
      if (iptc) {
        auto& field = dynamic_cast<TiffEntryBase&>(*component);
        auto remaining = Photoshop::setIptcIrb(field.pData(), field.size(), IptcData{});
        if (remaining.empty())
          return true;
        replaceValue(field,
                     std::make_unique<DataValue>(remaining.c_data(), remaining.size(), invalidByteOrder, unsignedByte));
      }
      return false;
    }
    return exif;
  });
}

void JxrImage::writeMetadata() {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  IoCloser closer(*io_);
  const byte* source = io_->mmap();
  const DecodeParams dp(max_recursion_depth_);
  auto original = parseFile(source, io_->size(), dp);
  auto& primary = original.images.at(original.primary);

  // All layouts and requested native-field changes are rejected before source
  // transfer. Missing native fields are retained, including after Exif removal.
  for (const auto& image : original.images) {
    for (const auto& datum : image.exif)
      enforce(!unsupportedPointer(datum.tag()), ErrorCode::kerWritingImageFormatUnsupported, "JPEG XR pointer layout");
  }
  const auto requestedNative = nativeValues(exifData_);
  enforce(requestedNative.empty() || requestedNative == nativeValues(primary.exif),
          ErrorCode::kerInvalidSettingForImage, "Protected decoding/layout fields", "JPEG XR");
  size_t metadataBytes = 0;
  for (const auto& datum : exifData_) {
    enforce(datum.size() <= 64 * 1024 * 1024 - metadataBytes, ErrorCode::kerImageWriteFailed);
    metadataBytes += datum.size();
    enforce(!unsupportedPointer(datum.tag()), ErrorCode::kerWritingImageFormatUnsupported, "JPEG XR pointer layout");
    enforce(datum.ifdId() == IfdId::ifd0Id || datum.ifdId() == IfdId::exifId || datum.ifdId() == IfdId::gpsId ||
                datum.ifdId() == IfdId::iopId,
            ErrorCode::kerWritingImageFormatUnsupported, "JPEG XR metadata group");
  }

  // Serialize categories once, preserving exact raw XMP with or without a toolkit.
  ExifData desired = exifData_;
  std::string packet = xmpPacket_;
#ifdef EXV_HAVE_XMP_TOOLKIT
  if (!writeXmpFromPacket()) {
    packet.clear();
    enforce(XmpParser::encode(packet, xmpData_, XmpParser::useCompactFormat | XmpParser::omitAllFormatting) <= 1,
            ErrorCode::kerInvalidXMP);
  }
#else
  enforce(xmpData_.empty(), ErrorCode::kerInvalidSettingForImage, "Structured XMP without toolkit", "JPEG XR");
#endif
  enforce(packet.size() <= 64 * 1024 * 1024 && iccProfile_.size() <= 64 * 1024 * 1024, ErrorCode::kerImageWriteFailed);
  setBytes(desired, xmpTag, reinterpret_cast<const byte*>(packet.data()), packet.size());
  setBytes(desired, iccTag, iccProfile_.c_data(), iccProfile_.size());
  XmpData serializedXmp = xmpData_;
  serializedXmp.setPacket(packet);
  serializedXmp.usePacket(true);

  // The shared copier retains native fields and stream references; the encoder
  // builds new metadata without attempting an in-place update of the source.
  JxrHeader header;
  auto replacement = TiffParserWorker::buildTree(std::move(primary.tree), desired, iptcData_, serializedXmp, Tag::root,
                                                 &header, TiffMapping::findEncoder);
  primary.tree = std::move(replacement);
  for (size_t i = 0; i < original.images.size(); ++i) {
    if (i != original.primary) {
      clearCategories(*original.images[i].tree, clearExif_, clearIptc_, clearXmp_, clearIcc_);
      if (clearExif_ && !clearIptc_ && !original.images[i].iptc.empty()) {
        // IPTC may have existed only inside the discarded Photoshop resources.
        auto& directory = dynamic_cast<TiffDirectory&>(*original.images[i].tree);
        auto& entries = directory.components();
        entries.erase(std::remove_if(entries.begin(), entries.end(),
                                     [](const auto& component) { return component->tag() == iptcTag; }),
                      entries.end());
        auto raw = IptcParser::encode(original.images[i].iptc);
        auto field = std::make_unique<TiffEntry>(iptcTag, IfdId::ifd0Id);
        field->updateValue(std::make_unique<DataValue>(raw.c_data(), raw.size(), invalidByteOrder, unsignedByte),
                           littleEndian);
        directory.addChild(std::move(field));
      }
    }
  }

  // Capture intended per-image metadata before linking local IFD0 trees. This
  // also checks that no requested ordinary Exif entry disappeared in construction.
  std::vector<JxrDirectory> expected;
  std::vector<std::string> packets;
  std::vector<std::string> profiles;
  for (auto& image : original.images) {
    expected.push_back(decodedTree(*image.tree, dp));
    packets.push_back(rawValue(*image.tree, xmpTag));
    profiles.push_back(rawValue(*image.tree, iccTag));
  }
  const auto& profile = profiles.at(original.primary);
  enforce(packets.at(original.primary) == packet && profile.size() == iccProfile_.size() &&
              (profile.empty() || std::memcmp(profile.data(), iccProfile_.c_data(), profile.size()) == 0),
          ErrorCode::kerImageWriteFailed);
  auto requested = desired;
  requested.erase(
      std::remove_if(requested.begin(), requested.end(),
                     [](const auto& datum) { return datum.ifdId() == IfdId::ifd0Id && protectedTag(datum.tag()); }),
      requested.end());
  auto actual = expected.at(original.primary).exif;
  actual.erase(
      std::remove_if(actual.begin(), actual.end(),
                     [](const auto& datum) { return datum.ifdId() == IfdId::ifd0Id && protectedTag(datum.tag()); }),
      actual.end());
  enforce(exifValues(requested) == exifValues(actual), ErrorCode::kerImageWriteFailed);
  enforce(categoryValues(iptcData_) == categoryValues(expected.at(original.primary).iptc),
          ErrorCode::kerImageWriteFailed);

  // Keep directory/preview ordering unchanged. TIFF's layout pass relocates all
  // nested metadata and image/alpha offsets into one file-relative output space.
  auto tree = std::move(original.images.front().tree);
  TiffComponent* tail = tree.get();
  for (size_t i = 1; i < original.images.size(); ++i)
    tail = tail->addNext(std::move(original.images[i].tree));
  uint64_t measured = uint64_t(header.size()) + tree->size();
  for (const auto& image : original.images) {
    for (const auto& range : image.streams)
      measured += range.size + (range.size & 1);
  }
  enforce(measured <= std::numeric_limits<uint32_t>::max(), ErrorCode::kerImageWriteFailed);

  // Temporary I/O owns output bytes, while image entries borrow the original
  // streams. No compressed stream is materialized as an Exif Value/DataBuf.
  MemIo temporary;
  const auto bytes = header.write();
  IoWrapper output(temporary, bytes.c_data(), bytes.size(), nullptr);
  size_t imageIndex = std::string::npos;
  tree->write(output, littleEndian, bytes.size(), std::string::npos, std::string::npos, imageIndex);
  enforce(!temporary.error() && temporary.size() == measured, ErrorCode::kerImageWriteFailed);

  // Reparse independently from serialized offsets and verify every image's
  // metadata and every compressed stream before committing any source bytes.
  const byte* prepared = temporary.mmap();
  auto verified = parseFile(prepared, temporary.size(), dp);
  enforce(verified.images.size() == original.images.size() && verified.primary == original.primary,
          ErrorCode::kerImageWriteFailed);
  for (size_t i = 0; i < verified.images.size(); ++i) {
    auto& result = verified.images[i];
    enforce(exifValues(result.exif) == exifValues(expected[i].exif) &&
                categoryValues(result.iptc) == categoryValues(expected[i].iptc) &&
                categoryValues(result.xmp) == categoryValues(expected[i].xmp) &&
                rawValue(*result.tree, xmpTag) == packets[i] && rawValue(*result.tree, iccTag) == profiles[i] &&
                result.streams.size() == original.images[i].streams.size(),
            ErrorCode::kerImageWriteFailed);
    for (size_t j = 0; j < result.streams.size(); ++j) {
      const auto before = original.images[i].streams[j];
      const auto after = result.streams[j];
      enforce(before.size == after.size, ErrorCode::kerImageWriteFailed);
      for (size_t position = 0; position < before.size;) {
        const auto length = std::min<size_t>(65536, before.size - position);
        enforce(std::memcmp(source + before.offset + position, prepared + after.offset + position, length) == 0,
                ErrorCode::kerImageWriteFailed);
        position += length;
      }
    }
  }

  // Preserve pending edits/clear intent if transfer fails. The final BasicIo
  // transfer has the same guarantees as other format adapters.
  io_->transfer(temporary);
  // Transfer may invalidate the temporary mapping; reopen before adopting state.
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  auto written = parseFile(io_->mmap(), io_->size(), dp);
  adopt(written, writeXmpFromPacket());
}

}  // namespace

Image::UniquePtr newJxrInstance(BasicIo::UniquePtr io, const ImageCtorParams& params) {
  return std::make_unique<JxrImage>(std::move(io), params);
}

bool isJxrType(BasicIo& io, bool advance) {
  const auto position = io.tell();
  std::array<byte, 4> signature{};
  const bool match = io.read(signature.data(), signature.size()) == signature.size() &&
                     signature == std::array<byte, 4>{'I', 'I', 0xbc, 1};
  if (!advance || !match)
    io.seek(position, BasicIo::beg);
  return match;
}

}  // namespace Exiv2::Internal
