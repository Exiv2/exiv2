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
  //! Print SpotMode
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
