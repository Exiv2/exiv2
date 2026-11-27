// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIV2_DEVICESETTINGS_INT_HPP
#define EXIV2_DEVICESETTINGS_INT_HPP

#include "value.hpp"

#include <string>
#include <vector>

namespace Exiv2::Internal {

//! @brief Decoded display dimensions and ordered UCS-2 camera-setting values.
struct DeviceSettings {
  uint16_t columns{};                 //!< Display columns, independent of the number of settings.
  uint16_t rows{};                    //!< Display rows, independent of the number of settings.
  std::vector<std::string> settings;  //!< UTF-8 text, without signatures or terminators.
};

/*!
  @brief Own an UNDEFINED DeviceSettingDescription value and its TIFF byte order.

  Cloning retains the byte order. Byte display and the TIFF type remain those
  of DataValue; serialization adjusts dimensions and unsigned text when the
  destination TIFF uses a different byte order. Signed UCS-2 strings retain
  their own byte order. Malformed values remain available as their raw bytes.
 */
class DeviceSettingsValue final : public DataValue {
 public:
  //! @brief Construct an empty value without an assumed source byte order.
  DeviceSettingsValue() = default;

  using DataValue::read;

  //! @brief Copy native bytes and remember the supplied TIFF byte order.
  int read(const byte* buf, size_t len, ByteOrder byteOrder) override;

  //! @brief Copy bytes to caller-owned storage, relocating valid byte-order-dependent fields.
  size_t copy(byte* buf, ByteOrder byteOrder) const override;

  //! @brief Return the byte order retained from parsing or reconstruction.
  ByteOrder byteOrder() const {
    return byteOrder_;
  }

  //! @brief Remember the native projection loaded from this sidecar's XMP.
  void rememberXmpOrigin() {
    xmpOrigin_ = value_;
    hasXmpOrigin_ = true;
  }

  //! @brief Whether the native projection is still unchanged since the sidecar read.
  bool unchangedXmpOrigin() const {
    return hasXmpOrigin_ && xmpOrigin_ == value_;
  }

 private:
  //! @brief Clone the owned bytes together with their byte-order context.
  DeviceSettingsValue* clone_() const override;

  ByteOrder byteOrder_{invalidByteOrder};
  Blob xmpOrigin_;  //!< Owned snapshot; clones preserve sidecar edit precedence.
  bool hasXmpOrigin_{false};
};

/*!
  @brief Decode a complete native DeviceSettings value without changing it.
  @param value UNDEFINED bytes, preferably retaining their parsed TIFF byte order.
  @param settings Receives the complete result only on success.
  @return False for unknown/ambiguous byte order, truncated fields, invalid XML
          characters, malformed UCS-2, or missing string terminators.
 */
bool decodeDeviceSettings(const Value& value, DeviceSettings& settings);

/*!
  @brief Encode dimensions and ordered UTF-8 text as a signed UCS-2 native value.
  @param settings Complete settings; list length is independent of display dimensions.
  @param byteOrder Valid TIFF byte order for dimensions and newly signed strings.
  @param value Receives the owned native value only on success.
  @return False for text outside UCS-2/XML, embedded NULs, invalid UTF-8, or overflow.
 */
bool encodeDeviceSettings(const DeviceSettings& settings, ByteOrder byteOrder, DeviceSettingsValue& value);

}  // namespace Exiv2::Internal

#endif  // EXIV2_DEVICESETTINGS_INT_HPP
