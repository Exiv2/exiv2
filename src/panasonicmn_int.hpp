// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef PANASONICMN_INT_HPP_
#define PANASONICMN_INT_HPP_

// *****************************************************************************
// included header files
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
class ExifData;
class Value;
struct TagInfo;

namespace Internal {
// *****************************************************************************
// class definitions

//! MakerNote for Panasonic cameras
class PanasonicMakerNote {
 public:
  //! Return read-only list of built-in Panasonic tags
  static constexpr auto tagList() {
    return tagInfo_;
  }
  //! Return read-only list of built-in Panasonic RAW image tags (IFD0)
  static constexpr auto tagListRaw() {
    return tagInfoRaw_;
  }

  //! @name Print functions for Panasonic %MakerNote tags
  //@{
  /*! @brief Display validated face positions in the camera's thumbnail coordinate system.
   *  Requires MakerNote byte-order metadata. Unknown/truncated records remain raw.
   *  The encoded parent Value is never modified or replaced by derived fields.
   */
  static std::ostream& printFacePositions(std::ostream& os, const Value& value, const ExifData* metadata);
  //! @brief Display bounded face names, rectangles and ages; preserve unsupported layouts as raw values.
  static std::ostream& printRecognizedFaces(std::ostream& os, const Value& value, const ExifData* metadata);
  //! @brief Display packed-decimal date/time and an optional shot counter, without inferring a time zone.
  static std::ostream& printTimeInfo(std::ostream& os, const Value& value, const ExifData* metadata);
  //! @brief Display the two signed settings while retaining their unsigned TIFF encoding.
  static std::ostream& printHighlightShadow(std::ostream& os, const Value& value, const ExifData*);
  //! @brief Interpret the packed filter selector without rational division, including zero denominators.
  static std::ostream& printFilterEffect(std::ostream& os, const Value& value, const ExifData*);
  //! @brief Decode the documented all-zero merging mode; other pairs remain visible unchanged.
  static std::ostream& printPostFocusMerging(std::ostream& os, const Value& value, const ExifData*);
  //! @brief Display one unsigned 16-bit component as signed, preserving its encoded value.
  static std::ostream& printSignedShort(std::ostream& os, const Value& value, const ExifData*);
  //! @brief Display one unsigned byte as signed; unexpected types/counts remain raw.
  static std::ostream& printSignedByte(std::ostream& os, const Value& value, const ExifData*);
  //! @brief Display the camera's reserved high ISO values as unavailable.
  static std::ostream& printISO(std::ostream& os, const Value& value, const ExifData*);
  //! @brief Print AF area mode, using camera identity for overlapping encodings.
  static std::ostream& print0x000f(std::ostream& os, const Value& value, const ExifData*);
  //! Print WhiteBalanceBias
  static std::ostream& print0x0023(std::ostream& os, const Value& value, const ExifData*);
  //! Print TimeSincePowerOn
  static std::ostream& print0x0029(std::ostream& os, const Value& value, const ExifData*);
  //! Print Baby age
  static std::ostream& print0x0033(std::ostream& os, const Value& value, const ExifData*);
  //! Print Travel days
  static std::ostream& print0x0036(std::ostream& os, const Value& value, const ExifData*);
  //! Print ISO
  static std::ostream& print0x003c(std::ostream& os, const Value& value, const ExifData*);
  //! Print Manometer Pressure
  static std::ostream& printPressure(std::ostream& os, const Value& value, const ExifData*);
  //! Print special text values: title, landmark, county and so on
  static std::ostream& printPanasonicText(std::ostream& os, const Value& value, const ExifData*);
  //! Print accelerometer readings
  static std::ostream& printAccelerometer(std::ostream& os, const Value& value, const ExifData*);
  //! Print roll angle
  static std::ostream& printRollAngle(std::ostream& os, const Value& value, const ExifData*);
  //! Print pitch angle
  static std::ostream& printPitchAngle(std::ostream& os, const Value& value, const ExifData*);
  //@}

 private:
  //! Makernote tag list
  static const TagInfo tagInfo_[];
  //! Taglist for IFD0 of Panasonic RAW images
  static const TagInfo tagInfoRaw_[];

};  // class PanasonicMakerNote
}  // namespace Internal
}  // namespace Exiv2

#endif  // #ifndef PANASONICMN_INT_HPP_
