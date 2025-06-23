// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef PENTAXMN_INT_HPP_
#define PENTAXMN_INT_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "value.hpp"

#include "tags_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
// *****************************************************************************
// class definitions

//! MakerNote for Pentaxfilm cameras
class PentaxMakerNote {
 public:
  //! Return read-only list of built-in Pentaxfilm tags
  static const TagInfo* tagList();

  //! Print Pentax version
  static std::ostream& printVersion(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax resolution
  static std::ostream& printResolution(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax date
  static std::ostream& printDate(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax time
  static std::ostream& printTime(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax exposure
  static std::ostream& printExposure(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax F value
  static std::ostream& printFValue(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax focal length
  static std::ostream& printFocalLength(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax compensation
  static std::ostream& printCompensation(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax temperature
  static std::ostream& printTemperature(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax flash compensation
  static std::ostream& printFlashCompensation(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax bracketing
  static std::ostream& printBracketing(std::ostream& os, const Value& value, const ExifData*);
  //! Print Pentax shutter count
  static std::ostream& printShutterCount(std::ostream& os, const Value& value, const ExifData*);

 private:
  //! Tag information
  static const TagInfo tagInfo_[];
};  // class PentaxMakerNote

}  // namespace Exiv2::Internal

#endif  // #ifndef PENTAXMN_INT_HPP_
