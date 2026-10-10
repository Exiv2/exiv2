// SPDX-License-Identifier: GPL-2.0-or-later

#include "heifimage_int.hpp"

#ifdef EXV_ENABLE_BMFF
#include "basicio.hpp"
#include "bmffimage.hpp"
#include "bmffwrite_int.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "tags.hpp"
#include "tiffimage.hpp"
#include "value.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <iterator>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <string_view>

#ifdef EXV_ENABLE_FILESYSTEM
#include <filesystem>
#include <random>
#endif

namespace Exiv2::Internal {
namespace {
using Bytes = std::vector<byte>;

// Search validated headers only; UUID payloads and other opaque bytes remain unread.
bool hasAdobeXmpUuid(const std::vector<BmffBox>& boxes) {
  static constexpr std::array<uint8_t, 16> xmpUuid{0xbe, 0x7a, 0xcf, 0xcb, 0x97, 0xa9, 0x42, 0xe8,
                                                   0x9c, 0x71, 0x99, 0x94, 0x91, 0xe3, 0xaf, 0xac};
  for (const auto& box : boxes) {
    if ((box.type == bmffType("uuid") && box.userType == xmpUuid) || hasAdobeXmpUuid(box.children))
      return true;
  }
  return false;
}

// Reject unsupported edits before they can replace the source stream.
void supported(bool condition, std::string_view reason) {
  if (!condition)
    throw Error(ErrorCode::kerErrorMessage, std::string("Unsupported HEIF edit: ") + std::string(reason));
}

// Read a bounded property payload from a validated absolute input span.
Bytes readRange(BasicIo& io, BmffSpan span) {
  supported(span.size <= bmffMetadataLimit, "metadata exceeds the allocation limit");
  Bytes bytes(static_cast<size_t>(span.size));
  io.seekOrThrow(static_cast<int64_t>(span.offset), BasicIo::beg, ErrorCode::kerInputDataReadFailed);
  io.readOrThrow(bytes.data(), bytes.size(), ErrorCode::kerInputDataReadFailed);
  return bytes;
}

// Serialize structured XMP compactly; empty data denotes an absent packet.
std::string encodeXmp(const XmpData& data) {
  if (data.empty())
    return {};

  std::string packet;
  enforce(XmpParser::encode(packet, data, XmpParser::useCompactFormat | XmpParser::omitAllFormatting) == 0,
          ErrorCode::kerInvalidXMP);
  supported(packet.size() <= bmffMetadataLimit, "XMP exceeds the allocation limit");
  return packet;
}

// Keep last-item precedence without rescanning all previously decoded tags for
// each item. The list preserves file order; the index removes superseded groups.
template <typename Data>
class MetadataMerger {
 public:
  //! @brief Merge the next item, retaining duplicate Exif values within its winning key group.
  void add(const Data& source, bool preserveDuplicates = false) {
    std::set<std::string> seen;
    for (const auto& datum : source) {
      // Remove a superseded group once, but preserve duplicates within the incoming Exif item.
      const auto key = datum.key();
      if (!preserveDuplicates || seen.insert(key).second) {
        if (auto found = index_.find(key); found != index_.end()) {
          for (auto entry : found->second)
            values_.erase(entry);
          index_.erase(found);
        }
      }

      values_.push_back(datum);
      index_[key].push_back(std::prev(values_.end()));
    }
  }

  //! @brief Materialize the accumulated metadata in its surviving file order.
  Data data() const {
    Data result;
    for (const auto& datum : values_)
      result.add(datum);
    return result;
  }

 private:
  using Datum = typename std::iterator_traits<typename Data::iterator>::value_type;
  std::list<Datum> values_;
  std::map<std::string, std::vector<typename std::list<Datum>::iterator>> index_;
};

// Compare raw values, preserving duplicate tags and attached thumbnail data.
template <typename Data>
bool sameData(const Data& a, const Data& b) {
  if (a.count() != b.count())
    return false;

  // Compare canonical raw values and attached data areas, independent of container offsets.
  auto snapshot = [](const Data& data) {
    std::multimap<std::string, std::pair<TypeId, Bytes>> values;
    uint64_t total = 0;
    for (const auto& datum : data) {
      supported(datum.size() <= bmffMetadataLimit - total, "metadata exceeds the allocation limit");
      total += datum.size();
      Bytes bytes(datum.size());
      if (!bytes.empty())
        datum.copy(bytes.data(), littleEndian);

      supported(datum.value().sizeDataArea() <= bmffMetadataLimit - total, "metadata exceeds the allocation limit");

      auto area = datum.value().dataArea();
      total += area.size();
      if (!area.empty())
        bytes.insert(bytes.end(), area.c_data(), area.c_data() + area.size());
      values.emplace(datum.key(), std::pair(datum.typeId(), std::move(bytes)));
    }
    return values;
  };

  return snapshot(a) == snapshot(b);
}

// Decoded primary metadata plus the raw packet and image properties needed for preservation.
struct Metadata {
  ExifData exif;
  IptcData iptc;
  XmpData xmp;
  XmpData embeddedXmp;
  std::string packet;
  bool hasEmbeddedXmp{}, needsXmpToolkit{};
  ByteOrder order{littleEndian};
  DataBuf icc;
  uint32_t width{}, height{};
};

// Decode primary-image items in file order and retain the state needed for later writes.
Metadata readMetadata(BasicIo& io, const BmffDocument& document, const DecodeParams& params) {
  Metadata metadata;
  MetadataMerger<ExifData> exifMerger;
  MetadataMerger<XmpData> embeddedMerger;
  uint64_t total = 0;

  // Charge all Exif and XMP payloads to one aggregate allocation budget.
  auto read = [&](uint32_t id) {
    auto bytes = readBmffItem(io, document.items.at(id), bmffMetadataLimit - total);
    total += bytes.size();
    return bytes;
  };

#ifndef EXV_HAVE_XMP_TOOLKIT
  // Without a toolkit, preserve packets exactly. Different packets cannot be
  // merged when several primary metadata items must become one new Exif item.
  auto rememberPacket = [&](const std::string& packet) {
    if (packet.empty())
      return;
    metadata.needsXmpToolkit |= !metadata.packet.empty() && metadata.packet != packet;
    metadata.packet = packet;
  };
#endif
  for (auto id : document.metadataItems(bmffType("Exif"))) {
    // Exif offsets are relative to the byte after the four-byte HEIF prefix.
    auto bytes = read(id);
    enforce(bytes.size() >= 12, ErrorCode::kerCorruptedMetadata);
    const auto offset = getULong(bytes.data(), bigEndian);
    enforce(offset <= bytes.size() - 12, ErrorCode::kerCorruptedMetadata);
    ExifData exif;
    IptcData iptc;
    XmpData xmp;
    auto order = TiffParser::decode(exif, iptc, xmp, bytes.data() + 4 + offset, bytes.size() - 4 - offset, params);
    enforce(order != invalidByteOrder, ErrorCode::kerCorruptedMetadata);
    metadata.order = order;

    // Remember embedded packets even when structured XMP decoding is unavailable.
    if (auto xml = exif.findKey(ExifKey("Exif.Image.XMLPacket")); xml != exif.end()) {
      metadata.hasEmbeddedXmp = true;
#ifndef EXV_HAVE_XMP_TOOLKIT
      Bytes packet(xml->size());
      xml->copy(packet.data(), order);
      rememberPacket(std::string(packet.begin(), packet.end()));
#endif
    }

    // Later Exif items replace earlier keys while retaining their own duplicate values.
    exifMerger.add(exif, true);
    for (const auto& datum : iptc)
      metadata.iptc.add(datum);
    embeddedMerger.add(xmp);
  }
  metadata.exif = exifMerger.data();
  metadata.embeddedXmp = embeddedMerger.data();

  // Separate MIME XMP takes precedence over the XMP embedded in Exif.
#ifdef EXV_HAVE_XMP_TOOLKIT
  MetadataMerger<XmpData> xmpMerger;
  xmpMerger.add(metadata.embeddedXmp);
#endif
  auto xmpIds = document.metadataItems(bmffType("mime"));
  for (auto id : xmpIds) {
    supported(document.items.at(id).info.contentEncoding.empty(), "compressed primary XMP");
    auto bytes = read(id);
    std::string packet(bytes.begin(), bytes.end());
#ifdef EXV_HAVE_XMP_TOOLKIT
    XmpData xmp;
    enforce(XmpParser::decode(xmp, packet, params) == 0, ErrorCode::kerInvalidXMP);
    xmpMerger.add(xmp);
    metadata.packet = std::move(packet);
#else
    rememberPacket(packet);
#endif
  }

#ifdef EXV_HAVE_XMP_TOOLKIT
  metadata.xmp = xmpMerger.data();
  if (xmpIds.size() != 1 || !metadata.embeddedXmp.empty())
    metadata.packet = encodeXmp(metadata.xmp);
#endif

  // Read dimensions and ICC only from properties associated with the primary item.
  std::set<uint16_t> properties;
  for (const auto& entry : document.associations) {
    if (entry.itemId == document.primaryItem) {
      for (const auto& association : entry.properties)
        if (association.index != 0)
          properties.insert(association.index);
    }
  }
  for (auto index : properties) {
    const auto& property = document.properties.at(index - 1);
    if (property.type != bmffType("ispe") && property.type != bmffType("colr"))
      continue;
    auto bytes = readRange(io, property.payload());
    if (property.type == bmffType("ispe")) {
      enforce(bytes.size() == 12, ErrorCode::kerCorruptedMetadata);
      metadata.width = getULong(bytes.data() + 4, bigEndian);
      metadata.height = getULong(bytes.data() + 8, bigEndian);
    } else if (bytes.size() >= 4 && (getULong(bytes.data(), bigEndian) == bmffType("prof") ||
                                     getULong(bytes.data(), bigEndian) == bmffType("rICC"))) {
      metadata.icc = DataBuf(bytes.data() + 4, bytes.size() - 4);
    }
  }

  return metadata;
}

// Remove every occurrence of a tag, including duplicates from merged metadata.
void eraseTag(ExifData& exif, std::string_view key) {
  exif.erase(std::remove_if(exif.begin(), exif.end(), [&](const auto& d) { return d.key() == key; }), exif.end());
}

#ifdef EXV_HAVE_XMP_TOOLKIT
// Isolate all raw embedded packets so duplicate insertion and deletion count as edits.
ExifData xmlPacketData(const ExifData& exif) {
  ExifData packets;
  for (const auto& datum : exif)
    if (datum.key() == "Exif.Image.XMLPacket")
      packets.add(datum);
  return packets;
}
#endif

// Identify synthesized Canon autofocus fields that cannot be edited independently.
bool canonDerived(const Exifdatum& datum) {
  // These values are synthesized by decodeCanonAFInfo from Canon's 0x0026
  // record. ExifParser filters the same read-only fields for JPEG writing.
  return datum.groupName() == "Canon" && ((datum.tag() >= 0x2600 && datum.tag() <= 0x260e) || datum.tag() == 0x2611);
}

// Isolate synthesized autofocus values for read-only consistency checks.
ExifData derivedCanonData(const ExifData& data) {
  ExifData derived;
  for (const auto& datum : data)
    if (canonDerived(datum))
      derived.add(datum);
  return derived;
}

// Exclude regenerated offsets and raw decoded MakerNotes from semantic comparisons.
ExifData exifValues(ExifData data) {
#ifdef EXV_HAVE_XMP_TOOLKIT
  // Embedded XMP is regenerated and checked separately through its decoded properties.
  eraseTag(data, "Exif.Image.XMLPacket");
#endif
  const bool decodedMakerNote = data.findKey(ExifKey("Exif.MakerNote.ByteOrder")) != data.end();
  data.erase(std::remove_if(data.begin(), data.end(),
                            [&](const auto& datum) {
                              const auto key = datum.key();
                              return key == "Exif.Image.ExifTag" || key == "Exif.Image.GPSTag" ||
                                     key == "Exif.Photo.InteroperabilityTag" || key == "Exif.MakerNote.Offset" ||
                                     key == "Exif.Thumbnail.JPEGInterchangeFormat" ||
                                     key == "Exif.Thumbnail.StripOffsets" ||
                                     (decodedMakerNote && key == "Exif.Photo.MakerNote");
                            }),
             data.end());
  return data;
}

// Encode a fresh HEIF Exif item and reject serializers that lose requested values.
Bytes encodeExif(ExifData exif, const IptcData& iptc, XmpData embedded, ByteOrder order, const DecodeParams& params) {
  // A fresh tree excludes original TIFF gaps, shortened values and tail bytes.
  // The serializer reconstructs known MakerNotes and copies retained data areas.
#ifdef EXV_HAVE_XMP_TOOLKIT
  // Replace every raw packet, including duplicates, with the intended structured XMP.
  eraseTag(exif, "Exif.Image.XMLPacket");
#endif
  const auto requested = exifValues(exif);
  exif.erase(std::remove_if(exif.begin(), exif.end(), canonDerived), exif.end());

  MemIo output;
  uint64_t total = 0;
  for (const auto& datum : exif) {
    supported(datum.size() <= bmffMetadataLimit - total, "Exif exceeds the allocation limit");
    total += datum.size();
    supported(datum.value().sizeDataArea() <= bmffMetadataLimit - total, "Exif exceeds the allocation limit");
    total += datum.value().sizeDataArea();
  }

  // Use no original TIFF backing buffer, so deleted values cannot survive in slack.
  TiffParser::encode(output, nullptr, 0, order, exif, iptc, embedded);
  supported(output.size() <= bmffMetadataLimit - 4, "Exif exceeds the allocation limit");

  // A zero HEIF Exif offset places the TIFF header immediately after the prefix.
  Bytes bytes(4 + output.size(), 0);
  output.seekOrThrow(0, BasicIo::beg, ErrorCode::kerInputDataReadFailed);
  output.readOrThrow(bytes.data() + 4, bytes.size() - 4);

  // Decode the new item to catch unsupported MakerNote or metadata transformations.
  ExifData checked;
  IptcData checkedIptc;
  XmpData checkedXmp;
  enforce(TiffParser::decode(checked, checkedIptc, checkedXmp, bytes.data() + 4, bytes.size() - 4, params) == order,
          ErrorCode::kerCorruptedMetadata);
  supported(sameData(requested, exifValues(checked)), "TIFF serializer cannot preserve requested Exif values");
  supported(sameData(iptc, checkedIptc), "TIFF serializer cannot preserve embedded IPTC");
  supported(encodeXmp(embedded) == encodeXmp(checkedXmp), "TIFF serializer cannot preserve embedded XMP");

  return bytes;
}

// Detect media bytes that no current item owns, including stale metadata from older edits.
bool hasOrphanedMedia(const BmffDocument& document) {
  std::vector<BmffSpan> spans;
  for (const auto& [id, item] : document.items)
    for (const auto& extent : item.location.extents)
      spans.push_back(extent.source);

  std::sort(spans.begin(), spans.end(), [](auto a, auto b) { return a.offset < b.offset; });

  // Measure the union so shared extents do not disguise unreferenced media bytes.
  uint64_t covered = 0, previousEnd = 0;
  for (const auto span : spans) {
    const auto end = span.offset + span.size;
    if (end > previousEnd) {
      covered += end - std::max(previousEnd, span.offset);
      previousEnd = end;
    }
  }

  // A gap in either mdat or idat requires compaction even without a metadata edit.
  uint64_t available = document.itemData ? document.itemData->size : 0;
  for (const auto span : document.mediaData)
    available += span.size;
  return covered != available;
}

// Expose staged file bytes through BasicIo's generic transfer contract. The
// existing FileIo-to-FileIo optimization mistakes filesystem::remove's boolean
// success for an error. Keep that independent FileIo repair outside this writer.
class TransferSource final : public BasicIo {
 public:
  //! @brief Borrow prepared bytes without exposing FileIo-specific transfer behavior.
  explicit TransferSource(BasicIo& io) : io_(io) {
  }

  //! @brief Open or rewind the borrowed source for a complete transfer.
  int open() override {
    return io_.isopen() ? io_.seek(0, BasicIo::beg) : io_.open();
  }

  //! @brief Close the borrowed stream when its transfer consumer finishes.
  int close() override {
    return io_.close();
  }

  //! @brief Reject direct writes through this read-only view.
  size_t write(const byte*, size_t) override {
    throw Error(ErrorCode::kerImageWriteFailed);
  }

  //! @brief Reject copying another stream into the prepared source.
  size_t write(BasicIo&) override {
    throw Error(ErrorCode::kerImageWriteFailed);
  }

  //! @brief Reject single-byte writes through this view.
  int putb(byte) override {
    throw Error(ErrorCode::kerImageWriteFailed);
  }

  //! @brief Read an owned buffer from the borrowed source.
  DataBuf read(size_t count) override {
    return io_.read(count);
  }

  //! @brief Read into the caller buffer using the borrowed source.
  size_t read(byte* data, size_t count) override {
    return io_.read(data, count);
  }

  //! @brief Read one byte from the borrowed source.
  int getb() override {
    return io_.getb();
  }

  //! @brief Reject replacing the prepared source through this view.
  void transfer(BasicIo&) override {
    throw Error(ErrorCode::kerImageWriteFailed);
  }

  //! @brief Seek the borrowed source using BasicIo positioning semantics.
  int seek(int64_t offset, Position position) override {
    return io_.seek(offset, position);
  }

  //! @brief Reject mapping, which would expose mutable prepared bytes.
  byte* mmap(bool = false) override {
    throw Error(ErrorCode::kerImageWriteFailed);
  }

  //! @brief Report success because this view never creates a mapping.
  int munmap() override {
    return 0;
  }

  //! @brief Return the borrowed stream position.
  size_t tell() const override {
    return io_.tell();
  }

  //! @brief Return the prepared byte count.
  size_t size() const override {
    return io_.size();
  }

  //! @brief Report whether the borrowed stream is open.
  bool isopen() const override {
    return io_.isopen();
  }

  //! @brief Forward the borrowed stream error state.
  int error() const override {
    return io_.error();
  }

  //! @brief Forward the borrowed stream end-of-file state.
  bool eof() const override {
    return io_.eof();
  }

  //! @brief Return the borrowed source path without copying it.
  const std::string& path() const noexcept override {
    return io_.path();
  }

  //! @brief Reject creation of synthetic contents in a prepared source.
  void populateFakeData() override {
    throw Error(ErrorCode::kerImageWriteFailed);
  }

 private:
  BasicIo& io_;
};

// Own staging storage and remove any temporary file when preparation or transfer ends.
class PreparedOutput {
 public:
  //! @brief Use a private sibling temporary file for FileIo, otherwise an owned MemIo.
  explicit PreparedOutput(BasicIo& source) {
#ifdef EXV_ENABLE_FILESYSTEM
    if (dynamic_cast<FileIo*>(&source)) {
      // Exclusive create keeps an existing path or symlink from being followed.
      // The open FileIo is retained until validation and final transfer.
      const auto parent = std::filesystem::path(reinterpret_cast<const char8_t*>(source.path().c_str())).parent_path();
      std::random_device random;
      for (unsigned attempt = 0; attempt != 32; ++attempt) {
        auto name = parent / (".exiv2-heif-" + std::to_string(random()) + "-" + std::to_string(random()));
#ifdef _WIN32
        auto file = std::make_unique<FileIo>(name.wstring());
#else
        auto file = std::make_unique<FileIo>(name.string());
#endif
        if (file->open("w+bx") == 0) {
          // Restrict staging permissions before retaining the temporary path.
          std::error_code error;
          std::filesystem::permissions(name, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
                                       std::filesystem::perm_options::replace, error);
          if (error) {
            file.reset();
            std::filesystem::remove(name, error);
            throw Error(ErrorCode::kerImageWriteFailed);
          }

          path_ = name;
          io_ = std::move(file);
          return;
        }

        if (errno != EEXIST)
          throw Error(ErrorCode::kerDataSourceOpenFailed, source.path(), strError());
      }
      throw Error(ErrorCode::kerImageWriteFailed);
    }
#else
    (void)source;
#endif

    // Memory-backed and filesystem-disabled builds use the same preparation contract.
    io_ = std::make_unique<MemIo>();
  }

  //! @brief Close staging storage before removing its temporary path, including on failure.
  ~PreparedOutput() {
    io_.reset();
#ifdef EXV_ENABLE_FILESYSTEM
    if (!path_.empty()) {
      std::error_code error;
      std::filesystem::remove(path_, error);
    }
#endif
  }

  //! @brief Borrow the open staging stream while this owner remains alive.
  BasicIo& io() {
    return *io_;
  }

 private:
  BasicIo::UniquePtr io_;
#ifdef EXV_ENABLE_FILESYSTEM
  std::filesystem::path path_;
#endif
};

// Private subclass supplies edit intent without changing BmffImage's public ABI.
class HeifImage final : public BmffImage {
 public:
  //! @brief Own the source stream and register the HEIF metadata capabilities.
  HeifImage(BasicIo::UniquePtr io, const ImageCtorParams& params) : BmffImage(std::move(io), params) {
    setTypeSupported(ImageType::heif, mdExif | mdIptc | mdXmp);
  }

  //! @brief Report the MIME type corresponding to the most recently read major brand.
  std::string mimeType() const override {
    if (legacyRead_)
      return BmffImage::mimeType();
    return brand_ == bmffType("mif1") ? "image/heif" : "image/heic";
  }

  //! @brief Load the native item model, falling back to the legacy reader for deferred layouts.
  void readMetadata() override {
    enforce(io_->open() == 0, ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
    IoCloser closer(*io_);

    try {
      const auto document = parseBmff(*io_);
      brand_ = document.majorBrand;

      // Opaque Adobe UUID packets need the legacy reader's traversal and precedence.
      if (!hasAdobeXmpUuid(document.boxes)) {
        assign(Internal::readMetadata(*io_, document, DecodeParams(max_recursion_depth_)));
        return;
      }
    } catch (const Error&) {
      // Deferred layouts also retain the existing reader and its failure behavior.
    }

    // Reading replaces all buffered categories, including those retained by a user clear.
    Image::clearMetadata();

    // Finalize either fallback only after reading succeeds. Writing still reparses and rejects.
    BmffImage::readMetadata();
    legacyRead_ = true;
    loaded_ = true;
    removeExif_ = removeXmp_ = false;
  }

  /*!
    @brief Copy writable Exif/XMP metadata, preserving raw packets without the XMP toolkit.
    @param image Source image; self-copy is supported and source buffers are not retained.
    @throws Error If structured XMP needs an unavailable toolkit or a packet exceeds the size limit.

    The copy changes buffered metadata only. An empty source XMP packet requests
    removal when the toolkit is unavailable, just as an explicit clear does.
   */
  void setMetadata(const Image& image) override {
#ifdef EXV_HAVE_XMP_TOOLKIT
    Image::setMetadata(image);
#else
    supported(image.xmpData().empty(), "structured XMP editing requires the XMP toolkit");
    const auto& sourcePacket = image.xmpPacket();
    supported(sourcePacket.size() <= bmffMetadataLimit, "XMP exceeds the allocation limit");

    // Snapshot both views before changing this image, which may also be the source.
    const auto exif = image.exifData();
    const auto packet = sourcePacket;
    setExifData(exif);
    setXmpPacket(packet);
#endif
  }

  //! @brief Replace Exif data, treating an empty replacement as an explicit removal request.
  void setExifData(const ExifData& exif) override {
    Image::setExifData(exif);
    removeExif_ = exif.empty();
  }

  //! @brief Request physical Exif removal even when the decoded view is already empty.
  void clearExifData() override {
    Image::clearExifData();
    removeExif_ = true;
  }

  //! @brief Replace structured XMP, or request removal when the replacement is empty.
  void setXmpData(const XmpData& xmp) override {
#ifndef EXV_HAVE_XMP_TOOLKIT
    supported(xmp.empty(), "structured XMP editing requires the XMP toolkit");
#endif

    if (xmp.empty())
      Image::clearXmpPacket();

    Image::setXmpData(xmp);
    removeXmp_ = xmp.empty();
  }

  //! @brief Clear both XMP views so a cached packet cannot restore removed metadata.
  void clearXmpData() override {
    Image::clearXmpPacket();
    Image::clearXmpData();
    removeXmp_ = true;
  }

  //! @brief Select a bounded raw XMP replacement without requiring structured editing support.
  void setXmpPacket(const std::string& packet) override {
    supported(packet.size() <= bmffMetadataLimit, "XMP exceeds the allocation limit");
#ifdef EXV_HAVE_XMP_TOOLKIT
    Image::setXmpPacket(packet);
#else
    xmpPacket_ = packet;
    xmpData_.clear();
#endif

    writeXmpFromPacket(true);
    removeXmp_ = packet.empty();
  }

  //! @brief Request removal of the primary XMP packet from the rewritten output.
  void clearXmpPacket() override {
    Image::clearXmpPacket();
    removeXmp_ = true;
  }

  //! @brief Clear writable Exif/XMP categories while retaining unsupported standalone IPTC/ICC state.
  void clearMetadata() override {
    clearExifData();
    clearXmpPacket();
    clearXmpData();
    // Only the writable categories are cleared; standalone IPTC/ICC edits are unsupported.
  }

  //! @brief Prepare and verify a compact edit before transfer; final transfer is not guaranteed atomic.
  void writeMetadata() override {
    // Reparse current source bytes before deciding whether any edit can be supported.
    supported(dynamic_cast<RemoteIo*>(io_.get()) == nullptr, "remote I/O");
    enforce(io_->open() == 0, ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
    IoCloser closer(*io_);
    auto document = parseBmff(*io_);
    enforceHeifWriteSupport(document);
    const DecodeParams params(max_recursion_depth_);
    auto original = Internal::readMetadata(*io_, document, params);

    // Reject changes to categories that the HEIF writer only preserves.
    if (loaded_ || !iccProfile_.empty())
      supported(iccProfile_.size() == original.icc.size() &&
                    (iccProfile_.empty() ||
                     std::memcmp(iccProfile_.c_data(), original.icc.c_data(), iccProfile_.size()) == 0),
                "ICC profile editing");
    if (loaded_ || !iptcData_.empty())
      supported(sameData(iptcData_, original.iptc) || (exifData_.empty() && iptcData_.empty()),
                "standalone IPTC editing");
    supported(comment_.empty(), "image comments");

    // Deleting the MakerNote also removes its decoded vendor fields from the fresh TIFF tree.
    auto exif = exifData_;
    if (original.exif.findKey(ExifKey("Exif.Photo.MakerNote")) != original.exif.end() &&
        exif.findKey(ExifKey("Exif.Photo.MakerNote")) == exif.end()) {
      exif.erase(std::remove_if(exif.begin(), exif.end(),
                                [](const auto& datum) {
                                  return ExifTags::isMakerGroup(datum.groupName()) || datum.groupName() == "MakerNote";
                                }),
                 exif.end());
    }

    // Determine effective edit intent from both explicit clears and mutable metadata views.
#ifndef EXV_HAVE_XMP_TOOLKIT
    supported(xmpData_.empty(), "structured XMP editing requires the XMP toolkit");
#endif
    auto packet = writeXmpFromPacket() ? xmpPacket_ : encodeXmp(xmpData_);
    supported(packet.size() <= bmffMetadataLimit, "XMP exceeds the allocation limit");
    const bool xmpChanged =
        removeXmp_ || (writeXmpFromPacket() ? packet != original.packet : packet != encodeXmp(original.xmp));
    const bool exifChanged = removeExif_ || !sameData(exif, original.exif);
    supported(!original.needsXmpToolkit || xmpChanged || !exifChanged,
              "merging different primary XMP packets requires the XMP toolkit");

    // Promote embedded XMP to its own item when Exif removal would otherwise discard it.
    BmffMetadataUpdate update;
    if (xmpChanged || (exifChanged && exif.empty() && original.hasEmbeddedXmp))
      update.xmp = Bytes(packet.begin(), packet.end());

    // A separate MIME item becomes authoritative when XMP is edited. Remove its
    // old embedded TIFF copy so it cannot reappear after removing the MIME item.
    auto embedded = original.embeddedXmp;

    // Even without a live XMLPacket tag, prior XMP may survive in TIFF slack.
    const bool embeddedChanged = xmpChanged && !exif.empty();
    if (xmpChanged) {
      eraseTag(exif, "Exif.Image.XMLPacket");
      embedded.clear();
    }
#ifdef EXV_HAVE_XMP_TOOLKIT
    else if (exifChanged && !sameData(xmlPacketData(exif), xmlPacketData(original.exif))) {
      // An unrelated Exif edit retains the union of embedded packets. Only an
      // explicit raw-tag edit replaces that union with the current first packet.
      embedded.clear();
      if (auto xml = exif.findKey(ExifKey("Exif.Image.XMLPacket")); xml != exif.end()) {
        supported(xml->size() <= bmffMetadataLimit, "embedded XMP exceeds the allocation limit");
        Bytes bytes(xml->size());
        xml->copy(bytes.data(), original.order);
        enforce(XmpParser::decode(embedded, std::string(bytes.begin(), bytes.end()), params) == 0,
                ErrorCode::kerInvalidXMP);
      }
    }
#endif

    // Rebuild Exif when its values change or when old embedded XMP must be scrubbed.
    if (exifChanged || embeddedChanged) {
      if (exif.findKey(ExifKey("Exif.Canon.AFInfo")) != exif.end() &&
          original.exif.findKey(ExifKey("Exif.Canon.AFInfo")) != original.exif.end()) {
        supported(sameData(derivedCanonData(exif), derivedCanonData(original.exif)),
                  "Canon synthesized autofocus fields are read-only");
      }

      if (exif.empty())
        update.exif = Bytes{};
      else
        update.exif = encodeExif(exif, original.iptc, embedded, original.order, params);
    }

    // Preserve byte identity only when neither an edit nor orphaned-media cleanup is pending.
    if (!update.exif && !update.xmp && !hasOrphanedMedia(document))
      return;

    // Prepare and decode the complete result before touching the original destination.
    PreparedOutput prepared(*io_);
    rewriteBmff(*io_, prepared.io(), document, update);
    auto verified = Internal::readMetadata(prepared.io(), parseBmff(prepared.io()), params);
    const auto preparedSize = prepared.io().size();

    // The final BasicIo transfer can still fail after preparation; it is not an atomic replacement.
#ifdef EXV_ENABLE_FILESYSTEM
    if (dynamic_cast<FileIo*>(io_.get())) {
      TransferSource source(prepared.io());
      io_->transfer(source);
    } else
#endif
    {
      io_->transfer(prepared.io());
    }

    // Publish the verified in-memory state only after the transfer reports the expected size.
    enforce(io_->size() == preparedSize, ErrorCode::kerTransferFailed, io_->path(), "short prepared-output transfer");
    assign(std::move(verified));
    brand_ = document.majorBrand;
  }

  //! @brief Print metadata or delegate structural tracing without changing pending edits.
  void printStructure(std::ostream& out, PrintStructureOption option, size_t depth) override {
    if (!loaded_)
      readMetadata();

    if (option == kpsXMP) {
#ifdef EXV_HAVE_XMP_TOOLKIT
      std::string packet;
      enforce(XmpParser::encode(packet, xmpData_) == 0, ErrorCode::kerInvalidXMP);
      out << packet;
#else
      out << xmpPacket_;
#endif
    } else if (option == kpsIccProfile)
      out.write(iccProfile_.c_str(), iccProfile_.size());
    else if (option == kpsBasic || option == kpsRecursive) {
      // The legacy trace reader owns its traversal limits and metadata state.
      // Give it a borrowed read-only view, preserving this object's pending edits.
      BmffImage printer(std::make_unique<TransferSource>(*io_), ImageCtorParams(false, max_recursion_depth_));
      printer.printStructure(out, option, depth);
    }
  }

 private:
  // Adopt verified metadata and reset edit intent after a successful read or write.
  void assign(Metadata metadata) {
    exifData_ = std::move(metadata.exif);
    iptcData_ = std::move(metadata.iptc);
    xmpData_ = std::move(metadata.xmp);
    xmpPacket_ = std::move(metadata.packet);
    iccProfile_ = std::move(metadata.icc);
    pixelWidth_ = metadata.width;
    pixelHeight_ = metadata.height;
    setByteOrder(metadata.order);

    // The adopted state becomes the new baseline for detecting the next edit.
    writeXmpFromPacket(false);
    legacyRead_ = false;
    loaded_ = true;
    removeExif_ = removeXmp_ = false;
  }

  uint32_t brand_{bmffType("heic")};
  bool loaded_{}, removeExif_{}, removeXmp_{};
  bool legacyRead_{};  //!< MIME reporting uses the brand read by the successful legacy traversal.
};
}  // namespace

bool isHeifType(BasicIo& io, bool advance) {
  // Probe only the leading ftyp box, without constructing a metadata reader.
  const auto start = io.tell();
  std::array<byte, 16> header{};
  bool matched = io.read(header.data(), 8) == 8 && getULong(header.data() + 4, bigEndian) == bmffType("ftyp");
  uint64_t size = matched ? getULong(header.data(), bigEndian) : 0;
  unsigned headerSize = 8;

  if (matched && size == 1) {
    matched = io.read(header.data() + 8, 8) == 8;
    size = matched ? getULongLong(header.data() + 8, bigEndian) : 0;
    headerSize = 16;
  }

  // Accept complete bounded brand tables, including the extended-size header form.
  matched = matched && size >= headerSize + 8 && size <= BmffLimits{}.maxBytesRead && size <= io.size() - start &&
            (size - headerSize) % 4 == 0;

  if (matched) {
    matched = io.read(header.data(), 8) == 8;
    const auto brand = getULong(header.data(), bigEndian);
    matched = matched && (brand == bmffType("heic") || brand == bmffType("heix") || brand == bmffType("mif1"));

    // An AVIF compatible brand keeps generic mif1 files on the read-only path.
    for (uint64_t offset = headerSize + 8; matched && offset < size; offset += 4) {
      matched = io.read(header.data(), 4) == 4;
      const auto compatible = getULong(header.data(), bigEndian);
      matched = matched && compatible != bmffType("avif") && compatible != bmffType("avis");
    }
  }

  // Failed probes and non-advancing probes restore the caller position.
  if (!advance || !matched)
    io.seek(static_cast<int64_t>(start), BasicIo::beg);

  return matched;
}

Image::UniquePtr newHeifInstance(std::unique_ptr<BasicIo> io, const ImageCtorParams& params) {
  auto image = std::make_unique<HeifImage>(std::move(io), params);
  return image->good() ? std::move(image) : nullptr;
}
}  // namespace Exiv2::Internal
#endif
