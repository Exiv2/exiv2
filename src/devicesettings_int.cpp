// SPDX-License-Identifier: GPL-2.0-or-later

#include "devicesettings_int.hpp"

#include <algorithm>
#include <array>
#include <string_view>

namespace {
using namespace Exiv2;

// UCS-2 excludes surrogate pairs; XML also excludes NUL and most control characters.
bool isTextUnit(uint32_t unit) {
  return unit == 9 || unit == 10 || unit == 13 || (unit >= 0x20 && unit < 0xd800) || (unit > 0xdfff && unit < 0xfffe);
}

// Convert one already validated UCS-2 unit to its shortest UTF-8 representation.
void appendUtf8(std::string& text, uint16_t unit) {
  if (unit < 0x80) {
    text.push_back(static_cast<char>(unit));
  } else if (unit < 0x800) {
    text.push_back(static_cast<char>(0xc0 | (unit >> 6)));
    text.push_back(static_cast<char>(0x80 | (unit & 0x3f)));
  } else {
    text.push_back(static_cast<char>(0xe0 | (unit >> 12)));
    text.push_back(static_cast<char>(0x80 | ((unit >> 6) & 0x3f)));
    text.push_back(static_cast<char>(0x80 | (unit & 0x3f)));
  }
}

// Append one native unit after checking vector growth before arithmetic.
bool appendUnit(Blob& bytes, uint16_t unit, ByteOrder order) {
  if (bytes.max_size() - bytes.size() < 2)
    return false;
  std::array<byte, 2> encoded;
  us2Data(encoded.data(), unit, order);
  bytes.insert(bytes.end(), encoded.begin(), encoded.end());
  return true;
}

// Decode strict UTF-8 into the BMP subset representable by an Exif UCS-2 string.
bool appendText(Blob& bytes, std::string_view text, ByteOrder order) {
  if (!appendUnit(bytes, 0xfeff, order))
    return false;

  for (size_t pos = 0; pos < text.size();) {
    const auto first = static_cast<byte>(text[pos++]);
    uint32_t unit = first;
    size_t continuation = 0;
    uint32_t minimum = 0;
    if (first >= 0xc2 && first <= 0xdf) {
      unit = first & 0x1f;
      continuation = 1;
      minimum = 0x80;
    } else if (first >= 0xe0 && first <= 0xef) {
      unit = first & 0x0f;
      continuation = 2;
      minimum = 0x800;
    } else if (first >= 0x80) {
      return false;
    }

    if (continuation > text.size() - pos)
      return false;
    while (continuation > 0) {
      --continuation;
      const auto next = static_cast<byte>(text.at(pos++));
      if ((next & 0xc0) != 0x80)
        return false;
      unit = (unit << 6) | (next & 0x3f);
    }
    if (unit < minimum || !isTextUnit(unit) || !appendUnit(bytes, static_cast<uint16_t>(unit), order))
      return false;
  }
  return appendUnit(bytes, 0, order);
}

// A signature identifies a string's byte order independently of the TIFF header.
ByteOrder signedOrder(const byte* bytes) {
  if (bytes[0] == 0xfe && bytes[1] == 0xff)
    return bigEndian;
  if (bytes[0] == 0xff && bytes[1] == 0xfe)
    return littleEndian;
  return invalidByteOrder;
}
}  // namespace

namespace Exiv2::Internal {

int DeviceSettingsValue::read(const byte* buf, size_t len, ByteOrder byteOrder) {
  const auto result = DataValue::read(buf, len, byteOrder);
  byteOrder_ = byteOrder;
  xmpOrigin_.clear();
  hasXmpOrigin_ = false;
  return result;
}

DeviceSettingsValue* DeviceSettingsValue::clone_() const {
  return new DeviceSettingsValue(*this);
}

size_t DeviceSettingsValue::copy(byte* buf, ByteOrder targetOrder) const {
  const auto size = DataValue::copy(buf, targetOrder);
  if (targetOrder == invalidByteOrder || byteOrder_ == invalidByteOrder || targetOrder == byteOrder_)
    return size;

  // Preserve malformed raw values; change coordinates only after full validation.
  DeviceSettings settings;
  if (!decodeDeviceSettings(*this, settings))
    return size;
  us2Data(buf, settings.columns, targetOrder);
  us2Data(buf + 2, settings.rows, targetOrder);

  // Signed text is self-describing; legacy unsigned strings follow the TIFF order.
  size_t pos = 4;
  while (pos < size) {
    const auto signature = signedOrder(value_.data() + pos);
    const auto order = signature == invalidByteOrder ? byteOrder_ : signature;
    if (signature != invalidByteOrder)
      pos += 2;
    uint16_t unit;
    do {
      unit = getUShort(value_.data() + pos, order);
      if (signature == invalidByteOrder)
        std::swap(buf[pos], buf[pos + 1]);
      pos += 2;
    } while (unit != 0);
  }
  return size;
}

bool decodeDeviceSettings(const Value& value, DeviceSettings& settings) {
  const auto raw = dynamic_cast<const DataValue*>(&value);
  if (!raw || value.typeId() != undefined || raw->value_.size() < 4 || raw->value_.size() % 2 != 0)
    return false;

  const auto native = dynamic_cast<const DeviceSettingsValue*>(&value);
  auto order = native ? native->byteOrder() : invalidByteOrder;
  const auto& bytes = raw->value_;
  if (order == invalidByteOrder) {
    // An untyped caller value is safe only when both interpretations agree.
    if (getUShort(bytes.data(), littleEndian) != getUShort(bytes.data(), bigEndian) ||
        getUShort(bytes.data() + 2, littleEndian) != getUShort(bytes.data() + 2, bigEndian))
      return false;
  }

  DeviceSettings decoded;
  const auto dimensionsOrder = order == invalidByteOrder ? littleEndian : order;
  decoded.columns = getUShort(bytes.data(), dimensionsOrder);
  decoded.rows = getUShort(bytes.data() + 2, dimensionsOrder);

  // Settings are a list, not a rows-times-columns matrix (Exif figure 22).
  size_t pos = 4;
  while (pos < bytes.size()) {
    const auto signature = signedOrder(bytes.data() + pos);
    const auto textOrder = signature == invalidByteOrder ? order : signature;
    if (textOrder == invalidByteOrder)
      return false;
    if (signature != invalidByteOrder)
      pos += 2;

    std::string text;
    bool terminated = false;
    while (pos < bytes.size()) {
      const auto unit = getUShort(bytes.data() + pos, textOrder);
      pos += 2;
      if (unit == 0) {
        terminated = true;
        break;
      }
      if (!isTextUnit(unit))
        return false;
      appendUtf8(text, unit);
    }
    if (!terminated)
      return false;
    decoded.settings.push_back(std::move(text));
  }
  settings = std::move(decoded);
  return true;
}

bool encodeDeviceSettings(const DeviceSettings& settings, ByteOrder order, DeviceSettingsValue& value) {
  if (order != littleEndian && order != bigEndian)
    return false;
  Blob bytes;
  if (!appendUnit(bytes, settings.columns, order) || !appendUnit(bytes, settings.rows, order))
    return false;
  for (const auto& text : settings.settings) {
    if (!appendText(bytes, text, order))
      return false;
  }
  value.read(bytes.data(), bytes.size(), order);
  return true;
}

}  // namespace Exiv2::Internal
