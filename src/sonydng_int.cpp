// SPDX-License-Identifier: GPL-2.0-or-later

#include "sonydng_int.hpp"

#include "enforce.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "tags.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "tiffvisitor_int.hpp"
#include "value.hpp"
#include "xmp_exiv2.hpp"

#include <algorithm>
#include <cstring>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace Exiv2::Internal {
namespace {

// Borrow a validated MakN payload together with its original file coordinates.
struct SonyDngRecord {
  std::span<const byte> bytes;
  ByteOrder order;
  uint32_t originalOffset;
};

// Validate all framing before selecting one MakN; unknown records remain opaque.
std::optional<SonyDngRecord> findMakerNote(const byte* data, size_t size) {
  if (!isAdobeDngPrivateData(data, size))
    return std::nullopt;

  std::optional<SonyDngRecord> found;
  size_t position = 6;
  size_t records = 0;
  while (position < size) {
    enforce(++records <= 256 && size - position >= 8, ErrorCode::kerCorruptedMetadata);
    const bool makerNote = std::memcmp(data + position, "MakN", 4) == 0;
    const size_t length = getULong(data + position + 4, bigEndian);
    position += 8;
    enforce(length <= size - position, ErrorCode::kerCorruptedMetadata);

    // Record framing and saved position are big endian, independent of its IFD.
    if (makerNote) {
      enforce(!found && length >= 6, ErrorCode::kerCorruptedMetadata);
      ByteOrder order = invalidByteOrder;
      if (data[position] == 'I' && data[position + 1] == 'I')
        order = littleEndian;
      else if (data[position] == 'M' && data[position + 1] == 'M')
        order = bigEndian;
      enforce(order != invalidByteOrder, ErrorCode::kerCorruptedMetadata);
      found = SonyDngRecord{{data + position + 6, length - 6}, order, getULong(data + position + 2, bigEndian)};
    }

    position += length;
    if (length % 2) {
      enforce(position < size, ErrorCode::kerCorruptedMetadata);
      ++position;
    }
  }
  return found;
}

// Compare derived fields including duplicate keys, types, counts, and raw bytes.
auto makerValues(const ExifData& data) {
  std::vector<std::tuple<std::string, TypeId, size_t, Blob>> values;
  for (const auto& datum : data) {
    if (!ExifTags::isMakerGroup(datum.groupName()))
      continue;

    Blob bytes(datum.size());
    datum.copy(bytes.data(), littleEndian);
    values.emplace_back(datum.key(), datum.typeId(), datum.count(), std::move(bytes));
  }
  std::sort(values.begin(), values.end());
  return values;
}

// Detect an existing opaque Adobe block without decoding the containing image.
bool hasAdobeSource(TiffComponent* source) {
  if (!source)
    return false;

  TiffFinder finder(0xc634, IfdId::ifd0Id);
  source->accept(finder);
  const auto entry = dynamic_cast<const TiffEntryBase*>(finder.result());
  return entry && isAdobeDngPrivateData(entry->pData(), entry->size());
}

}  // namespace

bool isAdobeDngPrivateData(const byte* data, size_t size) {
  return size >= 6 && data && std::memcmp(data, "Adobe\0", 6) == 0;
}

ExifData decodeSonyDngPrivateData(const byte* data, size_t size, const ExifData& context, const DecodeParams& params) {
  ExifData decoded;
  const auto record = findMakerNote(data, size);
  if (!record)
    return decoded;

  // Headered Sony1 layouts have different child/preview handling; keep them opaque.
  const auto bytes = record->bytes;
  if (bytes.size() >= 12 && std::memcmp(bytes.data(), "SONY DSC \0\0\0", 12) == 0)
    return decoded;

  enforce(bytes.size() >= 6 && bytes.size() <= 16 * 1024 * 1024, ErrorCode::kerCorruptedMetadata);
  const auto count = getUShort(bytes.data(), record->order);
  enforce(count <= 256 && bytes.size() >= 6 + size_t{12} * count, ErrorCode::kerCorruptedMetadata);

  // Camera-dependent array selectors consult the same context as ordinary reading.
  TiffDirectory root(0, IfdId::ifd0Id, false);
  for (const auto* key : {"Exif.Image.Make", "Exif.Image.Model"}) {
    if (const auto datum = context.findKey(ExifKey(key)); datum != context.end()) {
      auto entry = std::make_shared<TiffEntry>(datum->tag(), IfdId::ifd0Id);
      entry->setValue(datum->getValue());
      root.addChild(std::move(entry));
    }
  }

  auto directory = std::make_shared<TiffDirectory>(0, IfdId::sony2Id, false);
  directory->setStart(bytes.data());
  root.addChild(directory);
  TiffReader reader(bytes.data(), bytes.size(), &root, {record->order, 0}, record->originalOffset);
  directory->accept(reader);
  reader.postProcess();

  IptcData iptc;
  XmpData xmp;
  TiffDecoder decoder(decoded, iptc, xmp, &root, TiffMapping::findDecoder, params);
  directory->accept(decoder);
  return decoded;
}

void filterSonyDngMetadata(ExifData& data, TiffComponent* source) {
  const auto native = data.findKey(ExifKey("Exif.Photo.MakerNote"));
  if (native != data.end() && native->size() != 0)
    return;

  const auto privateData = data.findKey(ExifKey("Exif.Image.DNGPrivateData"));
  const auto make = data.findKey(ExifKey("Exif.Image.Make"));
  const bool sony = make != data.end() && make->toString().starts_with("SONY");
  const bool sonyFields =
      std::any_of(data.begin(), data.end(), [](const auto& datum) { return datum.groupName().starts_with("Sony"); });
  if (!sony && !sonyFields)
    return;

  const bool existing = hasAdobeSource(source);
  if (privateData == data.end()) {
    enforce(!existing || makerValues(data).empty(), ErrorCode::kerInvalidSettingForImage, "Sony MakerNotes",
            "Adobe DNG");
    return;
  }

  Blob bytes(privateData->size());
  privateData->copy(bytes.data(), littleEndian);
  if (!isAdobeDngPrivateData(bytes.data(), bytes.size()))
    return;

  // Opaque imports need no derived view. Existing exposed fields are read-only.
  const auto actual = makerValues(data);
  if (actual.empty() && !existing)
    return;

  ExifData decoded;
  try {
    decoded = decodeSonyDngPrivateData(bytes.data(), bytes.size(), data, DecodeParams{100});
  } catch (const Error&) {
    // Invalid optional data exposed no derived fields and can remain opaque.
    enforce(actual.empty(), ErrorCode::kerInvalidSettingForImage, "Sony MakerNotes", "Adobe DNG");
    return;
  }
  enforce(actual == makerValues(decoded), ErrorCode::kerInvalidSettingForImage, "Sony MakerNotes", "Adobe DNG");
  data.erase(std::remove_if(data.begin(), data.end(),
                            [](const auto& datum) {
                              return ExifTags::isMakerGroup(datum.groupName()) || datum.groupName() == "MakerNote";
                            }),
             data.end());
}

}  // namespace Exiv2::Internal
